#pragma once
#include <cstring>
#include <string>
#include <mutex>

#include <vector>
#include <set>

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>

namespace wlb
{


class CRedisClient;

using Key = std::string;
using Value = std::string;
using ValueList = std::vector<std::string>;

class IRedisClient
{
public:

// Async
    // String
    virtual void Set(const char* key, const char* value, int time_out_s = -1) = 0;

    // List

    // Set
    virtual void SAdd(const Key& key, const Value& value) = 0;
    virtual void SAdd(const Key& key, const ValueList& list) = 0;

    // 

    // other
    virtual void Del(const Key& key) = 0;

// Sync
    // String
    virtual void Get(const std::string& key, std::string& value) = 0;

    // List

    // Set
    virtual bool SyncSAdd(const Key& key, int32_t value) = 0;
    virtual bool SIsMember(const Key& key, const Value& value) = 0;

    // 



    virtual ~IRedisClient() = default;
private:
};

IRedisClient* CreateRedisClient(const char* ip, uint port);
IRedisClient* GetRedisClient();


}


