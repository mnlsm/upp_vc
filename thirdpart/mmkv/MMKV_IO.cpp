/*
* Tencent is pleased to support the open source community by making
* MMKV available.
*
* Copyright (C) 2020 THL A29 Limited, a Tencent company.
* All rights reserved.
*
* Licensed under the BSD 3-Clause License (the "License"); you may not use
* this file except in compliance with the License. You may obtain a copy of
* the License at
*
*       https://opensource.org/licenses/BSD-3-Clause
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "MMKV_IO.h"
#include "CodedInputData.h"
#include "CodedOutputData.h"
#include "InterProcessLock.h"
#include "MMBuffer.h"
#include "MMKVLog.h"
#include "MMKVMetaInfo.hpp"
#include "MemoryFile.h"
#include "MiniPBCoder.h"
#include "PBUtility.h"
#include "ScopedLock.hpp"
#include "ThreadLock.h"
#include <algorithm>
#include <cassert>
#include <cstring>



using namespace std;
using namespace mmkv;
typedef  std::pair<bool, KeyValueHolder> KVHolderRet_t ;

const DWORD Fixed32Size = pbFixed32Size();

MMKV_NAMESPACE_BEGIN

const bool KeepSequence = false;
const bool IncreaseSequence = true;


void MMKV::loadFromFile() {
    if (m_metaFile->isFileValid()) {
        m_metaInfo->read(m_metaFile->getMemory());
    }

    if (!m_file->isFileValid()) {
        m_file->reloadFromFile();
    }
    if (!m_file->isFileValid()) {
        MMKVError("file [%s] not valid", m_path.c_str());
    } else {
        // error checking
        bool loadFromFile = false, needFullWriteback = false;
        checkDataValid(loadFromFile, needFullWriteback);
        MMKVInfo("loading [%s] with %zu actual size, file size %zu, InterProcess %d, meta info "
                 "version:%u",
                 m_mmapID.c_str(), m_actualSize, m_file->getFileSize(), m_isInterProcess, m_metaInfo->m_version);
        BYTE * ptr = (BYTE *) m_file->getMemory();
        // loading
        if (loadFromFile && m_actualSize > 0) {
            MMKVInfo("loading [%s] with crc %u sequence %u version %u", m_mmapID.c_str(), m_metaInfo->m_crcDigest,
                     m_metaInfo->m_sequence, m_metaInfo->m_version);
            MMBuffer inputBuffer(ptr + Fixed32Size, m_actualSize, MMBufferNoCopy);
            if (m_crypter) {
                clearDictionary(m_dicCrypt);
            } else {
                clearDictionary(m_dic);
            }
            if (needFullWriteback) {
                {
                    MiniPBCoder::greedyDecodeMap(*m_dic, inputBuffer);
                }
            } else {
                {
                    MiniPBCoder::decodeMap(*m_dic, inputBuffer);
                }
            }
            m_output = new CodedOutputData(ptr + Fixed32Size, m_file->getFileSize() - Fixed32Size);
            m_output->seek(m_actualSize);
            if (needFullWriteback) {
                fullWriteback();
            }
        } else {
            // file not valid or empty, discard everything
            SCOPED_LOCK(InterProcessLock, m_exclusiveProcessLock);

            m_output = new CodedOutputData(ptr + Fixed32Size, m_file->getFileSize() - Fixed32Size);
            if (m_actualSize > 0) {
                writeActualSize(0, 0, NULL, IncreaseSequence);
                sync(MMKV_SYNC);
            } else {
                writeActualSize(0, 0, NULL, KeepSequence);
            }
        }
        size_t count = m_crypter ? m_dicCrypt->size() : m_dic->size();
        MMKVInfo("loaded [%s] with %zu key-values", m_mmapID.c_str(), count);
    }

    m_needLoadFromFile = false;
}

// read from last m_position
void MMKV::partialLoadFromFile() {
    m_metaInfo->read(m_metaFile->getMemory());

    size_t oldActualSize = m_actualSize;
    m_actualSize = readActualSize();
    size_t fileSize = m_file->getFileSize();
    MMKVDebug("loading [%s] with file size %zu, oldActualSize %zu, newActualSize %zu", m_mmapID.c_str(), fileSize,
              oldActualSize, m_actualSize);

    if (m_actualSize > 0) {
        if (m_actualSize < fileSize && m_actualSize + Fixed32Size <= fileSize) {
            if (m_actualSize > oldActualSize) {
                size_t position = oldActualSize;
                size_t addedSize = m_actualSize - position;
                BYTE* basePtr = (BYTE *) m_file->getMemory() + Fixed32Size;
                // incremental update crc digest
                m_crcDigest = (DWORD) CRC32(m_crcDigest, basePtr + position, addedSize);
                if (m_crcDigest == m_metaInfo->m_crcDigest) {
                    MMBuffer inputBuffer(basePtr, m_actualSize, MMBufferNoCopy);
                    {
                        MiniPBCoder::greedyDecodeMap(*m_dic, inputBuffer, position);
                    }
                    m_output->seek(addedSize);
                    m_hasFullWriteback = false;

                    size_t count = m_crypter ? m_dicCrypt->size() : m_dic->size();
                    MMKVDebug("partial loaded [%s] with %zu values", m_mmapID.c_str(), count);
                    return;
                } else {
                    MMKVError("m_crcDigest[%u] != m_metaInfo->m_crcDigest[%u]", m_crcDigest, m_metaInfo->m_crcDigest);
                }
            }
        }
    }
    // something is wrong, do a full load
    clearMemoryCache();
    loadFromFile();
}

void MMKV::checkLastConfirmedInfo(bool &loadFromFile, bool &needFullWriteback, size_t& fileSize) {
    if (m_metaInfo->m_version >= MMKVVersionActualSize) {
        // downgrade & upgrade support
        DWORD oldStyleActualSize = 0;
        memcpy(&oldStyleActualSize, m_file->getMemory(), Fixed32Size);
        if (oldStyleActualSize != m_actualSize) {
            MMKVWarning("oldStyleActualSize %u not equal to meta actual size %lu", oldStyleActualSize,
                        m_actualSize);
            if (oldStyleActualSize < fileSize && (oldStyleActualSize + Fixed32Size) <= fileSize) {
                if (checkFileCRCValid(oldStyleActualSize, m_metaInfo->m_crcDigest)) {
                    MMKVInfo("looks like [%s] been downgrade & upgrade again", m_mmapID.c_str());
                    loadFromFile = true;
                    writeActualSize(oldStyleActualSize, m_metaInfo->m_crcDigest, NULL, KeepSequence);
                    return;
                }
            } else {
                MMKVWarning("oldStyleActualSize %u greater than file size %lu", oldStyleActualSize, fileSize);
            }
        }

        DWORD lastActualSize = m_metaInfo->m_lastConfirmedMetaInfo.lastActualSize;
        if (lastActualSize < fileSize && (lastActualSize + Fixed32Size) <= fileSize) {
            DWORD lastCRCDigest = m_metaInfo->m_lastConfirmedMetaInfo.lastCRCDigest;
            if (checkFileCRCValid(lastActualSize, lastCRCDigest)) {
                loadFromFile = true;
                writeActualSize(lastActualSize, lastCRCDigest, NULL, KeepSequence);
            } else {
                MMKVError("check [%s] error: lastActualSize %u, lastActualCRC %u", m_mmapID.c_str(), lastActualSize,
                          lastCRCDigest);
            }
        } else {
            MMKVError("check [%s] error: lastActualSize %u, file size is %u", m_mmapID.c_str(), lastActualSize,
                      fileSize);
        }
    }
}

void MMKV::checkDataValid(bool &loadFromFile, bool &needFullWriteback) {
    // try auto recover from last confirmed location
    size_t fileSize = m_file->getFileSize();


    m_actualSize = readActualSize();

    if (m_actualSize < fileSize && (m_actualSize + Fixed32Size) <= fileSize) {
        if (checkFileCRCValid(m_actualSize, m_metaInfo->m_crcDigest)) {
            loadFromFile = true;
        } else {
            checkLastConfirmedInfo(loadFromFile, needFullWriteback, fileSize);

            if (!loadFromFile) {
                MMKVRecoverStrategic strategic = onMMKVCRCCheckFail(m_mmapID);
                if (strategic == OnErrorRecover) {
                    loadFromFile = true;
                    needFullWriteback = true;
                }
                MMKVInfo("recover strategic for [%s] is %d", m_mmapID.c_str(), strategic);
            }
        }
    } else {
        MMKVError("check [%s] error: %zu size in total, file size is %zu", m_mmapID.c_str(), m_actualSize, fileSize);

        checkLastConfirmedInfo(loadFromFile, needFullWriteback, fileSize);

        if (!loadFromFile) {
            MMKVRecoverStrategic strategic = onMMKVFileLengthError(m_mmapID);
            if (strategic == OnErrorRecover) {
                // make sure we don't over read the file
                m_actualSize = fileSize - Fixed32Size;
                loadFromFile = true;
                needFullWriteback = true;
            }
            MMKVInfo("recover strategic for [%s] is %d", m_mmapID.c_str(), strategic);
        }
    }
}

void MMKV::checkLoadData() {
    if (m_needLoadFromFile) {
        SCOPED_LOCK(InterProcessLock, m_sharedProcessLock);

        m_needLoadFromFile = false;
        loadFromFile();
        return;
    }
    if (!m_isInterProcess) {
        return;
    }

    if (!m_metaFile->isFileValid()) {
        return;
    }
    SCOPED_LOCK(InterProcessLock, m_sharedProcessLock);

    MMKVMetaInfo metaInfo;
    metaInfo.read(m_metaFile->getMemory());
    if (m_metaInfo->m_sequence != metaInfo.m_sequence) {
        MMKVInfo("[%s] oldSeq %u, newSeq %u", m_mmapID.c_str(), m_metaInfo->m_sequence, metaInfo.m_sequence);
        SCOPED_LOCK(InterProcessLock, m_sharedProcessLock);

        clearMemoryCache();
        loadFromFile();
        notifyContentChanged();
    } else if (m_metaInfo->m_crcDigest != metaInfo.m_crcDigest) {
        MMKVDebug("[%s] oldCrc %u, newCrc %u, new actualSize %u", m_mmapID.c_str(), m_metaInfo->m_crcDigest,
                  metaInfo.m_crcDigest, metaInfo.m_actualSize);
        SCOPED_LOCK(InterProcessLock, m_sharedProcessLock);

        size_t fileSize = m_file->getActualFileSize();
        if (m_file->getFileSize() != fileSize) {
            MMKVInfo("file size has changed [%s] from %zu to %zu", m_mmapID.c_str(), m_file->getFileSize(), fileSize);
            clearMemoryCache();
            loadFromFile();
        } else {
            partialLoadFromFile();
        }
        notifyContentChanged();
    }
}

const DWORD ItemSizeHolder = 0x00ffffff;
const DWORD ItemSizeHolderSize = 4;

static pair<MMBuffer, size_t> prepareEncode(const MMKVMap &dic) {
    // make some room for placeholder
    size_t totalSize = ItemSizeHolderSize;
    MMKVMap::const_iterator it= dic.begin();
    for (; it != dic.end(); ++it) {
        const KeyValueHolder &kvHolder = it->second;
        totalSize += kvHolder.computedKVSize + kvHolder.valueSize;
    }
    pair<MMBuffer, size_t> result;
    result.first = MMBuffer();
    result.second = totalSize;
    return result;
}

// since we use append mode, when -[setData: forKey:] many times, space may not be enough
// try a full rewrite to make space
bool MMKV::ensureMemorySize(size_t newSize) {
    if (!isFileValid()) {
        MMKVWarning("[%s] file not valid", m_mmapID.c_str());
        return false;
    }

    if (newSize >= m_output->spaceLeft() || (m_crypter ? m_dicCrypt->empty() : m_dic->empty())) {
        // try a full rewrite to make space
        size_t fileSize = m_file->getFileSize();
        pair<MMBuffer, size_t> preparedData;
        preparedData = m_crypter ? prepareEncode(*m_dicCrypt) : prepareEncode(*m_dic);
        size_t sizeOfDic = preparedData.second;
        size_t lenNeeded = sizeOfDic + Fixed32Size + newSize;
        size_t dicCount = m_crypter ? m_dicCrypt->size() : m_dic->size();
        size_t avgItemSize = lenNeeded / std::max<size_t>(1, dicCount);
        size_t futureUsage = avgItemSize * std::max<size_t>(8, (dicCount + 1) / 2);
        // 1. no space for a full rewrite, double it
        // 2. or space is not large enough for future usage, double it to avoid frequently full rewrite
        if (lenNeeded >= fileSize || (lenNeeded + futureUsage) >= fileSize) {
            size_t oldSize = fileSize;
            do {
                fileSize *= 2;
            } while (lenNeeded + futureUsage >= fileSize);
            MMKVInfo("extending [%s] file size from %zu to %zu, incoming size:%zu, future usage:%zu", m_mmapID.c_str(),
                     oldSize, fileSize, newSize, futureUsage);

            // if we can't extend size, rollback to old state
            if (!m_file->truncate(fileSize)) {
                return false;
            }

            // check if we fail to make more space
            if (!isFileValid()) {
                MMKVWarning("[%s] file not valid", m_mmapID.c_str());
                return false;
            }
        }
        return doFullWriteBack(preparedData, NULL);
    }
    return true;
}

size_t MMKV::readActualSize() {
    MMKV_ASSERT(m_file->getMemory());
    MMKV_ASSERT(m_metaFile->isFileValid());

    DWORD actualSize = 0;
    memcpy(&actualSize, m_file->getMemory(), Fixed32Size);

    if (m_metaInfo->m_version >= MMKVVersionActualSize) {
        if (m_metaInfo->m_actualSize != actualSize) {
            MMKVWarning("[%s] actual size %u, meta actual size %u", m_mmapID.c_str(), actualSize,
                        m_metaInfo->m_actualSize);
        }
        return m_metaInfo->m_actualSize;
    } else {
        return actualSize;
    }
}

void MMKV::oldStyleWriteActualSize(size_t actualSize) {
    MMKV_ASSERT(m_file->getMemory());

    m_actualSize = actualSize;
#ifdef MMKV_IOS
    auto ret = guardForBackgroundWriting(m_file->getMemory(), Fixed32Size);
    if (!ret.first) {
        return;
    }
#endif
    memcpy(m_file->getMemory(), &actualSize, Fixed32Size);
}

bool MMKV::writeActualSize(size_t size, DWORD crcDigest, const void *iv, bool increaseSequence) {
    // backward compatibility
    oldStyleWriteActualSize(size);

    if (!m_metaFile->isFileValid()) {
        return false;
    }

    bool needsFullWrite = false;
    m_actualSize = size;
    m_metaInfo->m_actualSize = static_cast<DWORD>(size);
    m_crcDigest = crcDigest;
    m_metaInfo->m_crcDigest = crcDigest;
    if (m_metaInfo->m_version < MMKVVersionSequence) {
        m_metaInfo->m_version = MMKVVersionSequence;
        needsFullWrite = true;
    }
    if (unlikely(increaseSequence)) {
        m_metaInfo->m_sequence++;
        m_metaInfo->m_lastConfirmedMetaInfo.lastActualSize = static_cast<DWORD>(size);
        m_metaInfo->m_lastConfirmedMetaInfo.lastCRCDigest = crcDigest;
        if (m_metaInfo->m_version < MMKVVersionActualSize) {
            m_metaInfo->m_version = MMKVVersionActualSize;
        }
        needsFullWrite = true;
        MMKVInfo("[%s] increase sequence to %u, crc %u, actualSize %u", m_mmapID.c_str(), m_metaInfo->m_sequence,
                 m_metaInfo->m_crcDigest, m_metaInfo->m_actualSize);
    }
    if (unlikely(needsFullWrite)) {
        m_metaInfo->write(m_metaFile->getMemory());
    } else {
        m_metaInfo->writeCRCAndActualSizeOnly(m_metaFile->getMemory());
    }
    return true;
}

MMBuffer MMKV::getDataForKey(MMKVKey_t key) {
    checkLoadData();
    {
        MMKVMap::iterator itr = m_dic->find(key);
        if (itr != m_dic->end()) {
            BYTE* basePtr = (BYTE *) (m_file->getMemory()) + Fixed32Size;
            return itr->second.toMMBuffer(basePtr);
        }
    }
    MMBuffer nan;
    return nan;
}



bool MMKV::setDataForKey(MMBuffer &data, MMKVKey_t key, bool isDataHolder) {
    if ((!isDataHolder && data.length() == 0) || isKeyEmpty(key)) {
        return false;
    }
    SCOPED_LOCK(ThreadLock, m_lock);
    SCOPED_LOCK(InterProcessLock, m_exclusiveProcessLock);
    checkLoadData();


    {
        MMKVMap::iterator itr = m_dic->find(key);
        if (itr != m_dic->end()) {
            KVHolderRet_t ret = appendDataWithKey(data, itr->second, isDataHolder);
            if (!ret.first) {
                return false;
            }
            itr->second = ret.second;
        } else {
            KVHolderRet_t ret = appendDataWithKey(data, key, isDataHolder);
            if (!ret.first) {
                return false;
            }
            (*m_dic)[key] = ret.second;
        }
    }
    m_hasFullWriteback = false;
    return true;
}

bool MMKV::removeDataForKey(MMKVKey_t key) {
    if (isKeyEmpty(key)) {
        return false;
    }

    {
        MMKVMap::iterator itr = m_dic->find(key);
        if (itr != m_dic->end()) {
            m_hasFullWriteback = false;
            static MMBuffer nan;
            KVHolderRet_t ret = appendDataWithKey(nan, itr->second);
            if (ret.first) {
                m_dic->erase(itr);
            }
            return ret.first;
        }
    }

    return false;
}

KVHolderRet_t
MMKV::doAppendDataWithKey(const MMBuffer &data, const MMBuffer &keyData, bool isDataHolder, DWORD originKeyLength) {
    bool isKeyEncoded = (originKeyLength < keyData.length());
    DWORD keyLength = static_cast<DWORD>(keyData.length());
    DWORD valueLength = static_cast<DWORD>(data.length());
    if (isDataHolder) {
        valueLength += pbRawVarint32Size(valueLength);
    }
    // size needed to encode the key
    size_t size = isKeyEncoded ? keyLength : (keyLength + pbRawVarint32Size(keyLength));
    // size needed to encode the value
    size += valueLength + pbRawVarint32Size(valueLength);

    SCOPED_LOCK(InterProcessLock, m_exclusiveProcessLock);

    bool hasEnoughSize = ensureMemorySize(size);
    if (!hasEnoughSize || !isFileValid()) {
        return make_pair(false, KeyValueHolder());
    }

    try {
        if (isKeyEncoded) {
            m_output->writeRawData(keyData);
        } else {
            m_output->writeData(keyData);
        }
        if (isDataHolder) {
            m_output->writeRawVarint32((INT) valueLength);
        }
        m_output->writeData(data); // note: write size of data
    } catch (std::exception &e) {
        MMKVError("%s", e.what());
        return make_pair(false, KeyValueHolder());
    }

    DWORD offset = static_cast<DWORD>(m_actualSize);
    BYTE* ptr = (BYTE *) m_file->getMemory() + Fixed32Size + m_actualSize;
    m_actualSize += size;
    updateCRCDigest(ptr, size);

    return make_pair(true, KeyValueHolder(originKeyLength, valueLength, offset));
}

KVHolderRet_t MMKV::appendDataWithKey(const MMBuffer &data, MMKVKey_t key, bool isDataHolder) {
    MMBuffer keyData((void *) key.data(), key.size(), MMBufferNoCopy);
    return doAppendDataWithKey(data, keyData, isDataHolder, static_cast<DWORD>(keyData.length()));
}

KVHolderRet_t MMKV::appendDataWithKey(const MMBuffer &data, const KeyValueHolder &kvHolder, bool isDataHolder) {
    SCOPED_LOCK(InterProcessLock, m_exclusiveProcessLock);

    DWORD keyLength = kvHolder.keySize;
    // size needed to encode the key
    size_t rawKeySize = keyLength + pbRawVarint32Size(keyLength);

    // ensureMemorySize() might change kvHolder.offset, so have to do it early
    {
        DWORD valueLength = static_cast<DWORD>(data.length());
        if (isDataHolder) {
            valueLength += pbRawVarint32Size(valueLength);
        }
        DWORD size = rawKeySize + valueLength + pbRawVarint32Size(valueLength);
        bool hasEnoughSize = ensureMemorySize(size);
        if (!hasEnoughSize) {
            return make_pair(false, KeyValueHolder());
        }
    }
    BYTE* basePtr = (BYTE *) m_file->getMemory() + Fixed32Size;
    MMBuffer keyData(basePtr + kvHolder.offset, rawKeySize, MMBufferNoCopy);

    return doAppendDataWithKey(data, keyData, isDataHolder, keyLength);
}

bool MMKV::fullWriteback(AESCrypt *newCrypter) {
    if (m_hasFullWriteback) {
        return true;
    }
    if (m_needLoadFromFile) {
        return true;
    }
    if (!isFileValid()) {
        MMKVWarning("[%s] file not valid", m_mmapID.c_str());
        return false;
    }

    if (m_crypter ? m_dicCrypt->empty() : m_dic->empty()) {
        clearAll();
        return true;
    }

    pair<MMBuffer, size_t> preparedData;
    preparedData = m_crypter ? prepareEncode(*m_dicCrypt) : prepareEncode(*m_dic);
    size_t sizeOfDic = preparedData.second;
    SCOPED_LOCK(InterProcessLock, m_exclusiveProcessLock);
    if (sizeOfDic > 0) {
        size_t fileSize = m_file->getFileSize();
        if (sizeOfDic + Fixed32Size <= fileSize) {
            return doFullWriteBack(preparedData, newCrypter);
        } else {
            assert(0);
            assert(newCrypter == NULL);
            // ensureMemorySize will extend file & full rewrite, no need to write back again
            return ensureMemorySize(sizeOfDic + Fixed32Size - fileSize);
        }
    }
    return false;
}

// we don't need to really serialize the dictionary, just reuse what's already in the file
static void
memmoveDictionary(MMKVMap &dic, CodedOutputData *output, BYTE *ptr, AESCrypt *encrypter, size_t totalSize) {
    BYTE* originOutputPtr = output->curWritePointer();
    // make space to hold the fake size of dictionary's serialization result
    BYTE* writePtr = originOutputPtr + ItemSizeHolderSize;
    // reuse what's already in the file
    if (!dic.empty()) {
        // sort by offset
        vector<KeyValueHolder *> vec;
        vec.reserve(dic.size());
        
        for (MMKVMap::iterator itr = dic.begin(); itr != dic.end(); ++itr) {
            vec.push_back(&itr->second);
        }
        struct sortf {
;
            bool operator()(const KeyValueHolder* left, const KeyValueHolder* right) const {
                return left->offset < right->offset;
            }

        };

        sort(vec.begin(), vec.end(), sortf());

        // merge nearby items to make memmove quicker
        vector<pair<DWORD, DWORD>> dataSections; // pair(offset, size)
        pair<DWORD, DWORD> tpair;
        tpair.first = vec.front()->offset;
        tpair.second = vec.front()->computedKVSize + vec.front()->valueSize;

        dataSections.push_back(tpair);
        for (size_t index = 1, total = vec.size(); index < total; index++) {
            KeyValueHolder* kvHolder = vec[index];
            pair<DWORD, DWORD> &lastSection = dataSections.back();
            if (kvHolder->offset == lastSection.first + lastSection.second) {
                lastSection.second += kvHolder->computedKVSize + kvHolder->valueSize;
            } else {
                tpair.first = kvHolder->offset;
                tpair.second = kvHolder->computedKVSize + kvHolder->valueSize;
                dataSections.push_back(tpair);
            }
        }
        // do the move
        BYTE* basePtr = ptr + Fixed32Size;
        for (size_t i = 0; i < dataSections.size(); i++) {
            pair<DWORD, DWORD>& section =  dataSections[i];
            // memmove() should handle this well: src == dst
            memmove(writePtr, basePtr + section.first, section.second);
            writePtr += section.second;
        }
        // update offset
        if (!encrypter) {
            DWORD offset = ItemSizeHolderSize;
            for (size_t i = 0; i < vec.size(); i++) {
                KeyValueHolder* kvHolder = vec[i];
                kvHolder->offset = offset;
                offset += kvHolder->computedKVSize + kvHolder->valueSize;
            }
        }
    }
    // hold the fake size of dictionary's serialization result
    output->writeRawVarint32(ItemSizeHolder);
    size_t writtenSize = static_cast<size_t>(writePtr - originOutputPtr);
    assert(writtenSize == totalSize);
    output->seek(writtenSize - ItemSizeHolderSize);
}


bool MMKV::doFullWriteBack(pair<MMBuffer, size_t>& preparedData, AESCrypt *newCrypter) {
    BYTE* ptr = (BYTE *) m_file->getMemory();
    size_t totalSize = preparedData.second;
    delete m_output;
    m_output = new CodedOutputData(ptr + Fixed32Size, m_file->getFileSize() - Fixed32Size);
    {
        AESCrypt * encrypter = m_crypter;
        memmoveDictionary(*m_dic, m_output, ptr, encrypter, totalSize);
    }

    m_actualSize = totalSize;
    {
        recaculateCRCDigestWithIV(NULL);
    }
    m_hasFullWriteback = true;
    // make sure lastConfirmedMetaInfo is saved
    sync(MMKV_SYNC);
    return true;
}


void MMKV::trim() {
    SCOPED_LOCK(ThreadLock, m_lock);
    MMKVInfo("prepare to trim %s", m_mmapID.c_str());

    checkLoadData();

    if (m_actualSize == 0) {
        clearAll();
        return;
    } else if (m_file->getFileSize() <= DEFAULT_MMAP_SIZE) {
        return;
    }
    SCOPED_LOCK(InterProcessLock, m_exclusiveProcessLock);

    fullWriteback();
    size_t oldSize = m_file->getFileSize();
    size_t fileSize = oldSize;
    while (fileSize > (m_actualSize + Fixed32Size) * 2) {
        fileSize /= 2;
    }
    fileSize = std::max<size_t>(fileSize, DEFAULT_MMAP_SIZE);
    if (oldSize == fileSize) {
        MMKVInfo("there's no need to trim %s with size %zu, actualSize %zu", m_mmapID.c_str(), fileSize, m_actualSize);
        return;
    }

    MMKVInfo("trimming %s from %zu to %zu, actualSize %zu", m_mmapID.c_str(), oldSize, fileSize, m_actualSize);

    if (!m_file->truncate(fileSize)) {
        return;
    }
    fileSize = m_file->getFileSize();
    BYTE* ptr = (BYTE *) m_file->getMemory();
    delete m_output;
    m_output = new CodedOutputData(ptr + pbFixed32Size(), fileSize - Fixed32Size);
    m_output->seek(m_actualSize);

    MMKVInfo("finish trim %s from %zu to %zu", m_mmapID.c_str(), oldSize, fileSize);
}

void MMKV::clearAll() {
    MMKVInfo("cleaning all key-values from [%s]", m_mmapID.c_str());
    SCOPED_LOCK(ThreadLock, m_lock);
    SCOPED_LOCK(InterProcessLock,m_exclusiveProcessLock);

    checkLoadData();

    if (m_file->getFileSize() == DEFAULT_MMAP_SIZE && m_actualSize == 0) {
        MMKVInfo("nothing to clear for [%s]", m_mmapID.c_str());
        return;
    }
    m_file->truncate(DEFAULT_MMAP_SIZE);

    writeActualSize(0, 0, NULL, IncreaseSequence);
    m_metaFile->msync(MMKV_SYNC);

    clearMemoryCache();
    loadFromFile();
}

bool MMKV::isFileValid(const string &mmapID, MMKVPath_t *relatePath) {
    MMKVPath_t kvPath = mappedKVPathWithID(mmapID, MMKV_SINGLE_PROCESS, relatePath);
    if (!isFileExist(kvPath)) {
        return true;
    }

    MMKVPath_t crcPath = crcPathWithID(mmapID, MMKV_SINGLE_PROCESS, relatePath);
    if (!isFileExist(crcPath)) {
        return false;
    }

    DWORD crcFile = 0;
    MMBuffer *data = readWholeFile(crcPath);
    if (data) {
        if (data->getPtr()) {
            MMKVMetaInfo metaInfo;
            metaInfo.read(data->getPtr());
            crcFile = metaInfo.m_crcDigest;
        }
        delete data;
    } else {
        return false;
    }

    DWORD crcDigest = 0;
    MMBuffer *fileData = readWholeFile(kvPath);
    if (fileData) {
        if (fileData->getPtr() && (fileData->length() >= Fixed32Size)) {
            DWORD actualSize = 0;
            memcpy(&actualSize, fileData->getPtr(), Fixed32Size);
            if (actualSize > (fileData->length() - Fixed32Size)) {
                delete fileData;
                return false;
            }

            crcDigest = (DWORD) CRC32(0, (const BYTE *) fileData->getPtr() + Fixed32Size, (DWORD) actualSize);
        }
        delete fileData;
        return crcFile == crcDigest;
    } else {
        return false;
    }
}

MMKV_NAMESPACE_END
