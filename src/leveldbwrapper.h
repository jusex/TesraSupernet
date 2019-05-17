



#ifndef BITCOIN_LEVELDBWRAPPER_H
#define BITCOIN_LEVELDBWRAPPER_H

#include "clientversion.h"
#include "serialize.h"
#include "streams.h"
#include "util.h"
#include "version.h"

#include <boost/filesystem/path.hpp>

#include <leveldb/db.h>
#include <leveldb/write_batch.h>



class leveldb_error : public std::runtime_error
{
public:
    leveldb_error(const std::string& msg) : std::runtime_error(msg) {}
};

void HandleError(const leveldb::Status& status) throw(leveldb_error);


class CLevelDBBatch
{
    friend class CLevelDBWrapper;

private:
    leveldb::WriteBatch batch;

public:
    template <typename K, typename V>
    void Write(const K& key, const V& value)
    {
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(ssKey.GetSerializeSize(key));
        ssKey << key;
        leveldb::Slice slKey(&ssKey[0], ssKey.size());

        CDataStream ssValue(SER_DISK, CLIENT_VERSION);
        ssValue.reserve(ssValue.GetSerializeSize(value));
        ssValue << value;
        leveldb::Slice slValue(&ssValue[0], ssValue.size());

        batch.Put(slKey, slValue);
    }

    template <typename K>
    void Erase(const K& key)
    {
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(ssKey.GetSerializeSize(key));
        ssKey << key;
        leveldb::Slice slKey(&ssKey[0], ssKey.size());

        batch.Delete(slKey);
    }
};

class CLevelDBWrapper
{
private:
    
    leveldb::Env* penv;

    
    leveldb::Options options;

    
    leveldb::ReadOptions readoptions;

    
    leveldb::ReadOptions iteroptions;

    
    leveldb::WriteOptions writeoptions;

    
    leveldb::WriteOptions syncoptions;

    
    leveldb::DB* pdb;

public:
    CLevelDBWrapper(const boost::filesystem::path& path, size_t nCacheSize, bool fMemory = false, bool fWipe = false);
    ~CLevelDBWrapper();

    template <typename K, typename V>
    bool Read(const K& key, V& value) const throw(leveldb_error)
    {
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(ssKey.GetSerializeSize(key));
        ssKey << key;
        leveldb::Slice slKey(&ssKey[0], ssKey.size());

        std::string strValue;
        leveldb::Status status = pdb->Get(readoptions, slKey, &strValue);
        if (!status.ok()) {
            if (status.IsNotFound())
                return false;
            LogPrintf("LevelDB read failure: %s\n", status.ToString());
            HandleError(status);
        }
        try {
            CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_DISK, CLIENT_VERSION);
            ssValue >> value;
        } catch (const std::exception&) {
            return false;
        }
        return true;
    }

    template <typename K, typename V>
    bool Write(const K& key, const V& value, bool fSync = false) throw(leveldb_error)
    {
        CLevelDBBatch batch;
        batch.Write(key, value);
        return WriteBatch(batch, fSync);
    }

    template <typename K>
    bool Exists(const K& key) const throw(leveldb_error)
    {
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(ssKey.GetSerializeSize(key));
        ssKey << key;
        leveldb::Slice slKey(&ssKey[0], ssKey.size());

        std::string strValue;
        leveldb::Status status = pdb->Get(readoptions, slKey, &strValue);
        if (!status.ok()) {
            if (status.IsNotFound())
                return false;
            LogPrintf("LevelDB read failure: %s\n", status.ToString());
            HandleError(status);
        }
        return true;
    }

    template <typename K>
    bool Erase(const K& key, bool fSync = false) throw(leveldb_error)
    {
        CLevelDBBatch batch;
        batch.Erase(key);
        return WriteBatch(batch, fSync);
    }

    bool WriteBatch(CLevelDBBatch& batch, bool fSync = false) throw(leveldb_error);

    
    bool Flush()
    {
        return true;
    }

    bool Sync() throw(leveldb_error)
    {
        CLevelDBBatch batch;
        return WriteBatch(batch, true);
    }

    
    leveldb::Iterator* NewIterator()
    {
        return pdb->NewIterator(iteroptions);
    }
};

#endif 
