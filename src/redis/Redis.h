#include "RedisClientInterface.h"
#include <mutex>
#include "Logger.h"

namespace wlb
{



class CRedisClient:public IRedisClient
{
public:

// Async
    // String
    virtual void Set(const char* key, const char* value) override;

    // List

    // Set

    // 

// Sync




















    static CRedisClient*    CreateClient(const char* ip, uint port);
    static CRedisClient*    getInstance();

private:
    CRedisClient() {}
    ~CRedisClient() ;
    
    int                     AsyncCommand(const char* format);
    void*                   Command(const char* format);

private:
    static std::mutex           _mutex;
    static CRedisClient*        s_Instance;

    static redisContext*        s_pRedisContext;

    static redisAsyncContext*   s_pRedisAsyncContext;
    static event_base*          s_eventBase;

    static std::thread*         s_pThread;

};



}


