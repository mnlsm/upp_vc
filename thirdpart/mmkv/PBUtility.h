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

#ifndef MMKV_PBUTILITY_H
#define MMKV_PBUTILITY_H
#ifdef  __cplusplus

#include "MMKVPredef.h"


#    ifndef likely
#        define unlikely(x) (x)
#        define likely(x) (x)
#    endif

namespace mmkv {

template <typename T, typename P>
union Converter {
    //static_assert(sizeof(T) == sizeof(P), "size not match");
    T first;
    P second;
};

static inline __int64 Float64ToInt64(double v) {
    Converter<double, __int64> converter;
    converter.first = v;
    return converter.second;
}

static inline INT Float32ToInt32(float v) {
    Converter<float, INT> converter;
    converter.first = v;
    return converter.second;
}

static inline double Int64ToFloat64(__int64 v) {
    Converter<double, __int64> converter;
    converter.second = v;
    return converter.first;
}

static inline float Int32ToFloat32(INT v) {
    Converter<float, INT> converter;
    converter.second = v;
    return converter.first;
}

static inline unsigned __int64 Int64ToUInt64(__int64 v) {
    Converter<__int64, unsigned __int64> converter;
    converter.first = v;
    return converter.second;
}

static inline __int64 UInt64ToInt64(unsigned __int64 v) {
    Converter<__int64, unsigned __int64> converter;
    converter.second = v;
    return converter.first;
}

static inline DWORD Int32ToUInt32(INT v) {
    Converter<INT, DWORD> converter;
    converter.first = v;
    return converter.second;
}

static inline INT UInt32ToInt32(DWORD v) {
    Converter<INT, DWORD> converter;
    converter.second = v;
    return converter.first;
}

static inline INT logicalRightShift32(INT value, DWORD spaces) {
    return UInt32ToInt32((Int32ToUInt32(value) >> spaces));
}

static inline __int64 logicalRightShift64(__int64 value, DWORD spaces) {
    return UInt64ToInt64((Int64ToUInt64(value) >> spaces));
}

extern const DWORD LittleEdian32Size;

inline const DWORD pbFloatSize() {
    return LittleEdian32Size;
}

inline const DWORD pbFixed32Size() {
    return LittleEdian32Size;
}

extern const DWORD LittleEdian64Size;

inline const DWORD pbDoubleSize() {
    return LittleEdian64Size;
}

inline const DWORD pbBoolSize() {
    return 1;
}

extern DWORD pbRawVarint32Size(DWORD value);

static inline DWORD pbRawVarint32Size(INT value) {
    return pbRawVarint32Size(Int32ToUInt32(value));
}

extern DWORD pbUInt64Size(unsigned __int64 value);

static inline DWORD pbInt64Size(__int64 value) {
    return pbUInt64Size(Int64ToUInt64(value));
}

static inline DWORD pbInt32Size(INT value) {
    if (value >= 0) {
        return pbRawVarint32Size(value);
    } else {
        return 10;
    }
}

static inline DWORD pbUInt32Size(DWORD value) {
    return pbRawVarint32Size(value);
}

} // namespace mmkv

#endif
#endif //MMKV_PBUTILITY_H
