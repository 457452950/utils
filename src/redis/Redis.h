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

    // List

    // Set

    //


// Sync
    // String
    void Set(const char* key, const char* value, int time_out_s = -1) override;
    void Get(const std::string& key, std::string& value) override;

    // Hash
    bool HSetNX(const Key& key, const Field& field, const Value& value) override;
    bool HSetNX(const Key& key, int field, const Value& value) override;

    void HDEL(const Key& key, int field) override;
    
    void HGetAll(const Key& key, std::vector<std::tuple<Value, Value>>& values) override;

    // List

    // Set
    void SAdd(const Key& key, const Value& value) override;
    bool SAdd(const Key& key, int32_t value) override;
    bool SIsMember(const Key& key, const Value& value) override;

    // 


    // other
    void Del(const Key& key) override;
    void INCR(const Key& key) override; // 自增
    void DECR(const Key& key) override; // 自减
    void INCRBY(const Key& key, int32_t value) override;
    void DECRBY(const Key& key, int32_t value) override;

















    static CRedisClient*    CreateClient(const char* ip, uint port);
    static CRedisClient*    getInstance();

    void SetActive(bool active) {this->_isActive = active; };
    bool IsActive(){ return this->_isActive; };

private:
    CRedisClient() {}
    ~CRedisClient() ;
    
    redisReply*                   Command(const char* format, ...);

private:
    static std::mutex           _mutex;
    static CRedisClient*        s_Instance;

    static redisContext*        s_pRedisContext;

    bool _isActive{false};
};



}


