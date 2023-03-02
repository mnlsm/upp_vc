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

#ifndef MMKV_MMBUFFER_H
#define MMKV_MMBUFFER_H
#ifdef __cplusplus

#include "MMKVPredef.h"


#include <cstdlib>
#include <cstddef>

#include <unordered_set>
#include <unordered_map>


namespace mmkv {

typedef bool MMBufferCopyFlag;
extern MMBufferCopyFlag MMBufferCopy;
extern MMBufferCopyFlag MMBufferNoCopy;

#pragma pack(push, 1)

class MMBuffer {
/*
    enum MMBufferType : BYTE {
        MMBufferType_Small,  // store small buffer in stack memory
        MMBufferType_Normal, // store in heap memory
    };
*/
    typedef BYTE MMBufferType;
    static const MMBufferType MMBufferType_Small = 0x00;
    static const MMBufferType MMBufferType_Normal = 0x01;
    MMBufferType type;

    union {
        struct {
            MMBufferCopyFlag isNoCopy;
            size_t size;
            void *ptr;
        };
        struct {
            BYTE paddedSize;
            // make at least 10 bytes to hold all primitive types (negative int32, int64, double etc) on 32 bit device
            // on 64 bit device it's guaranteed larger than 10 bytes
            BYTE paddedBuffer[10];
        };
    };


public:
    explicit MMBuffer(size_t length = 0);
    MMBuffer(void *source, size_t length, MMBufferCopyFlag flag = MMBufferCopy);
    MMBuffer(MMBuffer &other) ;
    MMBuffer& operator=(MMBuffer &other) ;

    ~MMBuffer();

    bool isStoredOnStack() const { return (type == MMBufferType_Small); }

    void *getPtr() const { return isStoredOnStack() ? (void *) paddedBuffer : ptr; }

    size_t length() const { return isStoredOnStack() ? paddedSize : size; }

    // transfer ownership to others
    void detach();



};

#pragma pack(pop)

} // namespace mmkv

#endif
#endif //MMKV_MMBUFFER_H
