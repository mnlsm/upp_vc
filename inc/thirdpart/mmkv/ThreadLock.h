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

#ifndef MMKV_THREADLOCK_H
#define MMKV_THREADLOCK_H
#ifdef  __cplusplus

#include "MMKVPredef.h"


namespace mmkv {


typedef LONG ThreadOnceTokenEnum;
extern const ThreadOnceTokenEnum ThreadOnceUninitialized;
extern const ThreadOnceTokenEnum ThreadOnceInitializing;
extern const ThreadOnceTokenEnum ThreadOnceInitialized;


typedef volatile LONG ThreadOnceToken_t;


class ThreadLock {
private:
    CRITICAL_SECTION m_lock;

public:
    ThreadLock();
    ~ThreadLock();

    void initialize();

    void lock();
    void unlock();

    static void ThreadOnce(ThreadOnceToken_t *onceToken, void (*callback)(void));

    static void Sleep(int ms);

    // just forbid it for possibly misuse
    explicit ThreadLock(const ThreadLock &other);
    ThreadLock &operator=(const ThreadLock &other);
};

} // namespace mmkv

#endif
#endif //MMKV_THREADLOCK_H
