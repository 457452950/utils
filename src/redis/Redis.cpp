#include "Redis.h"
#include "stdio.h"
#include <stdlib.h>

namespace wlb
{
    using namespace Log;

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
redisAsyncContext* CRedisClient::s_pRedisAsyncContext = nullptr;
event_base* CRedisClient::s_eventBase = nullptr;
std::thread* CRedisClient::s_pThread = nullptr;

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

            s_eventBase = event_base_new();

            redisOptions options = {0};
            REDIS_OPTIONS_SET_TCP(&options, ip, port);
            struct timeval tv = {0};
            tv.tv_sec = 1;
            options.connect_timeout = &tv;

            s_pRedisAsyncContext = redisAsyncConnectWithOptions(&options);
            
            redisLibeventAttach(s_pRedisAsyncContext,
                                s_eventBase);
            redisAsyncSetConnectCallback(s_pRedisAsyncContext, 
                                            connectCallback);
            redisAsyncSetDisconnectCallback(s_pRedisAsyncContext, 
                                            disconnectCallback);
            
            if (s_pRedisAsyncContext == nullptr){
            }
            if (s_pRedisAsyncContext->err){
                std::cout << "s_pRedisAsyncContext error " << s_pRedisAsyncContext->err 
                            << " str : " << s_pRedisAsyncContext->errstr << std::endl;
            } 

            s_pThread = new std::thread(
                        event_base_dispatch, 
                        CRedisClient::s_eventBase);
            // event_base_dispatch(s_eventBase);

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
    redisAsyncDisconnect(s_pRedisAsyncContext);

    if (s_pThread != nullptr)
    {
        if (s_pThread->joinable())
        {
            s_pThread->join();
        }
        delete s_pThread;
        s_pThread = nullptr;
    }
    
        
    if (s_pRedisContext != nullptr)
    {
        redisFree(s_pRedisContext);
        s_pRedisContext = nullptr;
    }
    if (s_pRedisAsyncContext != nullptr)
    {
        redisAsyncFree(s_pRedisAsyncContext);
        s_pRedisAsyncContext = nullptr;
    }
        
}

int CRedisClient::AsyncCommand(const char* format)
{
    return ::redisAsyncCommand(s_pRedisAsyncContext, 
                                pushCallback,
                                nullptr,
                                format);
}

void* CRedisClient::Command(const char* format)
{
    std::cout << " cmd : " << format << std::endl;
    return ::redisCommand(s_pRedisContext, format);
}

void CRedisClient::Set(const char* key, const char* value, int time_out_s)
{
    if (time_out_s <= 0)
    {
        std::string cmd("SET ");
        cmd = cmd + key + " " + value;
        int res = this->AsyncCommand(cmd.c_str());
        return;
    }
    else
    {
        std::string _time = std::to_string(time_out_s);
        if (_time.empty())
            return;

        std::string cmd("SETEX ");
        cmd = cmd + key + " " + _time + " " + value;
        std::cout << cmd << std::endl;
        int res = this->AsyncCommand(cmd.c_str());
        std::cout << "res" << res << std::endl;
        return;
    }
}
void CRedisClient::SAdd(const Key& key, const Value& value)
{
    std::string cmd("SADD ");
    cmd = cmd + key + " " + value;
    std::cout << cmd << std::endl;
    int res = this->AsyncCommand(cmd.c_str());
    std::cout << "res" << res << std::endl;
    return;
}
void CRedisClient::SAdd(const Key& key, const ValueList& list) 
{
    std::string cmd("SADD ");
    cmd = cmd + key;

    for (auto& value : list)
    {
        cmd += ( " " + value);
    }
    
    std::cout << cmd << std::endl;
    int res = this->AsyncCommand(cmd.c_str());
    std::cout << "res" << res << std::endl;
    return;
}

void CRedisClient::Get(const std::string& key, std::string& value)
{
    value.clear();
    
    std::string cmd("GET ");
    cmd = cmd + key;
    redisReply* reply = (redisReply*)this->Command(cmd.c_str());
    if (reply == nullptr || reply->len == 0) {
        return; 
    }
    value = std::string(reply->str);
    freeReplyObject(reply);
}

bool CRedisClient::SIsMember(const Key& key, const Value& value)
{
    std::string cmd("SISMEMBER ");
    cmd = cmd + key + " " + value;
    redisReply* reply = (redisReply*)this->Command(cmd.c_str());
    if (reply == nullptr) {
        std::cout << "CRedisClient::SIsMember error" << std::endl;
        return false;
    }
    std::cout << "CRedisClient::SIsMember " << reply->type << " " << reply->integer << std::endl;
    bool ok = reply->integer;
    freeReplyObject(reply);
    return ok;
}

bool CRedisClient::SyncSAdd(const Key& key, int32_t value)
{
    char cmd[150];
    sprintf(cmd, "SADD %s %d", key.c_str(), value);

    redisReply* reply = (redisReply*)this->Command(cmd);
    if (reply == nullptr) {
        std::cout << "CRedisClient::SyncSAdd error" << std::endl;
        return false;
    }
    std::cout << "CRedisClient::SyncSAdd " << reply->type << " " << reply->integer << std::endl;
    bool ok = reply->integer;
    freeReplyObject(reply);
    return ok;
}

void CRedisClient::Del(const Key& key)
{
    std::string cmd("DEL ");
    cmd = cmd + key;

    std::cout << cmd << std::endl;
    int res = this->AsyncCommand(cmd.c_str());
    std::cout << "res" << res << std::endl;
}

}


