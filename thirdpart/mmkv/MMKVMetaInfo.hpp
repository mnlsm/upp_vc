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

#ifndef MMKV_MMKVMETAINFO_H
#define MMKV_MMKVMETAINFO_H
#ifdef __cplusplus


#include <cstring>

namespace mmkv {

enum MMKVVersion : DWORD {
    MMKVVersionDefault = 0,

    // record full write back count
    MMKVVersionSequence = 1,

    // store random iv for encryption
    MMKVVersionRandomIV = 2,

    // store actual size together with crc checksum, try to reduce file corruption
    MMKVVersionActualSize = 3,
};

struct MMKVMetaInfo {
    DWORD m_crcDigest;
    DWORD m_version ;
    DWORD m_sequence; // full write-back count
    BYTE m_vector[AES_KEY_LEN];
    DWORD m_actualSize;

    MMKVMetaInfo() {
        m_crcDigest = 0;
        m_version = MMKVVersionSequence;
        m_sequence = 0; // full write-back count
        m_actualSize = 0;
        ZeroMemory(m_vector, AES_KEY_LEN);
    }


    // confirmed info: it's been synced to file
    struct ConfirmedMetaInfo{
        ConfirmedMetaInfo() {
            lastActualSize = 0;
            lastCRCDigest = 0;
            ZeroMemory(_reserved, 16 * sizeof(DWORD));
        }
        DWORD lastActualSize;
        DWORD lastCRCDigest;
        DWORD _reserved[16];

    } m_lastConfirmedMetaInfo;

    void write(void *ptr) const {
        MMKV_ASSERT(ptr);
        memcpy(ptr, this, sizeof(MMKVMetaInfo));
    }

    void writeCRCAndActualSizeOnly(void *ptr) const {
        MMKV_ASSERT(ptr);
        MMKVMetaInfo* other = (MMKVMetaInfo *) ptr;
        other->m_crcDigest = m_crcDigest;
        other->m_actualSize = m_actualSize;
    }

    void read(const void *ptr) {
        MMKV_ASSERT(ptr);
        memcpy(this, ptr, sizeof(MMKVMetaInfo));
    }
};

//static_assert(sizeof(MMKVMetaInfo) <= (4 * 1024), "MMKVMetaInfo lager than one pagesize");

} // namespace mmkv

#endif
#endif //MMKV_MMKVMETAINFO_H
