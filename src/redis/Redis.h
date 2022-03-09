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
    virtual void Set(const char* key, const char* value, int time_out_s = -1) override;

    // List

    // Set

    // 

// Sync
    // String
    virtual void Get(const std::string& key, std::string& value) override;

    // List

    // Set

    // 



















    static CRedisClient*    CreateClient(const char* ip, uint port);
    static CRedisClient*    getInstance();

    void SetActive(bool active) {this->_isActive = active; };
    bool IsActive(){ return this->_isActive; };

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

    bool _isActive{false};
};



}


