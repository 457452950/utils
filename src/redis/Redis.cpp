#include "Redis.h"

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
        LOG(ERROR) << "errstr : " << c->errstr;
        return;
    }

    LOG(INFO) << "Redis connected...";
}

void disconnectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        LOG(ERROR) << "errstr : " << c->errstr;
        return;
    }

    LOG(INFO) << "Redis disconnected...";
}

void pushCallback(redisAsyncContext *c, void *r, void *privdata) 
{
    redisReply* reply = (redisReply*)r;
    if (reply == NULL) 
        return;
    LOG(ERROR) << "reply : " << reply->str << " , cmd : " << (char*)privdata;
}

CRedisClient* CRedisClient::s_Instance = nullptr;
redisContext* CRedisClient::s_pRedisContext = nullptr;
redisAsyncContext* CRedisClient::s_pRedisAsyncContext = nullptr;

CRedisClient* CRedisClient::CreateClient(const char* ip, uint port)
{
    if (s_Instance != nullptr)
    {
        return nullptr;
    }

    s_pRedisAsyncContext = redisAsyncConnect(ip, port);
    redisAsyncSetConnectCallback(s_pRedisAsyncContext, 
                                    connectCallback);
    redisAsyncSetDisconnectCallback(s_pRedisAsyncContext, 
                                    disconnectCallback);

    timeval t{1, 0};      // set 1s
    s_pRedisContext = redisConnectWithTimeout(ip, port, t);

    return s_Instance;
}

CRedisClient* CRedisClient::getInstance()
{
    return s_Instance;
}

CRedisClient::~CRedisClient() 
{
    redisAsyncDisconnect(s_pRedisAsyncContext);
        
    if (s_pRedisContext != nullptr)
    {
        redisFree(s_pRedisContext);
    }
    if (s_pRedisAsyncContext != nullptr)
    {
        redisAsyncFree(s_pRedisAsyncContext);
    }
        
}

int CRedisClient::AsyncCommand(const char* format)
{
    LOG(INFO) << "async cmd : " << format;
    return ::redisAsyncCommand(s_pRedisAsyncContext, 
                                pushCallback,
                                (void*)format,
                                format);
}

void* CRedisClient::Command(const char* format)
{
    LOG(INFO) << " cmd : " << format;
    return ::redisCommand(s_pRedisContext, format);
}

void CRedisClient::Set(const char* key, const char* value)
{
    std::string cmd("GET ");
    cmd = cmd + key + " " + value;
    this->AsyncCommand(cmd.c_str());
}

}


