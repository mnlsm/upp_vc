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

#include "KeyValueHolder.h"
#include "PBUtility.h"
#include <cerrno>
#include <cstring>
#include <stdexcept>

namespace mmkv {

KeyValueHolder::KeyValueHolder(DWORD keyLength, DWORD valueLength, DWORD off)
    : keySize(static_cast<unsigned short>(keyLength)), valueSize(valueLength), offset(off) {
    computedKVSize = keySize + static_cast<unsigned short>(pbRawVarint32Size(keySize));
    computedKVSize += static_cast<unsigned short>(pbRawVarint32Size(valueSize));
}

MMBuffer KeyValueHolder::toMMBuffer(const void *basePtr) const {
    BYTE* realPtr = (BYTE *) basePtr + offset;
    realPtr += computedKVSize;
    return MMBuffer(realPtr, valueSize, MMBufferNoCopy);
}

} // namespace mmkv

