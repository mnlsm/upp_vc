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

#ifndef MMKV_MMKV_H
#define MMKV_MMKV_H
#ifdef  __cplusplus

#include "MMBuffer.h"


namespace mmkv {
class CodedOutputData;
class MemoryFile;
class AESCrypt;
struct MMKVMetaInfo;
class FileLock;
class InterProcessLock;
class ThreadLock;
} // namespace mmkv

MMKV_NAMESPACE_BEGIN
typedef DWORD MMKVMode;
extern const MMKVMode MMKV_SINGLE_PROCESS ;
extern const MMKVMode MMKV_MULTI_PROCESS ;



class MMKV {
    std::string m_mmapKey;
    MMKV(const std::string &mmapID, MMKVMode mode, std::string *cryptKey, MMKVPath_t *rootPath);
    ~MMKV();

    std::string m_mmapID;
    MMKVPath_t m_path;
    MMKVPath_t m_crcPath;
    mmkv::MMKVMap *m_dic;
    mmkv::MMKVMapCrypt *m_dicCrypt;

    mmkv::MemoryFile *m_file;
    size_t m_actualSize;
    mmkv::CodedOutputData *m_output;

    bool m_needLoadFromFile;
    bool m_hasFullWriteback;

    DWORD m_crcDigest;
    mmkv::MemoryFile *m_metaFile;
    mmkv::MMKVMetaInfo *m_metaInfo;

    mmkv::AESCrypt *m_crypter;

    mmkv::ThreadLock *m_lock;
    mmkv::FileLock *m_fileLock;
    mmkv::InterProcessLock *m_sharedProcessLock;
    mmkv::InterProcessLock *m_exclusiveProcessLock;


    typedef const std::string& MMKVKey_t ;
    static bool isKeyEmpty(MMKVKey_t key) { return key.empty(); }

    void loadFromFile();

    void partialLoadFromFile();

    void checkDataValid(bool &loadFromFile, bool &needFullWriteback);

    void checkLoadData();

    bool isFileValid();

    bool checkFileCRCValid(size_t actualSize, DWORD crcDigest);

    void recaculateCRCDigestWithIV(const void *iv);

    void updateCRCDigest(const BYTE *ptr, size_t length);

    size_t readActualSize();

    void oldStyleWriteActualSize(size_t actualSize);

    bool writeActualSize(size_t size, DWORD crcDigest, const void *iv, bool increaseSequence);

    bool ensureMemorySize(size_t newSize);

    bool fullWriteback(mmkv::AESCrypt *newCrypter = NULL);

    bool doFullWriteBack(std::pair<mmkv::MMBuffer, size_t>& preparedData, mmkv::AESCrypt *newCrypter);

    mmkv::MMBuffer getDataForKey(MMKVKey_t key);

    // isDataHolder: avoid memory copying
    bool setDataForKey(mmkv::MMBuffer &data, MMKVKey_t key, bool isDataHolder = false);

    bool removeDataForKey(MMKVKey_t key);

    typedef std::pair<bool, mmkv::KeyValueHolder> KVHolderRet_t;
    // isDataHolder: avoid memory copying
    KVHolderRet_t doAppendDataWithKey(const mmkv::MMBuffer &data, const mmkv::MMBuffer &key, bool isDataHolder, DWORD keyLength);
    KVHolderRet_t appendDataWithKey(const mmkv::MMBuffer &data, MMKVKey_t key, bool isDataHolder = false);
    KVHolderRet_t appendDataWithKey(const mmkv::MMBuffer &data, const mmkv::KeyValueHolder &kvHolder, bool isDataHolder = false);


    void notifyContentChanged();


    static bool backupOneToDirectory(const std::string &mmapKey, const MMKVPath_t &dstPath, const MMKVPath_t &srcPath, bool compareFullPath);
    static size_t backupAllToDirectory(const MMKVPath_t &dstDir, const MMKVPath_t &srcDir, bool isInSpecialDir);
    static bool restoreOneFromDirectory(const std::string &mmapKey, const MMKVPath_t &srcPath, const MMKVPath_t &dstPath, bool compareFullPath);
    static size_t restoreAllFromDirectory(const MMKVPath_t &srcDir, const MMKVPath_t &dstDir, bool isInSpecialDir);

public:
    // call this before getting any MMKV instance
    static void initializeMMKV(const MMKVPath_t &rootDir, MMKVLogLevel logLevel = MMKVLogInfo, mmkv::LogHandler handler = NULL);



    // a generic purpose instance
    static MMKV *defaultMMKV(MMKVMode mode = MMKV_SINGLE_PROCESS, std::string *cryptKey = NULL);


    // mmapID: any unique ID (com.tencent.xin.pay, etc)
    // if you want a per-user mmkv, you could merge user-id within mmapID
    // cryptKey: 16 bytes at most
    static MMKV *mmkvWithID(const std::string &mmapID,
                            MMKVMode mode = MMKV_SINGLE_PROCESS,
                            std::string *cryptKey = NULL,
                            MMKVPath_t *rootPath = NULL);



    // you can call this on application termination, it's totally fine if you don't call
    static void onExit();

    const std::string &mmapID() const;

    const bool m_isInterProcess;

    bool set(bool value, MMKVKey_t key);

    bool set(INT value, MMKVKey_t key);

    bool set(DWORD value, MMKVKey_t key);

    bool set(__int64 value, MMKVKey_t key);

    bool set(unsigned __int64 value, MMKVKey_t key);

    bool set(float value, MMKVKey_t key);

    bool set(double value, MMKVKey_t key);

    // avoid unexpected type conversion (pointer to bool, etc)
    template <typename T>
    bool set(T value, MMKVKey_t key);


    bool set(const char *value, MMKVKey_t key);

    bool set(const std::string &value, MMKVKey_t key);

    bool set(const mmkv::MMBuffer &value, MMKVKey_t key);

    bool set(const std::vector<std::string> &vector, MMKVKey_t key);

    bool getString(MMKVKey_t key, std::string &result);

    mmkv::MMBuffer getBytes(MMKVKey_t key);

    bool getBytes(MMKVKey_t key, mmkv::MMBuffer &result);

    bool getVector(MMKVKey_t key, std::vector<std::string> &result);

    bool getBool(MMKVKey_t key, bool defaultValue = false, OUT bool *hasValue = NULL);

    INT getInt32(MMKVKey_t key, INT defaultValue = 0, OUT bool *hasValue = NULL);

    DWORD getUInt32(MMKVKey_t key, DWORD defaultValue = 0, OUT bool *hasValue = NULL);

    __int64 getInt64(MMKVKey_t key, __int64 defaultValue = 0, OUT bool *hasValue = NULL);

    unsigned __int64 getUInt64(MMKVKey_t key, unsigned __int64 defaultValue = 0, OUT bool *hasValue = NULL);

    float getFloat(MMKVKey_t key, float defaultValue = 0, OUT bool *hasValue = NULL);

    double getDouble(MMKVKey_t key, double defaultValue = 0, OUT bool *hasValue = NULL);

    // return the actual size consumption of the key's value
    // pass actualSize = true to get value's length
    size_t getValueSize(MMKVKey_t key, bool actualSize);

    // return size written into buffer
    // return -1 on any error
    INT writeValueToBuffer(MMKVKey_t key, void *ptr, INT size);

    bool containsKey(MMKVKey_t key);

    size_t count();

    size_t totalSize();

    size_t actualSize();


    std::vector<std::string> allKeys();

    void removeValuesForKeys(const std::vector<std::string> &arrKeys);

    void removeValueForKey(MMKVKey_t key);

    void clearAll();

    // MMKV's size won't reduce after deleting key-values
    // call this method after lots of deleting if you care about disk usage
    // note that `clearAll` has the similar effect of `trim`
    void trim();

    // call this method if the instance is no longer needed in the near future
    // any subsequent call to the instance is undefined behavior
    void close();

    // call this method if you are facing memory-warning
    // any subsequent call to the instance will load all key-values from file again
    void clearMemoryCache();

    // you don't need to call this, really, I mean it
    // unless you worry about running out of battery
    void sync(SyncFlag flag = MMKV_SYNC);

    // get exclusive access
    void lock();
    void unlock();
    bool try_lock();

    static const MMKVPath_t &getRootDir();

    // backup one MMKV instance from srcDir to dstDir
    // if srcDir is null, then backup from the root dir of MMKV
    static bool backupOneToDirectory(const std::string &mmapID, const MMKVPath_t &dstDir, const MMKVPath_t *srcDir = NULL);

    // restore one MMKV instance from srcDir to dstDir
    // if dstDir is null, then restore to the root dir of MMKV
    static bool restoreOneFromDirectory(const std::string &mmapID, const MMKVPath_t &srcDir, const MMKVPath_t *dstDir = NULL);

    // backup all MMKV instance from srcDir to dstDir
    // if srcDir is null, then backup from the root dir of MMKV
    // return count of MMKV successfully backuped
    static size_t backupAllToDirectory(const MMKVPath_t &dstDir, const MMKVPath_t *srcDir = NULL);

    // restore all MMKV instance from srcDir to dstDir
    // if dstDir is null, then restore to the root dir of MMKV
    // return count of MMKV successfully restored
    static size_t restoreAllFromDirectory(const MMKVPath_t &srcDir, const MMKVPath_t *dstDir = NULL);

    // check if content been changed by other process
    void checkContentChanged();

    // called when content is changed by other process
    // doesn't guarantee real-time notification
    static void registerContentChangeHandler(mmkv::ContentChangeHandler handler);
    static void unRegisterContentChangeHandler();

    // by default MMKV will discard all datas on failure
    // return `OnErrorRecover` to recover any data from file
    static void registerErrorHandler(mmkv::ErrorHandler handler);
    static void unRegisterErrorHandler();

    // MMKVLogInfo by default
    // pass MMKVLogNone to disable all logging
    static void setLogLevel(MMKVLogLevel level);

    // by default MMKV will print log to the console
    // implement this method to redirect MMKV's log
    static void registerLogHandler(mmkv::LogHandler handler);
    static void unRegisterLogHandler();

    // detect if the MMKV file is valid or not
    // Note: Don't use this to check the existence of the instance, the return value is undefined if the file was never created.
    static bool isFileValid(const std::string &mmapID, MMKVPath_t *relatePath = NULL);

    // just forbid it for possibly misuse
    explicit MMKV(const MMKV &other);
    MMKV &operator=(const MMKV &other);

    void checkLastConfirmedInfo(bool &loadFromFile, bool &needFullWriteback, size_t& fileSize);

};

MMKV_NAMESPACE_END

#endif
#endif // MMKV_MMKV_H
