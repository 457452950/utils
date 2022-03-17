#include "Redis.h"
#include "stdio.h"
#include <stdlib.h>

namespace wlb
{

IRedisClient* CreateRedisClient(const char* ip, uint port)
{
    return CRedisClient::CreateClient(ip, port);
}
IRedisClient* GetRedisClient()
{
    return CRedisClient::getInstance();
}

void connectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        return;
    }
    CRedisClient::getInstance()->SetActive(true);

}

void disconnectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        std::cout << "errstr : " << c->errstr << std::endl;
        return;
    }
    CRedisClient::getInstance()->SetActive(false);
    // ::exit(-1);
    std::cout << "Redis disconnected..." << std::endl;
}

void pushCallback(redisAsyncContext *c, void *r, void *privdata) 
{
    std::cout << "pushCallback " << std::endl;
    redisReply* reply = (redisReply*)r;
    if (reply == NULL) 
        return;
    if (reply->type == 3)
    {
        std::cout << "reply integer " << reply->integer << std::endl;
    }
    else if (reply->type == 4)
    {
        std::cout << "reply nil " << reply->integer << std::endl;
    }
    else
        std::cout << "reply str" << reply->str << std::endl;
}

std::mutex    CRedisClient::_mutex;
CRedisClient* CRedisClient::s_Instance = nullptr;
redisContext* CRedisClient::s_pRedisContext = nullptr;

CRedisClient* CRedisClient::CreateClient(const char* ip, uint port)
{
    if (s_Instance != nullptr)
    {
        return nullptr;
    }

    if (s_Instance == nullptr)
    {
        _mutex.lock();

        if (s_Instance == nullptr)
        {
            s_Instance = new CRedisClient();

            timeval t{1, 0};      // set 1s
            s_pRedisContext = redisConnectWithTimeout(ip, port, t);
        }

        _mutex.unlock();   
    }

    return s_Instance;
}

CRedisClient* CRedisClient::getInstance()
{
    return s_Instance;
}

CRedisClient::~CRedisClient() 
{
        
    if (s_pRedisContext != nullptr)
    {
        redisFree(s_pRedisContext);
        s_pRedisContext = nullptr;
    }
}

redisReply* CRedisClient::Command(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    auto* reply = static_cast<redisReply*>(::redisvCommand(s_pRedisContext, format, ap));
    va_end(ap);
    return reply;
}

void CRedisClient::Set(const char* key, const char* value, int time_out_s)
{
    if (time_out_s <= 0)
    {
        redisReply* reply = static_cast<redisReply*>
                        (::redisCommand(s_pRedisContext, "SET %s %s ", key, value));
        if (reply == nullptr)
        {
            std::cout << "reply nullptr" << std::endl;
            return;
        } 
        else if (reply->type == REDIS_REPLY_ERROR || reply->type == REDIS_REPLY_STRING)
        {
            std::cout << "reply : " << reply->str << std::endl;
        }
        else if (reply->type == REDIS_REPLY_INTEGER)
        {
            std::cout << "reply : " << reply->integer << std::endl;
        }
        freeReplyObject(reply);
        return;
    }
    else
    {
        redisReply* reply = static_cast<redisReply*>
                        (::redisCommand(s_pRedisContext, "SETEX %s %d %s ", key, time_out_s, value));
        if (reply == nullptr)
        {
            std::cout << "reply nullptr" << std::endl;
            return;
        } 
        else if (reply->type == REDIS_REPLY_NIL)
        {
            std::cout << "reply nil" << std::endl;
        }
        else if (reply->type == REDIS_REPLY_INTEGER)
        {
            std::cout << "reply : " << reply->integer << std::endl;
        }
        else 
        {
            std::cout << "reply : " << reply->str << std::endl;
        }
        freeReplyObject(reply);
        return;
    }
}
void CRedisClient::SAdd(const Key& key, const Value& value)
{
    std::cout << "key : " << key << " value : " << value << std::endl;
    redisReply *reply = static_cast<redisReply *>
            (::redisCommand(s_pRedisContext, "SADD %s %s ", key.c_str(), value.c_str()));
    if (reply == nullptr)
    {
        std::cout << "reply nullptr" << std::endl;
        return;
    }
    else if (reply->type == REDIS_REPLY_NIL)
    {
        std::cout << "reply nil" << std::endl;
    }
    else if (reply->type == REDIS_REPLY_INTEGER)
    {
        std::cout << "reply : " << reply->integer << std::endl;
    }
    else
    {
        std::cout << "reply : " << reply->str << std::endl;
    }
    freeReplyObject(reply);
    return;
}

void CRedisClient::Get(const std::string& key, std::string& value)
{
    value.clear();
    
    std::cout << "key : " << key << std::endl;
    redisReply *reply = static_cast<redisReply *>
            (::redisCommand(s_pRedisContext, "GET %s ", key.c_str()));
    if (reply == nullptr)
    {
        std::cout << "reply nullptr" << std::endl;
        return;
    }
    else if (reply->type == REDIS_REPLY_NIL)
    {
        std::cout << "reply nil" << std::endl;
    }
    else if (reply->type == REDIS_REPLY_INTEGER)
    {
        std::cout << "reply : " << reply->integer << std::endl;
    }
    else
    {
        std::cout << "reply : " << reply->str << std::endl;
    }

    if (reply->type == REDIS_REPLY_STRING)
        value = std::string(reply->str);
    freeReplyObject(reply);
}


// Hash 
bool CRedisClient::HSetNX(const Key& key, const Field& field, const Value& value)
{
    redisReply *reply = static_cast<redisReply *>(::redisCommand(s_pRedisContext, 
                "HSETNX %s %s %s ", key.c_str(), field.c_str(), value.c_str()));
    if (reply == nullptr)
    {
        std::cout << "reply nullptr" << std::endl;
        return false;
    }
    else if (reply->type == REDIS_REPLY_NIL)
    {
        std::cout << "reply nil" << std::endl;
    }
    else if (reply->type == REDIS_REPLY_INTEGER)
    {
        std::cout << "reply : " << reply->integer << std::endl;
    }
    else
    {
        std::cout << "reply : " << reply->str << std::endl;
    }

    bool ok = false;
    if (reply->type == REDIS_REPLY_INTEGER)
        ok = reply->integer;
    freeReplyObject(reply);
    return ok;
}
bool CRedisClient::HSetNX(const Key& key, int field, const Value& value)
{
    redisReply *reply = static_cast<redisReply *>(::redisCommand(s_pRedisContext, 
                "HSETNX %s %d %s ", key.c_str(), field, value.c_str()));
    if (reply == nullptr)
    {
        std::cout << "reply nullptr" << std::endl;
        return false;
    }
    else if (reply->type == REDIS_REPLY_NIL)
    {
        std::cout << "reply nil" << std::endl;
    }
    else if (reply->type == REDIS_REPLY_INTEGER)
    {
        std::cout << "reply : " << reply->integer << std::endl;
    }
    else
    {
        std::cout << "reply : " << reply->str << std::endl;
    }

    bool ok = false;
    if (reply->type == REDIS_REPLY_INTEGER)
        ok = reply->integer;
    freeReplyObject(reply);
    return ok;
}

void CRedisClient::HGetAll(const Key& key, std::vector<std::tuple<Value, Value>>& values)
{
    std::string cmd = "HGETALL " + key;

    redisReply* reply = (redisReply*)this->Command(cmd.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_ARRAY) {
        return; 
    }
    



    freeReplyObject(reply);
}



// Set
bool CRedisClient::SIsMember(const Key& key, const Value& value)
{
    redisReply *reply = static_cast<redisReply *>(::redisCommand(s_pRedisContext, 
                "SISMEMBER %s %s ", key.c_str(), value.c_str()));
    if (reply == nullptr)
    {
        std::cout << "reply nullptr" << std::endl;
        return false;
    }
    else if (reply->type == REDIS_REPLY_NIL)
    {
        std::cout << "reply nil" << std::endl;
    }
    else if (reply->type == REDIS_REPLY_INTEGER)
    {
        std::cout << "reply : " << reply->integer << std::endl;
    }
    else
    {
        std::cout << "reply : " << reply->str << std::endl;
    }
    bool ok = false;
    if (reply->type == REDIS_REPLY_INTEGER)
        ok = reply->integer;
    freeReplyObject(reply);
    return ok;
}

bool CRedisClient::SAdd(const Key& key, int32_t value)
{
    std::cout << "key : " << key << " value : " << value << std::endl;
    redisReply *reply = static_cast<redisReply *>
            (::redisCommand(s_pRedisContext, "SADD %s %s ", key.c_str(), value));
    if (reply == nullptr)
    {
        std::cout << "reply nullptr" << std::endl;
        return false;
    }
    else if (reply->type == REDIS_REPLY_NIL)
    {
        std::cout << "reply nil" << std::endl;
    }
    else if (reply->type == REDIS_REPLY_INTEGER)
    {
        std::cout << "reply : " << reply->integer << std::endl;
    }
    else
    {
        std::cout << "reply : " << reply->str << std::endl;
    }
    bool ok = false;
    if (reply->type == REDIS_REPLY_INTEGER)
        ok = reply->integer;
    freeReplyObject(reply);
    return ok;
}

void CRedisClient::Del(const Key& key)
{
    std::cout << "key : " << key << std::endl;
    redisReply *reply = static_cast<redisReply *>
            (::redisCommand(s_pRedisContext, "DEL %s ", key.c_str()));
    if (reply == nullptr)
    {
        std::cout << "reply nullptr" << std::endl;
        return;
    }
    else if (reply->type == REDIS_REPLY_NIL)
    {
        std::cout << "reply nil" << std::endl;
    }
    else if (reply->type == REDIS_REPLY_INTEGER)
    {
        std::cout << "reply : " << reply->integer << std::endl;
    }
    else
    {
        std::cout << "reply : " << reply->str << std::endl;
    }
    freeReplyObject(reply);
    return;
}

}


