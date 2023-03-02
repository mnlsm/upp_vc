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
#include "PBUtility.h"
#include <stdexcept>
#include <sstream>

using namespace std;

namespace mmkv {

CodedInputData::CodedInputData(const void *oData, size_t length)
    : m_ptr((BYTE *) oData), m_size(length), m_position(0) {
    MMKV_ASSERT(m_ptr);
}

void CodedInputData::seek(size_t addedSize) {
    if (m_position + addedSize > m_size) {
        throw out_of_range("OutOfSpace");
    }
    m_position += addedSize;
}

double CodedInputData::readDouble() {
    return Int64ToFloat64(this->readRawLittleEndian64());
}

float CodedInputData::readFloat() {
    return Int32ToFloat32(this->readRawLittleEndian32());
}

__int64 CodedInputData::readInt64() {
    INT shift = 0;
    __int64 result = 0;
    while (shift < 64) {
        char b = this->readRawByte();
        result |= (__int64)(b & 0x7f) << shift;
        if ((b & 0x80) == 0) {
            return result;
        }
        shift += 7;
    }
    throw invalid_argument("InvalidProtocolBuffer malformedInt64");
}

unsigned __int64 CodedInputData::readUInt64() {
    return static_cast<unsigned __int64>(readInt64());
}

INT CodedInputData::readInt32() {
    return this->readRawVarint32();
}

DWORD CodedInputData::readUInt32() {
    return static_cast<DWORD>(readRawVarint32());
}

bool CodedInputData::readBool() {
    return this->readRawVarint32() != 0;
}

string CodedInputData::readString() {
    INT size = readRawVarint32();
    if (size < 0) {
        throw length_error("InvalidProtocolBuffer negativeSize");
    }

    size_t s_size = static_cast<size_t>(size);
    if (s_size <= m_size - m_position) {
        string result((char *) (m_ptr + m_position), s_size);
        m_position += s_size;
        return result;
    } else {
        throw out_of_range("InvalidProtocolBuffer truncatedMessage");
    }
}

string CodedInputData::readString(KeyValueHolder &kvHolder) {
    kvHolder.offset = static_cast<DWORD>(m_position);

    INT size = this->readRawVarint32();
    if (size < 0) {
        throw length_error("InvalidProtocolBuffer negativeSize");
    }

    size_t s_size = static_cast<size_t>(size);
    if (s_size <= m_size - m_position) {
        kvHolder.keySize = static_cast<unsigned short>(s_size);

        BYTE* ptr = m_ptr + m_position;
        string result((char *) (m_ptr + m_position), s_size);
        m_position += s_size;
        return result;
    } else {
        throw out_of_range("InvalidProtocolBuffer truncatedMessage");
    }
}

MMBuffer CodedInputData::readData() {
    INT size = this->readRawVarint32();
    if (size < 0) {
        throw length_error("InvalidProtocolBuffer negativeSize");
    }

    size_t s_size = static_cast<size_t>(size);
    if (s_size <= m_size - m_position) {
        MMBuffer data(((char *) m_ptr) + m_position, s_size);
        m_position += s_size;
        return data;
    } else {
        throw out_of_range("InvalidProtocolBuffer truncatedMessage");
    }
}

void CodedInputData::readData(KeyValueHolder &kvHolder) {
    INT size = this->readRawVarint32();
    if (size < 0) {
        throw length_error("InvalidProtocolBuffer negativeSize");
    }

    size_t s_size = static_cast<size_t>(size);
    if (s_size <= m_size - m_position) {
        kvHolder.computedKVSize = static_cast<unsigned short>(m_position - kvHolder.offset);
        kvHolder.valueSize = static_cast<DWORD>(s_size);

        m_position += s_size;
    } else {
        throw out_of_range("InvalidProtocolBuffer truncatedMessage");
    }
}

INT CodedInputData::readRawVarint32() {
    char tmp = this->readRawByte();
    if (tmp >= 0) {
        return tmp;
    }
    INT result = tmp & 0x7f;
    if ((tmp = this->readRawByte()) >= 0) {
        result |= tmp << 7;
    } else {
        result |= (tmp & 0x7f) << 7;
        if ((tmp = this->readRawByte()) >= 0) {
            result |= tmp << 14;
        } else {
            result |= (tmp & 0x7f) << 14;
            if ((tmp = this->readRawByte()) >= 0) {
                result |= tmp << 21;
            } else {
                result |= (tmp & 0x7f) << 21;
                result |= (tmp = this->readRawByte()) << 28;
                if (tmp < 0) {
                    // discard upper 32 bits
                    for (int i = 0; i < 5; i++) {
                        if (this->readRawByte() >= 0) {
                            return result;
                        }
                    }
                    throw invalid_argument("InvalidProtocolBuffer malformed varint32");
                }
            }
        }
    }
    return result;
}

INT CodedInputData::readRawLittleEndian32() {
    char b1 = this->readRawByte();
    char b2 = this->readRawByte();
    char b3 = this->readRawByte();
    char b4 = this->readRawByte();
    return (((INT) b1 & 0xff)) | (((INT) b2 & 0xff) << 8) | (((INT) b3 & 0xff) << 16) |
           (((INT) b4 & 0xff) << 24);
}

__int64 CodedInputData::readRawLittleEndian64() {
    char b1 = this->readRawByte();
    char b2 = this->readRawByte();
    char b3 = this->readRawByte();
    char b4 = this->readRawByte();
    char b5 = this->readRawByte();
    char b6 = this->readRawByte();
    char b7 = this->readRawByte();
    char b8 = this->readRawByte();
    return (((__int64) b1 & 0xff)) | (((__int64) b2 & 0xff) << 8) | (((__int64) b3 & 0xff) << 16) |
           (((__int64) b4 & 0xff) << 24) | (((__int64) b5 & 0xff) << 32) | (((__int64) b6 & 0xff) << 40) |
           (((__int64) b7 & 0xff) << 48) | (((__int64) b8 & 0xff) << 56);
}

char CodedInputData::readRawByte() {
    if (m_position == m_size) {
        std::ostringstream oss;
        oss << "reach end, m_position: " << m_position << ", m_size: " << m_size;
        throw out_of_range(oss.str());
    }
    char *bytes = (char *) m_ptr;
    return bytes[m_position++];
}


} // namespace mmkv
