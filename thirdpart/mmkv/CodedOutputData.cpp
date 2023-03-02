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

#include "CodedOutputData.h"
#include "PBUtility.h"
#include <cstring>
#include <stdexcept>
#include <sstream>

using namespace std;

namespace mmkv {

CodedOutputData::CodedOutputData(void *ptr, size_t len) : m_ptr((BYTE *) ptr), m_size(len), m_position(0) {
    MMKV_ASSERT(m_ptr);
}

BYTE *CodedOutputData::curWritePointer() {
    return m_ptr + m_position;
}

void CodedOutputData::writeDouble(double value) {
    this->writeRawLittleEndian64(Float64ToInt64(value));
}

void CodedOutputData::writeFloat(float value) {
    this->writeRawLittleEndian32(Float32ToInt32(value));
}

void CodedOutputData::writeInt64(__int64 value) {
    this->writeRawVarint64(value);
}

void CodedOutputData::writeUInt64(unsigned __int64 value) {
    writeRawVarint64(static_cast<__int64>(value));
}

void CodedOutputData::writeInt32(INT value) {
    if (value >= 0) {
        this->writeRawVarint32(value);
    } else {
        this->writeRawVarint64(value);
    }
}

void CodedOutputData::writeUInt32(DWORD value) {
    writeRawVarint32(static_cast<INT>(value));
}

void CodedOutputData::writeBool(bool value) {
    this->writeRawByte(static_cast<BYTE>(value ? 1 : 0));
}

void CodedOutputData::writeData(const MMBuffer &value) {
    this->writeRawVarint32((INT) value.length());
    this->writeRawData(value);
}


void CodedOutputData::writeString(const string &value) {
    size_t numberOfBytes = value.size();
    this->writeRawVarint32((INT) numberOfBytes);
    if (m_position + numberOfBytes > m_size) {
        std::ostringstream oss;
        oss << "m_position: " << m_position << ", numberOfBytes: " << numberOfBytes
            << ", m_size: " << m_size;
        throw out_of_range(oss.str());
    }
    memcpy(m_ptr + m_position, ((BYTE *) value.data()), numberOfBytes);
    m_position += numberOfBytes;
}


size_t CodedOutputData::spaceLeft() {
    if (m_size <= m_position) {
        return 0;
    }
    return m_size - m_position;
}

void CodedOutputData::seek(size_t addedSize) {
    m_position += addedSize;

    if (m_position > m_size) {
        throw out_of_range("OutOfSpace");
    }
}

void CodedOutputData::writeRawByte(BYTE value) {
    if (m_position == m_size) {
        std::ostringstream oss;
        oss << "m_position: " << m_position << " m_size: " << m_size;
        throw out_of_range(oss.str());
        return;
    }

    m_ptr[m_position++] = value;
}

void CodedOutputData::writeRawData(const MMBuffer &data) {
    size_t numberOfBytes = data.length();
    if (m_position + numberOfBytes > m_size) {
        std::ostringstream oss;
        throw out_of_range(oss.str());
    }
    memcpy(m_ptr + m_position, data.getPtr(), numberOfBytes);
    m_position += numberOfBytes;
}

void CodedOutputData::writeRawVarint32(INT value) {
    while (true) {
        if ((value & ~0x7f) == 0) {
            this->writeRawByte(static_cast<BYTE>(value));
            return;
        } else {
            this->writeRawByte(static_cast<BYTE>((value & 0x7F) | 0x80));
            value = logicalRightShift32(value, 7);
        }
    }
}

void CodedOutputData::writeRawVarint64(__int64 value) {
    while (true) {
        if ((value & ~0x7f) == 0) {
            this->writeRawByte(static_cast<BYTE>(value));
            return;
        } else {
            this->writeRawByte(static_cast<BYTE>((value & 0x7f) | 0x80));
            value = logicalRightShift64(value, 7);
        }
    }
}

void CodedOutputData::writeRawLittleEndian32(INT value) {
    this->writeRawByte(static_cast<BYTE>((value) &0xff));
    this->writeRawByte(static_cast<BYTE>((value >> 8) & 0xff));
    this->writeRawByte(static_cast<BYTE>((value >> 16) & 0xff));
    this->writeRawByte(static_cast<BYTE>((value >> 24) & 0xff));
}

void CodedOutputData::writeRawLittleEndian64(__int64 value) {
    this->writeRawByte(static_cast<BYTE>((value) &0xff));
    this->writeRawByte(static_cast<BYTE>((value >> 8) & 0xff));
    this->writeRawByte(static_cast<BYTE>((value >> 16) & 0xff));
    this->writeRawByte(static_cast<BYTE>((value >> 24) & 0xff));
    this->writeRawByte(static_cast<BYTE>((value >> 32) & 0xff));
    this->writeRawByte(static_cast<BYTE>((value >> 40) & 0xff));
    this->writeRawByte(static_cast<BYTE>((value >> 48) & 0xff));
    this->writeRawByte(static_cast<BYTE>((value >> 56) & 0xff));
}

} // namespace mmkv
