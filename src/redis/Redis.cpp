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
        LOG(L_ERROR) << "errstr : " << c->errstr;
        return;
    }

    LOG(L_INFO) << "Redis connected...";
}

void disconnectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        LOG(L_ERROR) << "errstr : " << c->errstr;
        return;
    }

    LOG(L_INFO) << "Redis disconnected...";
}

void pushCallback(redisAsyncContext *c, void *r, void *privdata) 
{
    redisReply* reply = (redisReply*)r;
    if (reply == NULL) 
        return;
    LOG(L_ERROR) << "reply : " << reply->str << " , cmd : " << (char*)privdata;
    std::cout << "reply : " << reply->str << " , cmd : " << (char*)privdata << std::endl;
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
        LOG(L_ERROR) << "you had created";
        return nullptr;
    }

    if (s_Instance == nullptr)
    {
        _mutex.lock();

        if (s_Instance == nullptr)
        {
            LOG(L_INFO) << "Create asynccontext and cpntext";

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
                LOG(L_ERROR) << "redisAsyncConnect error : nullptr";
            }
            if (s_pRedisAsyncContext->err){
                LOG(L_ERROR) << "s_pRedisAsyncContext error " << s_pRedisAsyncContext->err 
                            << " str : " << s_pRedisAsyncContext->errstr;
            } 

            s_pThread = new std::thread(
                        event_base_dispatch, 
                        CRedisClient::s_eventBase);
            // event_base_dispatch(s_eventBase);

            timeval t{1, 0};      // set 1s
            s_pRedisContext = redisConnectWithTimeout(ip, port, t);

            s_Instance = new CRedisClient();
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
    LOG(L_INFO) << "close redis";
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
    LOG(L_INFO) << "async cmd : " << format;
    return ::redisAsyncCommand(s_pRedisAsyncContext, 
                                pushCallback,
                                (void*)format,
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
        LOG(L_INFO) << res;
        return;
    }
    else
    {
        std::string _time = std::to_string(time_out_s);
        if (_time.empty())
            return;

        std::string cmd("SETEX ");
        cmd = cmd + key + " " + _time + " " + value;
        int res = this->AsyncCommand(cmd.c_str());
        std::cout << "res" << res << std::endl;
        return;
    }
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

}


