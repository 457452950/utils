#include "RedisClientInterface.h"
#include <mutex>
#include "Logger.h"

namespace wlb
{

/*
REDIS_REPLY_STRING      1       reply->str
REDIS_REPLY_ARRAY       2       redis->elements
REDIS_REPLY_INTERGER    3       reply->interger
REDIS_REPLY_NIL         4
REDIS_REPLY_STATUS      5       reply->str
REDIS_REPLY_ERROR       6       reply->str
*/

class CRedisClient:public IRedisClient
{
public:

// Async
    // String
    virtual void Set(const char* key, const char* value, int time_out_s = -1) override;

    // List

    // Set
    virtual void SAdd(const Key& key, const Value& value) override;
    virtual void SAdd(const Key& key, const ValueList& list) override;

    //

    // other
    void Del(const Key& key) override;

// Sync
    // String
    virtual void Get(const std::string& key, std::string& value) override;

    // List

    // Set
    bool SyncSAdd(const Key& key, int32_t value) override;
    bool SIsMember(const Key& key, const Value& value) override;

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


