/*
 * Tencent is pleased to support the open source community by making
 * MMKV available.
 *
 * Copyright (C) 2018 THL A29 Limited, a Tencent company.
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

#include "CodedInputData.h"
#include "CodedOutputData.h"
#include "InterProcessLock.h"
#include "KeyValueHolder.h"
#include "MMBuffer.h"
#include "MMKVLog.h"
#include "MMKVMetaInfo.hpp"
#include "MMKV_IO.h"
#include "MemoryFile.h"
#include "MiniPBCoder.h"
#include "PBUtility.h"
#include "ScopedLock.hpp"
#include "ThreadLock.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <unordered_set>
#include <Wincrypt.h>



using namespace std;
using namespace std::tr1;
using namespace mmkv;

unordered_map<string, MMKV *> *g_instanceDic;
ThreadLock *g_instanceLock;
MMKVPath_t g_rootDir;
static mmkv::ErrorHandler g_errorHandler;
size_t mmkv::DEFAULT_MMAP_SIZE;


const wchar_t* SPECIAL_CHARACTER_DIRECTORY_NAME = L"specialCharacter";
const wchar_t* CRC_SUFFIX = L".crc";


const DWORD Fixed32Size = pbFixed32Size();

MMKV_NAMESPACE_BEGIN

const MMKVMode MMKV_SINGLE_PROCESS = (1 << 0);
const MMKVMode MMKV_MULTI_PROCESS = (1 << 1);

static MMKVPath_t encodeFilePath(const string &mmapID, const MMKVPath_t &rootDir);
bool endsWith(const MMKVPath_t &str, const MMKVPath_t &suffix);
MMKVPath_t filename(const MMKVPath_t &path);

#ifndef MMKV_ANDROID
MMKV::MMKV(const string &mmapID, MMKVMode mode, string *cryptKey, MMKVPath_t *rootPath)
    : m_mmapID(mmapID)
    , m_path(mappedKVPathWithID(m_mmapID, mode, rootPath))
    , m_crcPath(crcPathWithID(m_mmapID, mode, rootPath))
    , m_dic(NULL)
    , m_dicCrypt(NULL)
    , m_file(new MemoryFile(m_path))
    , m_metaFile(new MemoryFile(m_crcPath))
    , m_metaInfo(new MMKVMetaInfo())
    , m_crypter(NULL)
    , m_lock(new ThreadLock())
    , m_fileLock(new FileLock(m_metaFile->getFd()))
    , m_sharedProcessLock(new InterProcessLock(m_fileLock, SharedLockType))
    , m_exclusiveProcessLock(new InterProcessLock(m_fileLock, ExclusiveLockType))
    , m_isInterProcess((mode & MMKV_MULTI_PROCESS) != 0) {
    m_actualSize = 0;
    m_output = NULL;


    m_dic = new MMKVMap();

    m_needLoadFromFile = true;
    m_hasFullWriteback = false;

    m_crcDigest = 0;

    m_lock->initialize();
    m_sharedProcessLock->m_enable = m_isInterProcess;
    m_exclusiveProcessLock->m_enable = m_isInterProcess;

    // sensitive zone
    {
        SCOPED_LOCK(mmkv::InterProcessLock, m_sharedProcessLock);
        loadFromFile();
    }
}
#endif

MMKV::~MMKV() {
    clearMemoryCache();

    delete m_dic;

    delete m_file;
    delete m_metaFile;
    delete m_metaInfo;
    delete m_lock;
    delete m_fileLock;
    delete m_sharedProcessLock;
    delete m_exclusiveProcessLock;


    MMKVInfo("destruct [%s]", m_mmapID.c_str());
}

MMKV *MMKV::defaultMMKV(MMKVMode mode, string *cryptKey) {
    return mmkvWithID(DEFAULT_MMAP_ID, mode, cryptKey);
}

void initialize() {
    g_instanceDic = new unordered_map<string, MMKV *>;
    g_instanceLock = new ThreadLock();
    g_instanceLock->initialize();

    mmkv::DEFAULT_MMAP_SIZE = mmkv::getPageSize();
    MMKVInfo("version %s, page size %d, arch %s", MMKV_VERSION, DEFAULT_MMAP_SIZE, MMKV_ABI);

}

ThreadOnceToken_t once_control = ThreadOnceUninitialized;

void MMKV::initializeMMKV(const MMKVPath_t &rootDir, MMKVLogLevel logLevel, mmkv::LogHandler handler) {
    g_currentLogLevel = logLevel;
    g_logHandler = handler;

    ThreadLock::ThreadOnce(&once_control, initialize);

    g_rootDir = rootDir;
    mkPath(g_rootDir);

    MMKVInfo("root dir: " MMKV_PATH_FORMAT, g_rootDir.c_str());
}

const MMKVPath_t &MMKV::getRootDir() {
    return g_rootDir;
}

#ifndef MMKV_ANDROID
MMKV *MMKV::mmkvWithID(const string &mmapID, MMKVMode mode, string *cryptKey, MMKVPath_t *rootPath) {

    if (mmapID.empty()) {
        return NULL;
    }
    SCOPED_LOCK(ThreadLock, g_instanceLock);

    std::string mmapKey = mmapedKVKey(mmapID, rootPath);
    unordered_map<string, MMKV *>::iterator itr = g_instanceDic->find(mmapKey);
    if (itr != g_instanceDic->end()) {
        MMKV *kv = itr->second;
        return kv;
    }

    if (rootPath) {
        MMKVPath_t specialPath = (*rootPath) + MMKV_PATH_SLASH + SPECIAL_CHARACTER_DIRECTORY_NAME;
        if (!isFileExist(specialPath)) {
            mkPath(specialPath);
        }
        MMKVInfo("prepare to load %s (id %s) from rootPath %s", mmapID.c_str(), mmapKey.c_str(), rootPath->c_str());
    }

    MMKV* kv = new MMKV(mmapID, mode, cryptKey, rootPath);
    kv->m_mmapKey = mmapKey;
    (*g_instanceDic)[mmapKey] = kv;
    return kv;
}
#endif

void MMKV::onExit() {
    SCOPED_LOCK(ThreadLock, g_instanceLock);
    unordered_map<string, MMKV *>::iterator iter = g_instanceDic->begin();
    for(; iter != g_instanceDic->end(); ++iter) {
        MMKV *kv = iter->second;
        kv->sync();
        kv->clearMemoryCache();
        delete kv;
        iter->second = NULL;
    }
    delete g_instanceDic;
    g_instanceDic = NULL;
}

const string &MMKV::mmapID() const {
    return m_mmapID;
}

mmkv::ContentChangeHandler g_contentChangeHandler = NULL;

void MMKV::notifyContentChanged() {
    if (g_contentChangeHandler) {
        g_contentChangeHandler(m_mmapID);
    }
}

void MMKV::checkContentChanged() {
    SCOPED_LOCK(ThreadLock, m_lock);
    checkLoadData();
}

void MMKV::registerContentChangeHandler(mmkv::ContentChangeHandler handler) {
    g_contentChangeHandler = handler;
}

void MMKV::unRegisterContentChangeHandler() {
    g_contentChangeHandler = NULL;
}

void MMKV::clearMemoryCache() {
    SCOPED_LOCK(ThreadLock, m_lock);
    if (m_needLoadFromFile) {
        return;
    }
    MMKVInfo("clearMemoryCache [%s]", m_mmapID.c_str());
    m_needLoadFromFile = true;
    m_hasFullWriteback = false;

    clearDictionary(m_dic);


    delete m_output;
    m_output = NULL;

    m_file->clearMemoryCache();
    m_actualSize = 0;
    m_metaInfo->m_crcDigest = 0;
}

void MMKV::close() {
    MMKVInfo("close [%s]", m_mmapID.c_str());
    SCOPED_LOCK(ThreadLock, g_instanceLock);
    m_lock->lock();
    unordered_map<string, MMKV *>::iterator itr = g_instanceDic->find(m_mmapKey);
    if (itr != g_instanceDic->end()) {
        g_instanceDic->erase(itr);
    }
    delete this;
}



bool MMKV::isFileValid() {
    return m_file->isFileValid();
}

// crc

// assuming m_file is valid
bool MMKV::checkFileCRCValid(size_t actualSize, DWORD crcDigest) {
    BYTE* ptr = (BYTE *) m_file->getMemory();
    if (ptr) {
        m_crcDigest = (DWORD) CRC32(0, (const BYTE *) ptr + Fixed32Size, (DWORD) actualSize);

        if (m_crcDigest == crcDigest) {
            return true;
        }
        MMKVError("check crc [%s] fail, crc32:%u, m_crcDigest:%u", m_mmapID.c_str(), crcDigest, m_crcDigest);
    }
    return false;
}

void MMKV::recaculateCRCDigestWithIV(const void *iv) {
    const BYTE * ptr = (const BYTE *) m_file->getMemory();
    if (ptr) {
        m_crcDigest = 0;
        m_crcDigest = (DWORD) CRC32(0, ptr + Fixed32Size, (DWORD) m_actualSize);
        writeActualSize(m_actualSize, m_crcDigest, iv, IncreaseSequence);
    }
}

void MMKV::updateCRCDigest(const BYTE *ptr, size_t length) {
    if (ptr == NULL) {
        return;
    }
    m_crcDigest = (DWORD) CRC32(m_crcDigest, ptr, (DWORD) length);

    writeActualSize(m_actualSize, m_crcDigest, NULL, KeepSequence);
}

// set & get

bool MMKV::set(bool value, MMKVKey_t key) {
    if (isKeyEmpty(key)) {
        return false;
    }
    size_t size = pbBoolSize();
    MMBuffer data(size);
    CodedOutputData output(data.getPtr(), size);
    output.writeBool(value);

    return setDataForKey(data, key);
}

bool MMKV::set(INT value, MMKVKey_t key) {
    if (isKeyEmpty(key)) {
        return false;
    }
    size_t size = pbInt32Size(value);
    MMBuffer data(size);
    CodedOutputData output(data.getPtr(), size);
    output.writeInt32(value);

    return setDataForKey(data, key);
}

bool MMKV::set(DWORD value, MMKVKey_t key) {
    if (isKeyEmpty(key)) {
        return false;
    }
    size_t size = pbUInt32Size(value);
    MMBuffer data(size);
    CodedOutputData output(data.getPtr(), size);
    output.writeUInt32(value);

    return setDataForKey(data, key);
}

bool MMKV::set(__int64 value, MMKVKey_t key) {
    if (isKeyEmpty(key)) {
        return false;
    }
    size_t size = pbInt64Size(value);
    MMBuffer data(size);
    CodedOutputData output(data.getPtr(), size);
    output.writeInt64(value);

    return setDataForKey(data, key);
}

bool MMKV::set(unsigned __int64 value, MMKVKey_t key) {
    if (isKeyEmpty(key)) {
        return false;
    }
    size_t size = pbUInt64Size(value);
    MMBuffer data(size);
    CodedOutputData output(data.getPtr(), size);
    output.writeUInt64(value);

    return setDataForKey(data, key);
}

bool MMKV::set(float value, MMKVKey_t key) {
    if (isKeyEmpty(key)) {
        return false;
    }
    size_t size = pbFloatSize();
    MMBuffer data(size);
    CodedOutputData output(data.getPtr(), size);
    output.writeFloat(value);

    return setDataForKey(data, key);
}

bool MMKV::set(double value, MMKVKey_t key) {
    if (isKeyEmpty(key)) {
        return false;
    }
    size_t size = pbDoubleSize();
    MMBuffer data(size);
    CodedOutputData output(data.getPtr(), size);
    output.writeDouble(value);

    return setDataForKey(data, key);
}



bool MMKV::set(const char *value, MMKVKey_t key) {
    if (!value) {
        removeValueForKey(key);
        return true;
    }
    return setDataForKey(MMBuffer((void *) value, strlen(value), MMBufferNoCopy), key, true);
}

bool MMKV::set(const string &value, MMKVKey_t key) {
    if (isKeyEmpty(key)) {
        return false;
    }
    return setDataForKey(MMBuffer((void *) value.data(), value.length(), MMBufferNoCopy), key, true);
}

bool MMKV::set(const MMBuffer &value, MMKVKey_t key) {
    if (isKeyEmpty(key)) {
        return false;
    }
    // delay write the size needed for encoding value
    // avoid memory copying
    return setDataForKey(MMBuffer(value.getPtr(), value.length(), MMBufferNoCopy), key, true);
}

bool MMKV::set(const vector<string> &v, MMKVKey_t key) {
    if (isKeyEmpty(key)) {
        return false;
    }
    MMBuffer data = MiniPBCoder::encodeDataWithObject(v);
    return setDataForKey(data, key);
}

bool MMKV::getString(MMKVKey_t key, string &result) {
    if (isKeyEmpty(key)) {
        return false;
    }
    SCOPED_LOCK(ThreadLock, m_lock);
    SCOPED_LOCK(InterProcessLock, m_sharedProcessLock);
    MMBuffer data = getDataForKey(key);
    if (data.length() > 0) {
        try {
            CodedInputData input(data.getPtr(), data.length());
            result = input.readString();
            return true;
        } catch (std::exception &exception) {
            MMKVError("%s", exception.what());
        }
    }
    return false;
}

bool MMKV::getBytes(MMKVKey_t key, mmkv::MMBuffer &result) {
    if (isKeyEmpty(key)) {
        return false;
    }
    SCOPED_LOCK(ThreadLock, m_lock);
    SCOPED_LOCK(InterProcessLock, m_sharedProcessLock);
    MMBuffer data = getDataForKey(key);
    if (data.length() > 0) {
        try {
            CodedInputData input(data.getPtr(), data.length());
            result = input.readData();
            return true;
        } catch (std::exception &exception) {
            MMKVError("%s", exception.what());
        }
    }
    return false;
}

MMBuffer MMKV::getBytes(MMKVKey_t key) {
    if (isKeyEmpty(key)) {
        return MMBuffer();
    }
    SCOPED_LOCK(ThreadLock, m_lock);
    SCOPED_LOCK(InterProcessLock, m_sharedProcessLock);
    MMBuffer data = getDataForKey(key);
    if (data.length() > 0) {
        try {
            CodedInputData input(data.getPtr(), data.length());
            return input.readData();
        } catch (std::exception &exception) {
            MMKVError("%s", exception.what());
        }
    }
    return MMBuffer();
}

bool MMKV::getVector(MMKVKey_t key, vector<string> &result) {
    if (isKeyEmpty(key)) {
        return false;
    }
    SCOPED_LOCK(ThreadLock, m_lock);
    SCOPED_LOCK(InterProcessLock, m_sharedProcessLock);
    MMBuffer data = getDataForKey(key);
    if (data.length() > 0) {
        try {
            result = MiniPBCoder::decodeVector(data);
            return true;
        } catch (std::exception &exception) {
            MMKVError("%s", exception.what());
        }
    }
    return false;
}



bool MMKV::getBool(MMKVKey_t key, bool defaultValue, bool *hasValue) {
    if (isKeyEmpty(key)) {
        if (hasValue != NULL) {
            *hasValue = false;
        }
        return defaultValue;
    }
    SCOPED_LOCK(ThreadLock, m_lock);
    SCOPED_LOCK(InterProcessLock, m_sharedProcessLock);
    MMBuffer data = getDataForKey(key);
    if (data.length() > 0) {
        try {
            CodedInputData input(data.getPtr(), data.length());
            if (hasValue != NULL) {
                *hasValue = true;
            }
            return input.readBool();
        } catch (std::exception &exception) {
            MMKVError("%s", exception.what());
        }
    }
    if (hasValue != NULL) {
        *hasValue = false;
    }
    return defaultValue;
}

INT MMKV::getInt32(MMKVKey_t key, INT defaultValue, bool *hasValue) {
    if (isKeyEmpty(key)) {
        if (hasValue != NULL) {
            *hasValue = false;
        }
        return defaultValue;
    }
    SCOPED_LOCK(ThreadLock, m_lock);
    SCOPED_LOCK(InterProcessLock, m_sharedProcessLock);
    MMBuffer data = getDataForKey(key);
    if (data.length() > 0) {
        try {
            CodedInputData input(data.getPtr(), data.length());
            if (hasValue != NULL) {
                *hasValue = true;
            }
            return input.readInt32();
        } catch (std::exception &exception) {
            MMKVError("%s", exception.what());
        }
    }
    if (hasValue != NULL) {
        *hasValue = false;
    }
    return defaultValue;
}

DWORD MMKV::getUInt32(MMKVKey_t key, DWORD defaultValue, bool *hasValue) {
    if (isKeyEmpty(key)) {
        if (hasValue != NULL) {
            *hasValue = false;
        }
        return defaultValue;
    }
    SCOPED_LOCK(ThreadLock, m_lock);
    SCOPED_LOCK(InterProcessLock, m_sharedProcessLock);
    MMBuffer data = getDataForKey(key);
    if (data.length() > 0) {
        try {
            CodedInputData input(data.getPtr(), data.length());
            if (hasValue != NULL) {
                *hasValue = true;
            }
            return input.readUInt32();
        } catch (std::exception &exception) {
            MMKVError("%s", exception.what());
        }
    }
    if (hasValue != NULL) {
        *hasValue = false;
    }
    return defaultValue;
}

__int64 MMKV::getInt64(MMKVKey_t key, __int64 defaultValue, bool *hasValue) {
    if (isKeyEmpty(key)) {
        if (hasValue != NULL) {
            *hasValue = false;
        }
        return defaultValue;
    }
    SCOPED_LOCK(ThreadLock, m_lock);
    SCOPED_LOCK(InterProcessLock, m_sharedProcessLock);
    MMBuffer data = getDataForKey(key);
    if (data.length() > 0) {
        try {
            CodedInputData input(data.getPtr(), data.length());
            if (hasValue != NULL) {
                *hasValue = true;
            }
            return input.readInt64();
        } catch (std::exception &exception) {
            MMKVError("%s", exception.what());
        }
    }
    if (hasValue != NULL) {
        *hasValue = false;
    }
    return defaultValue;
}

unsigned __int64 MMKV::getUInt64(MMKVKey_t key, unsigned __int64 defaultValue, bool *hasValue) {
    if (isKeyEmpty(key)) {
        if (hasValue != NULL) {
            *hasValue = false;
        }
        return defaultValue;
    }
    SCOPED_LOCK(ThreadLock, m_lock);
    SCOPED_LOCK(InterProcessLock, m_sharedProcessLock);
    MMBuffer data = getDataForKey(key);
    if (data.length() > 0) {
        try {
            CodedInputData input(data.getPtr(), data.length());
            if (hasValue != NULL) {
                *hasValue = true;
            }
            return input.readUInt64();
        } catch (std::exception &exception) {
            MMKVError("%s", exception.what());
        }
    }
    if (hasValue != NULL) {
        *hasValue = false;
    }
    return defaultValue;
}

float MMKV::getFloat(MMKVKey_t key, float defaultValue, bool *hasValue) {
    if (isKeyEmpty(key)) {
        if (hasValue != NULL) {
            *hasValue = false;
        }
        return defaultValue;
    }
    SCOPED_LOCK(ThreadLock, m_lock);
    SCOPED_LOCK(InterProcessLock, m_sharedProcessLock);
    MMBuffer data = getDataForKey(key);
    if (data.length() > 0) {
        try {
            CodedInputData input(data.getPtr(), data.length());
            if (hasValue != NULL) {
                *hasValue = true;
            }
            return input.readFloat();
        } catch (std::exception &exception) {
            MMKVError("%s", exception.what());
        }
    }
    if (hasValue != NULL) {
        *hasValue = false;
    }
    return defaultValue;
}

double MMKV::getDouble(MMKVKey_t key, double defaultValue, bool *hasValue) {
    if (isKeyEmpty(key)) {
        if (hasValue != NULL) {
            *hasValue = false;
        }
        return defaultValue;
    }
    SCOPED_LOCK(ThreadLock, m_lock);
    SCOPED_LOCK(InterProcessLock, m_sharedProcessLock);
    MMBuffer data = getDataForKey(key);
    if (data.length() > 0) {
        try {
            CodedInputData input(data.getPtr(), data.length());
            if (hasValue != NULL) {
                *hasValue = true;
            }
            return input.readDouble();
        } catch (std::exception &exception) {
            MMKVError("%s", exception.what());
        }
    }
    if (hasValue != NULL) {
        *hasValue = false;
    }
    return defaultValue;
}

size_t MMKV::getValueSize(MMKVKey_t key, bool actualSize) {
    if (isKeyEmpty(key)) {
        return 0;
    }
    SCOPED_LOCK(ThreadLock, m_lock);
    SCOPED_LOCK(InterProcessLock, m_sharedProcessLock);
    MMBuffer data = getDataForKey(key);
    if (actualSize) {
        try {
            CodedInputData input(data.getPtr(), data.length());
            INT length = input.readInt32();
            if (length >= 0) {
                size_t s_length = static_cast<size_t>(length);
                if (pbRawVarint32Size(length) + s_length == data.length()) {
                    return s_length;
                }
            }
        } catch (std::exception &exception) {
            MMKVError("%s", exception.what());
        }
    }
    return data.length();
}

INT MMKV::writeValueToBuffer(MMKVKey_t key, void *ptr, INT size) {
    if (isKeyEmpty(key) || size < 0) {
        return -1;
    }
    size_t s_size = static_cast<size_t>(size);

    SCOPED_LOCK(ThreadLock, m_lock);
    SCOPED_LOCK(InterProcessLock, m_sharedProcessLock);
    MMBuffer data = getDataForKey(key);
    try {
        CodedInputData input(data.getPtr(), data.length());
        INT length = input.readInt32();
        DWORD offset = pbRawVarint32Size(length);
        if (length >= 0) {
            size_t s_length = static_cast<size_t>(length);
            if (offset + s_length == data.length()) {
                if (s_length <= s_size) {
                    memcpy(ptr, (BYTE *) data.getPtr() + offset, s_length);
                    return length;
                }
            } else {
                if (data.length() <= s_size) {
                    memcpy(ptr, data.getPtr(), data.length());
                    return static_cast<INT>(data.length());
                }
            }
        }
    } catch (std::exception &exception) {
        MMKVError("%s", exception.what());
    }
    return -1;
}

// enumerate

bool MMKV::containsKey(MMKVKey_t key) {
    SCOPED_LOCK(ThreadLock, m_lock);
    checkLoadData();

    if (m_crypter) {
        return m_dicCrypt->find(key) != m_dicCrypt->end();
    } else {
        return m_dic->find(key) != m_dic->end();
    }
}

size_t MMKV::count() {
    SCOPED_LOCK(ThreadLock, m_lock);
    checkLoadData();
    if (m_crypter) {
        return m_dicCrypt->size();
    } else {
        return m_dic->size();
    }
}

size_t MMKV::totalSize() {
    SCOPED_LOCK(ThreadLock, m_lock);
    checkLoadData();
    return m_file->getFileSize();
}

size_t MMKV::actualSize() {
    SCOPED_LOCK(ThreadLock, m_lock);
    checkLoadData();
    return m_actualSize;
}

void MMKV::removeValueForKey(MMKVKey_t key) {
    if (isKeyEmpty(key)) {
        return;
    }
    SCOPED_LOCK(ThreadLock, m_lock);
    SCOPED_LOCK(InterProcessLock, m_exclusiveProcessLock);
    checkLoadData();

    removeDataForKey(key);
}



vector<string> MMKV::allKeys() {
    SCOPED_LOCK(ThreadLock, m_lock);
    checkLoadData();
    vector<string> keys;
    if (m_crypter) {
        mmkv::MMKVMapCrypt::const_iterator itr = m_dicCrypt->begin(); 
        for (; itr != m_dicCrypt->end(); ++itr) {
            keys.push_back(itr->first);
        }
    } else {
        MMKVMap::const_iterator itr = m_dic->begin();
        for (; itr != m_dic->end(); itr++ ) {
            keys.push_back(itr->first);
        }
    }
    return keys;
}

void MMKV::removeValuesForKeys(const vector<string> &arrKeys) {
    if (arrKeys.empty()) {
        return;
    }
    if (arrKeys.size() == 1) {
        return removeValueForKey(arrKeys[0]);
    }

    SCOPED_LOCK(ThreadLock, m_lock);
    SCOPED_LOCK(InterProcessLock, m_exclusiveProcessLock);
    checkLoadData();

    size_t deleteCount = 0;
    if (m_crypter) {
        for(size_t i = 0; i < arrKeys.size(); i++) {
            const string& key = arrKeys[i];
            mmkv::MMKVMapCrypt::iterator itr = m_dicCrypt->find(key);
            if (itr != m_dicCrypt->end()) {
                m_dicCrypt->erase(itr);
                deleteCount++;
            }
        }
    } else {
        for(size_t i = 0; i < arrKeys.size(); i++) {
            const string& key = arrKeys[i];
            mmkv::MMKVMapCrypt::iterator itr = m_dic->find(key);
            if (itr != m_dic->end()) {
                m_dic->erase(itr);
                deleteCount++;
            }
        }
    }
    if (deleteCount > 0) {
        m_hasFullWriteback = false;

        fullWriteback();
    }
}



// file

void MMKV::sync(SyncFlag flag) {
    SCOPED_LOCK(ThreadLock, m_lock);
    if (m_needLoadFromFile || !isFileValid()) {
        return;
    }
    SCOPED_LOCK(InterProcessLock, m_exclusiveProcessLock);

    m_file->msync(flag);
    m_metaFile->msync(flag);
}

void MMKV::lock() {
    SCOPED_LOCK(ThreadLock, m_lock);
    m_exclusiveProcessLock->lock();
}
void MMKV::unlock() {
    SCOPED_LOCK(ThreadLock, m_lock);
    m_exclusiveProcessLock->unlock();
}
bool MMKV::try_lock() {
    SCOPED_LOCK(ThreadLock, m_lock);
    return m_exclusiveProcessLock->try_lock();
}

// backup

static bool backupOneToDirectoryByFilePath(const string &mmapKey, const MMKVPath_t &srcPath, const MMKVPath_t &dstPath) {
    File crcFile(srcPath, mmkv::ReadOnly);
    if (!crcFile.isFileValid()) {
        return false;
    }

    bool ret = false;
    {
        MMKVInfo("backup one mmkv[%s] from [%ws] to [%ws]", mmapKey.c_str(), srcPath.c_str(), dstPath.c_str());
        FileLock fileLock(crcFile.getFd());
        InterProcessLock lock(&fileLock, SharedLockType);
        SCOPED_LOCK(InterProcessLock, &lock);

        ret = copyFile(srcPath, dstPath);
        if (ret) {
            MMKVPath_t srcCRCPath = srcPath + CRC_SUFFIX;
            MMKVPath_t dstCRCPath = dstPath + CRC_SUFFIX;
            ret = copyFile(srcCRCPath, dstCRCPath);
        }
        MMKVInfo("finish backup one mmkv[%s]", mmapKey.c_str());
    }
    return ret;
}

bool MMKV::backupOneToDirectory(const string &mmapKey, const MMKVPath_t &dstPath, const MMKVPath_t &srcPath, bool compareFullPath) {
    // we have to lock the creation of MMKV instance, regardless of in cache or not
    SCOPED_LOCK(ThreadLock, g_instanceLock);
    MMKV *kv = NULL;
    if (!compareFullPath) {
        unordered_map<string, MMKV *>::iterator itr = g_instanceDic->find(mmapKey);
        if (itr != g_instanceDic->end()) {
            kv = itr->second;
        }
    } else {
        // mmapKey is actually filename, we can't simply call find()
        unordered_map<string, MMKV *>::iterator itr = g_instanceDic->begin();
        for(; itr != g_instanceDic->end(); ++itr) {
        //for (auto &pair : *g_instanceDic) {
            if (itr->second->m_path == srcPath) {
                kv = itr->second;
                break;
            }
        }
    }
    // get one in cache, do it the easy way
    if (kv) {
        MMKVInfo("backup one cached mmkv[%s] from [%ws] to [%ws]", mmapKey.c_str(), srcPath.c_str(), dstPath.c_str());
        SCOPED_LOCK(ThreadLock, kv->m_lock);
        SCOPED_LOCK(InterProcessLock, kv->m_sharedProcessLock);

        kv->sync();
        bool ret = copyFile(kv->m_path, dstPath);
        if (ret) {
            MMKVPath_t dstCRCPath = dstPath + CRC_SUFFIX;
            ret = copyFile(kv->m_crcPath, dstCRCPath);
        }
        MMKVInfo("finish backup one mmkv[%s], ret: %d", mmapKey.c_str(), ret);
        return ret;
    }

    // no luck with cache, do it the hard way
    bool ret = backupOneToDirectoryByFilePath(mmapKey, srcPath, dstPath);
    return ret;
}

bool MMKV::backupOneToDirectory(const string &mmapID, const MMKVPath_t &dstDir, const MMKVPath_t *srcDir) {
    const MMKVPath_t* rootPath = srcDir ? srcDir : &g_rootDir;
    if (*rootPath == dstDir) {
        return true;
    }
    mkPath(dstDir);
    MMKVPath_t encodePath = encodeFilePath(mmapID, dstDir);
    MMKVPath_t dstPath = dstDir + MMKV_PATH_SLASH + encodePath;
    std::string mmapKey = mmapedKVKey(mmapID, rootPath);
    MMKVPath_t srcPath = *rootPath + MMKV_PATH_SLASH + encodePath;
    return backupOneToDirectory(mmapKey, dstPath, srcPath, false);
}

bool endsWith(const MMKVPath_t &str, const MMKVPath_t &suffix) {
    if (str.length() >= suffix.length()) {
        return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
    } else {
        return false;
    }
}

MMKVPath_t filename(const MMKVPath_t &path) {
    size_t startPos = path.rfind(MMKV_PATH_SLASH);
    startPos++; // don't need to check for npos, because npos+1 == 0
    MMKVPath_t filename = path.substr(startPos);
    return filename;
}


struct walkInDirFuncObj {
    walkInDirFuncObj(){}

    void walkInDirFunc(const MMKVPath_t &filePath, WalkType t) {
        if (endsWith(filePath, CRC_SUFFIX)) {
            mmapIDCRCSet->insert(filePath);
        } else {
            mmapIDSet->insert(filePath);
        }    
    }
    unordered_set<MMKVPath_t>* mmapIDSet;
    unordered_set<MMKVPath_t>* mmapIDCRCSet;
};

size_t MMKV::backupAllToDirectory(const MMKVPath_t &dstDir, const MMKVPath_t &srcDir, bool isInSpecialDir) {
    unordered_set<MMKVPath_t> mmapIDSet;
    unordered_set<MMKVPath_t> mmapIDCRCSet;
    walkInDirFuncObj fo;
    fo.mmapIDSet = &mmapIDSet;
    fo.mmapIDCRCSet = &mmapIDCRCSet;
    walkInDir(srcDir, WalkFile, std::tr1::bind(&walkInDirFuncObj::walkInDirFunc, 
        &fo, placeholders::_1, placeholders::_2)
        
        /*[&](const MMKVPath_t &filePath, WalkType) {
        if (endsWith(filePath, CRC_SUFFIX)) {
            mmapIDCRCSet.insert(filePath);
        } else {
            mmapIDSet.insert(filePath);
        }
    }*/
    );

    size_t count = 0;
    if (!mmapIDSet.empty()) {
        mkPath(dstDir);
        bool compareFullPath = isInSpecialDir;
        for(unordered_set<MMKVPath_t>::iterator iter = mmapIDSet.begin(); iter != mmapIDSet.end(); ++iter) {
            MMKVPath_t &srcPath = *iter;
            MMKVPath_t srcCRCPath = srcPath + CRC_SUFFIX;
            if (mmapIDCRCSet.find(srcCRCPath) == mmapIDCRCSet.end()) {
                MMKVWarning("crc not exist [%ws]", srcCRCPath.c_str());
                continue;
            }
            MMKVPath_t basename = filename(srcPath);
            const std::string &strBasename = MMKVPath_t2String(basename);
            std::string mmapKey = isInSpecialDir ? strBasename : mmapedKVKey(strBasename, &srcDir);
            MMKVPath_t dstPath = dstDir + MMKV_PATH_SLASH + basename;
            if (backupOneToDirectory(mmapKey, dstPath, srcPath, compareFullPath)) {
                count++;
            }
        }
    }
    return count;
}

size_t MMKV::backupAllToDirectory(const MMKVPath_t &dstDir, const MMKVPath_t *srcDir) {
    const MMKVPath_t* rootPath = srcDir ? srcDir : &g_rootDir;
    if (*rootPath == dstDir) {
        return true;
    }
    size_t count = backupAllToDirectory(dstDir, *rootPath, false);

    MMKVPath_t specialSrcDir = *rootPath + MMKV_PATH_SLASH + SPECIAL_CHARACTER_DIRECTORY_NAME;
    if (isFileExist(specialSrcDir)) {
        MMKVPath_t specialDstDir = dstDir + MMKV_PATH_SLASH + SPECIAL_CHARACTER_DIRECTORY_NAME;
        count += backupAllToDirectory(specialDstDir, specialSrcDir, true);
    }
    return count;
}

// restore

static bool restoreOneFromDirectoryByFilePath(const string &mmapKey, const MMKVPath_t &srcPath, const MMKVPath_t &dstPath) {
    MMKVPath_t dstCRCPath = dstPath + CRC_SUFFIX;
    File dstCRCFile(dstCRCPath, mmkv::ReadWrite | mmkv::Create);
    if (!dstCRCFile.isFileValid()) {
        return false;
    }

    bool ret = false;
    {
        MMKVInfo("restore one mmkv[%s] from [%ws] to [%ws]", mmapKey.c_str(), srcPath.c_str(), dstPath.c_str());
        FileLock fileLock(dstCRCFile.getFd());
        InterProcessLock lock(&fileLock, ExclusiveLockType);
        SCOPED_LOCK(InterProcessLock, &lock);

        ret = copyFileContent(srcPath, dstPath);
        if (ret) {
            MMKVPath_t srcCRCPath = srcPath + CRC_SUFFIX;
            ret = copyFileContent(srcCRCPath, dstCRCFile.getFd());
        }
        MMKVInfo("finish restore one mmkv[%s]", mmapKey.c_str());
    }
    return ret;
}

// We can't simply replace the existing file, because other processes might have already open it.
// They won't know a difference when the file has been replaced.
// We have to let them know by overriding the existing file with new content.
bool MMKV::restoreOneFromDirectory(const string &mmapKey, const MMKVPath_t &srcPath, const MMKVPath_t &dstPath, bool compareFullPath) {
    // we have to lock the creation of MMKV instance, regardless of in cache or not
    SCOPED_LOCK(ThreadLock, g_instanceLock);
    MMKV *kv = NULL;
    if (!compareFullPath) {
        unordered_map<string, MMKV *>::iterator itr = g_instanceDic->find(mmapKey);
        if (itr != g_instanceDic->end()) {
            kv = itr->second;
        }
    } else {
        // mmapKey is actually filename, we can't simply call find()
        unordered_map<string, MMKV *>::iterator itr = g_instanceDic->begin();
        for(; itr != g_instanceDic->end(); ++itr) {
            if (itr->second->m_path == dstPath) {
                kv = itr->second;
                break;
            }
        }
    }
    // get one in cache, do it the easy way
    if (kv) {
        MMKVInfo("restore one cached mmkv[%s] from [%ws] to [%ws]", mmapKey.c_str(), srcPath.c_str(), dstPath.c_str());
        SCOPED_LOCK(ThreadLock, kv->m_lock);
        SCOPED_LOCK(InterProcessLock, kv->m_exclusiveProcessLock);

        kv->sync();
        bool ret = copyFileContent(srcPath, kv->m_file->getFd());
        if (ret) {
            MMKVPath_t srcCRCPath = srcPath + CRC_SUFFIX;
            ret = copyFileContent(srcCRCPath, kv->m_metaFile->getFd());
        }

        // reload data after restore
        kv->clearMemoryCache();
        kv->loadFromFile();
        if (kv->m_isInterProcess) {
            kv->notifyContentChanged();
        }

        MMKVInfo("finish restore one mmkv[%s], ret: %d", mmapKey.c_str(), ret);
        return ret;
    }

    // no luck with cache, do it the hard way
    bool ret = restoreOneFromDirectoryByFilePath(mmapKey, srcPath, dstPath);
    return ret;
}

bool MMKV::restoreOneFromDirectory(const string &mmapID, const MMKVPath_t &srcDir, const MMKVPath_t *dstDir) {
    const MMKVPath_t* rootPath = dstDir ? dstDir : &g_rootDir;
    if (*rootPath == srcDir) {
        return true;
    }
    mkPath(*rootPath);
    MMKVPath_t encodePath = encodeFilePath(mmapID, *rootPath);
    MMKVPath_t srcPath = srcDir + MMKV_PATH_SLASH + encodePath;
    std::string mmapKey = mmapedKVKey(mmapID, rootPath);

    MMKVPath_t dstPath = *rootPath + MMKV_PATH_SLASH + encodePath;

    return restoreOneFromDirectory(mmapKey, srcPath, dstPath, false);
}

size_t MMKV::restoreAllFromDirectory(const MMKVPath_t &srcDir, const MMKVPath_t &dstDir, bool isInSpecialDir) {
    unordered_set<MMKVPath_t> mmapIDSet;
    unordered_set<MMKVPath_t> mmapIDCRCSet;

    walkInDirFuncObj fo;
    fo.mmapIDSet = &mmapIDSet;
    fo.mmapIDCRCSet = &mmapIDCRCSet;
    walkInDir(srcDir, WalkFile, std::tr1::bind(&walkInDirFuncObj::walkInDirFunc, 
        &fo, placeholders::_1, placeholders::_2)
    );

    /*
    walkInDir(srcDir, WalkFile, [&](const MMKVPath_t &filePath, WalkType) {
        if (endsWith(filePath, CRC_SUFFIX)) {
            mmapIDCRCSet.insert(filePath);
        } else {
            mmapIDSet.insert(filePath);
        }
    });
    */

    size_t count = 0;
    if (!mmapIDSet.empty()) {
        mkPath(dstDir);
        bool compareFullPath = isInSpecialDir;
        unordered_set<MMKVPath_t>::iterator iter = mmapIDSet.begin();
        for(; iter != mmapIDSet.end(); ++iter) {
            const MMKVPath_t& srcPath = *iter;
            MMKVPath_t srcCRCPath = srcPath + CRC_SUFFIX;
            if (mmapIDCRCSet.find(srcCRCPath) == mmapIDCRCSet.end()) {
                MMKVWarning("crc not exist [%ws]", srcCRCPath.c_str());
                continue;
            }
            MMKVPath_t basename = filename(srcPath);
            const std::string &strBasename = MMKVPath_t2String(basename);
            std::string mmapKey = isInSpecialDir ? strBasename : mmapedKVKey(strBasename, &dstDir);
            MMKVPath_t dstPath = dstDir + MMKV_PATH_SLASH + basename;
            if (restoreOneFromDirectory(mmapKey, srcPath, dstPath, compareFullPath)) {
                count++;
            }
        }
    }
    return count;
}

size_t MMKV::restoreAllFromDirectory(const MMKVPath_t &srcDir, const MMKVPath_t *dstDir) {
    const MMKVPath_t* rootPath = dstDir ? dstDir : &g_rootDir;
    if (*rootPath == srcDir) {
        return true;
    }
    size_t count = restoreAllFromDirectory(srcDir, *rootPath, true);

    MMKVPath_t specialSrcDir = srcDir + MMKV_PATH_SLASH + SPECIAL_CHARACTER_DIRECTORY_NAME;
    if (isFileExist(specialSrcDir)) {
        MMKVPath_t specialDstDir = *rootPath + MMKV_PATH_SLASH + SPECIAL_CHARACTER_DIRECTORY_NAME;
        count += restoreAllFromDirectory(specialSrcDir, specialDstDir, false);
    }
    return count;
}

// callbacks

void MMKV::registerErrorHandler(ErrorHandler handler) {
    SCOPED_LOCK(ThreadLock, g_instanceLock);
    g_errorHandler = handler;
}

void MMKV::unRegisterErrorHandler() {
    SCOPED_LOCK(ThreadLock, g_instanceLock);
    g_errorHandler = NULL;
}

void MMKV::registerLogHandler(LogHandler handler) {
    SCOPED_LOCK(ThreadLock, g_instanceLock);
    g_logHandler = handler;
}

void MMKV::unRegisterLogHandler() {
    SCOPED_LOCK(ThreadLock, g_instanceLock);
    g_logHandler = NULL;
}

void MMKV::setLogLevel(MMKVLogLevel level) {
    SCOPED_LOCK(ThreadLock, g_instanceLock);
    g_currentLogLevel = level;
}

static void mkSpecialCharacterFileDirectory() {
    MMKVPath_t path = g_rootDir + MMKV_PATH_SLASH + SPECIAL_CHARACTER_DIRECTORY_NAME;
    mkPath(path);
}

#pragma comment(lib, "Advapi32.lib")

static BOOL MD5Digest(const BYTE* csBuffer, DWORD dwLen, BYTE* out) {
    BOOL result = FALSE;
	HCRYPTPROV hCryptProv; 
	HCRYPTHASH hHash; 
    BYTE bHash[MD5_DIGEST_LENGTH] = {'\0'}; 
	DWORD dwHashLen= MD5_DIGEST_LENGTH; // The MD5 algorithm always returns 16 bytes. 
	DWORD cbContent=dwLen;
	BYTE* pbContent= new BYTE[dwLen];
	memcpy(pbContent,csBuffer,dwLen);
    do {
	    if(CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 
                    CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET))  {
		    if(CryptCreateHash(hCryptProv, 
			    CALG_MD5,	// algorithm identifier definitions see: wincrypt.h
			    0, 0, &hHash)) {
			    if(CryptHashData(hHash, pbContent, cbContent, 0)) {
 				    if(CryptGetHashParam(hHash, HP_HASHVAL, bHash, &dwHashLen, 0)) {
                        result = TRUE;
                        break;
 				    }
			    }
		    }
	    }
    } while(false);
	CryptDestroyHash(hHash); 
	CryptReleaseContext(hCryptProv, 0); 
	delete[] pbContent;
    memcpy(out, bHash, MD5_DIGEST_LENGTH); 
	return result;
}

template <typename T>
static string md5(const basic_string<T> &value) {
    BYTE md[MD5_DIGEST_LENGTH] = {'\0'};
    char tmp[3] = {'\0'};
    std::string result;
    MD5Digest((const BYTE *) value.c_str(), value.size() * (sizeof(T) / sizeof(BYTE)), (BYTE*)md);
    for (size_t i = 0; i < MD5_DIGEST_LENGTH; i++) {
        BYTE& ch = md[i];
        sprintf_s(tmp, sizeof(tmp), "%2.2x", ch);
        result.append((const char*)tmp);
    }
    return result;
}

static MMKVPath_t encodeFilePath(const string &mmapID) {
    const char *specialCharacters = "\\/:*?\"<>|";
    string encodedID;
    bool hasSpecialCharacter = false;
    for(size_t i = 0; i < mmapID.size(); i++) {
        const char& ch = mmapID[i];
        if (strchr(specialCharacters, ch) != NULL) {
            encodedID = md5(mmapID);
            hasSpecialCharacter = true;
            break;
        }
    }
    if (hasSpecialCharacter) {
        static ThreadOnceToken_t once_control = ThreadOnceUninitialized;
        ThreadLock::ThreadOnce(&once_control, mkSpecialCharacterFileDirectory);
        return MMKVPath_t(SPECIAL_CHARACTER_DIRECTORY_NAME) + MMKV_PATH_SLASH + string2MMKVPath_t(encodedID);
    } else {
        return string2MMKVPath_t(mmapID);
    }
}

static MMKVPath_t encodeFilePath(const string &mmapID, const MMKVPath_t &rootDir) {
    const char *specialCharacters = "\\/:*?\"<>|";
    string encodedID;
    bool hasSpecialCharacter = false;
    for(size_t i = 0; i < mmapID.size(); i++) {
        const char& ch = mmapID[i];
        if (strchr(specialCharacters, ch) != NULL) {
            encodedID = md5(mmapID);
            hasSpecialCharacter = true;
            break;
        }
    }
    if (hasSpecialCharacter) {
        MMKVPath_t path = rootDir + MMKV_PATH_SLASH + SPECIAL_CHARACTER_DIRECTORY_NAME;
        mkPath(path);

        return MMKVPath_t(SPECIAL_CHARACTER_DIRECTORY_NAME) + MMKV_PATH_SLASH + string2MMKVPath_t(encodedID);
    } else {
        return string2MMKVPath_t(mmapID);
    }
}

string mmapedKVKey(const string &mmapID, const MMKVPath_t *rootPath) {
    if (rootPath && g_rootDir != (*rootPath)) {
        return md5(*rootPath + MMKV_PATH_SLASH + string2MMKVPath_t(mmapID));
    }
    return mmapID;
}

MMKVPath_t mappedKVPathWithID(const string &mmapID, MMKVMode mode, const MMKVPath_t *rootPath) {
#ifndef MMKV_ANDROID
    if (rootPath) {
#else
    if (mode & MMKV_ASHMEM) {
        return ashmemMMKVPathWithID(encodeFilePath(mmapID));
    } else if (rootPath) {
#endif
        return *rootPath + MMKV_PATH_SLASH + encodeFilePath(mmapID);
    }
    return g_rootDir + MMKV_PATH_SLASH + encodeFilePath(mmapID);
}

MMKVPath_t crcPathWithID(const string &mmapID, MMKVMode mode, const MMKVPath_t *rootPath) {
    if (rootPath) {
        return *rootPath + MMKV_PATH_SLASH + encodeFilePath(mmapID) + CRC_SUFFIX;
    }
    return g_rootDir + MMKV_PATH_SLASH + encodeFilePath(mmapID) + CRC_SUFFIX;
}

MMKVRecoverStrategic onMMKVCRCCheckFail(const string &mmapID) {
    if (g_errorHandler) {
        return g_errorHandler(mmapID, MMKVErrorType::MMKVCRCCheckFail);
    }
    return OnErrorDiscard;
}

MMKVRecoverStrategic onMMKVFileLengthError(const string &mmapID) {
    if (g_errorHandler) {
        return g_errorHandler(mmapID, MMKVErrorType::MMKVFileLength);
    }
    return OnErrorDiscard;
}

MMKV_NAMESPACE_END
