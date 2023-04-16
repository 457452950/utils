#pragma once
#ifndef UTILS_REDIS_CLIENT_INTERFACE_H
#define UTILS_REDIS_CLIENT_INTERFACE_H

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
using Field = std::string;
using Value = std::string;
using ValueList = std::vector<std::string>;

class IRedisClient
{
public:

// Async
    // String

    // List

    // Set

    // 

// Sync
    // String
    virtual void Set(const char* key, const char* value, int time_out_s = -1) = 0;
    virtual void Get(const std::string& key, std::string& value) = 0;

    // Hash
    virtual bool HSetNX(const Key& key, const Field& field, const Value& value) = 0;
    virtual bool HSetNX(const Key& key, int field, const Value& value) = 0;

    virtual void HDEL(const Key& key, int field) = 0;

    virtual void HGetAll(const Key& key, std::vector<std::tuple<Value, Value>>& values) = 0;

    // List

    // Set
    virtual void SAdd(const Key& key, const Value& value) = 0;
    virtual bool SAdd(const Key& key, int32_t value) = 0;
    virtual bool SIsMember(const Key& key, const Value& value) = 0;

    // 


    // other
    virtual void Del(const Key& key) = 0;
    virtual void INCR(const Key& key) = 0;
    virtual void DECR(const Key& key) = 0;
    virtual void INCRBY(const Key& key, int32_t value) = 0;
    virtual void DECRBY(const Key& key, int32_t value) = 0;

    virtual ~IRedisClient() = default;
private:
};

IRedisClient* CreateRedisClient(const char* ip, uint port);
IRedisClient* GetRedisClient();


}

#endif //UTILS_REDIS_CLIENT_INTERFACE_H
