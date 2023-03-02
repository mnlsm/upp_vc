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

#include "MiniPBCoder.h"
#include "CodedInputData.h"
#include "CodedInputDataCrypt.h"
#include "CodedOutputData.h"
#include "PBEncodeItem.hpp"



using namespace std;

namespace mmkv {

MiniPBCoder::MiniPBCoder() : m_encodeItems(new std::vector<PBEncodeItem>()) {
    m_inputBuffer = NULL;
    m_inputData = NULL;
    m_inputDataDecrpt = NULL;

    m_outputBuffer = NULL;
    m_outputData = NULL;
    m_encodeItems = NULL;
}

MiniPBCoder::MiniPBCoder(const MMBuffer *inputBuffer, AESCrypt *crypter) {
    m_inputBuffer = NULL;
    m_inputData = NULL;
    m_inputDataDecrpt = NULL;

    m_outputBuffer = NULL;
    m_outputData = NULL;
    m_encodeItems = NULL;

    m_inputBuffer = inputBuffer;
    m_inputData = new CodedInputData(m_inputBuffer->getPtr(), m_inputBuffer->length());
}

MiniPBCoder::~MiniPBCoder() {
    delete m_inputData;
    delete m_outputBuffer;
    delete m_outputData;
    delete m_encodeItems;
}

// encode

// write object using prepared m_encodeItems[]
void MiniPBCoder::writeRootObject() {
    for (size_t index = 0, total = m_encodeItems->size(); index < total; index++) {
        PBEncodeItem *encodeItem = &(*m_encodeItems)[index];
        switch (encodeItem->type) {
            case PBEncodeItemType_Data: {
                m_outputData->writeData(*(encodeItem->value.bufferValue));
                break;
            }
            case PBEncodeItemType_Container: {
                m_outputData->writeUInt32(encodeItem->valueSize);
                break;
            }
            case PBEncodeItemType_String: {
                m_outputData->writeString(*(encodeItem->value.strValue));
                break;
            }
            case PBEncodeItemType_None: {
                MMKVError("%d", encodeItem->type);
                break;
            }
        }
    }
}

size_t MiniPBCoder::prepareObjectForEncode(const MMBuffer &buffer) {
    m_encodeItems->push_back(PBEncodeItem());
    PBEncodeItem *encodeItem = &(m_encodeItems->back());
    size_t index = m_encodeItems->size() - 1;
    {
        encodeItem->type = PBEncodeItemType_Data;
        encodeItem->value.bufferValue = &buffer;
        encodeItem->valueSize = static_cast<DWORD>(buffer.length());
    }
    encodeItem->compiledSize = pbRawVarint32Size(encodeItem->valueSize) + encodeItem->valueSize;

    return index;
}

MMBuffer MiniPBCoder::writePreparedItems(size_t index) {
    PBEncodeItem *oItem = (index < m_encodeItems->size()) ? &(*m_encodeItems)[index] : NULL;
    if (oItem && oItem->compiledSize > 0) {
        m_outputBuffer = new MMBuffer(oItem->compiledSize);
        m_outputData = new CodedOutputData(m_outputBuffer->getPtr(), m_outputBuffer->length());

        writeRootObject();
    }

    return *m_outputBuffer;
}

MMBuffer MiniPBCoder::encodeDataWithObject(const MMBuffer &obj) {
    try {
        DWORD valueSize = static_cast<DWORD>(obj.length());
        DWORD compiledSize = pbRawVarint32Size(valueSize) + valueSize;
        MMBuffer result(compiledSize);
        CodedOutputData output(result.getPtr(), result.length());
        output.writeData(obj);
        return result;
    } catch (const std::exception &exception) {
        MMKVError("%s", exception.what());
        return MMBuffer();
    }
}


size_t MiniPBCoder::prepareObjectForEncode(const string &str) {
    m_encodeItems->push_back(PBEncodeItem());
    PBEncodeItem *encodeItem = &(m_encodeItems->back());
    size_t index = m_encodeItems->size() - 1;
    {
        encodeItem->type = PBEncodeItemType_String;
        encodeItem->value.strValue = &str;
        encodeItem->valueSize = static_cast<INT>(str.size());
    }
    encodeItem->compiledSize = pbRawVarint32Size(encodeItem->valueSize) + encodeItem->valueSize;

    return index;
}

size_t MiniPBCoder::prepareObjectForEncode(const vector<string> &v) {
    m_encodeItems->push_back(PBEncodeItem());
    PBEncodeItem *encodeItem = &(m_encodeItems->back());
    size_t index = m_encodeItems->size() - 1;
    {
        encodeItem->type = PBEncodeItemType_Container;
        encodeItem->value.bufferValue = NULL;

        for (vector<string>::const_iterator iter = v.begin(); iter != v.end(); ++iter) {
            size_t itemIndex = prepareObjectForEncode(*iter);
            if (itemIndex < m_encodeItems->size()) {
                (*m_encodeItems)[index].valueSize += (*m_encodeItems)[itemIndex].compiledSize;
            }
        }

        encodeItem = &(*m_encodeItems)[index];
    }
    encodeItem->compiledSize = pbRawVarint32Size(encodeItem->valueSize) + encodeItem->valueSize;

    return index;
}

vector<string> MiniPBCoder::decodeOneVector() {
    vector<string> v;

    m_inputData->readInt32();

    while (!m_inputData->isAtEnd()) {
        std::string value = m_inputData->readString();
        v.push_back(value);
    }

    return v;
}

void MiniPBCoder::decodeOneMap1(size_t position, MMKVMap &dictionary) {
    if (position) {
        m_inputData->seek(position);
    } else {
        m_inputData->readInt32();
    }
    while (!m_inputData->isAtEnd()) {
        KeyValueHolder kvHolder;
        const std::string &key = m_inputData->readString(kvHolder);
        if (key.length() > 0) {
            m_inputData->readData(kvHolder);
            if (kvHolder.valueSize > 0) {
                dictionary[key] = kvHolder;
            } else {
                MMKVMap::iterator itr = dictionary.find(key);
                if (itr != dictionary.end()) {
                    dictionary.erase(itr);
                }
            }
        }
    }
}

void MiniPBCoder::decodeOneMap(MMKVMap &dic, size_t position, bool greedy) {
    if (greedy) {
        try {
            decodeOneMap1(position, dic);
        } catch (std::exception &exception) {
            MMKVError("%s", exception.what());
        }
    } else {
        try {
            MMKVMap tmpDic;
            decodeOneMap1(position,tmpDic);
            dic.swap(tmpDic);
        } catch (std::exception &exception) {
            MMKVError("%s", exception.what());
        }
    }
}



vector<string> MiniPBCoder::decodeVector(const MMBuffer &oData) {
    MiniPBCoder oCoder(&oData);
    return oCoder.decodeOneVector();
}



void MiniPBCoder::decodeMap(MMKVMap &dic, const MMBuffer &oData, size_t position) {
    MiniPBCoder oCoder(&oData);
    oCoder.decodeOneMap(dic, position, false);
}

void MiniPBCoder::greedyDecodeMap(MMKVMap &dic, const MMBuffer &oData, size_t position) {
    MiniPBCoder oCoder(&oData);
    oCoder.decodeOneMap(dic, position, true);
}


} // namespace mmkv
