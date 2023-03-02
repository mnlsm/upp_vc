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

#ifndef MMKV_CODEDINPUTDATA_H
#define MMKV_CODEDINPUTDATA_H
#ifdef __cplusplus

#include "MMKVPredef.h"

#include "KeyValueHolder.h"
#include "MMBuffer.h"


namespace mmkv {

class CodedInputData {
    BYTE *const m_ptr;
    size_t m_size;
    size_t m_position;

    char readRawByte();

    INT readRawVarint32();

    INT readRawLittleEndian32();

    __int64 readRawLittleEndian64();

public:
    CodedInputData(const void *oData, size_t length);

    bool isAtEnd() const { return m_position == m_size; };

    void seek(size_t addedSize);

    bool readBool();

    double readDouble();

    float readFloat();

    __int64 readInt64();

    unsigned __int64 readUInt64();

    INT readInt32();

    DWORD readUInt32();

    MMBuffer readData();
    void readData(KeyValueHolder &kvHolder);

    std::string readString();
    std::string readString(KeyValueHolder &kvHolder);
};

} // namespace mmkv

#endif
#endif //MMKV_CODEDINPUTDATA_H
