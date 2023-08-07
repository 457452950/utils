#include "wutils/redis/RedisClientInterface.h"
#include <mutex>

namespace wutils {

/*
REDIS_REPLY_STRING      1       reply->str
REDIS_REPLY_ARRAY       2       redis->elements
REDIS_REPLY_INTERGER    3       reply->integer
REDIS_REPLY_NIL         4
REDIS_REPLY_STATUS      5       reply->str
REDIS_REPLY_ERROR       6       reply->str
*/

/**
 * base on hiredis
 */
class RedisClient final : public IRedisClient {
public:
    // Async
    // String

    // List

    // Set

    //


    // Sync
    // String
    void Set(const Key &key, const Field &value, int time_out_sec) override;
    void Get(const Key &key, Field &value) override;

    // Hash
    bool HSetNX(const Key &key, const Field &field, const Value &value) override;
    bool HSetNX(const Key &key, int field, const Value &value) override;

    void HDEL(const Key &key, int field) override;

    void HGetAll(const Key &key, std::vector<std::tuple<Value, Value>> &values) override;

    // List

    // Set
    void SAdd(const Key &key, const Value &value) override;
    bool SAdd(const Key &key, int32_t value) override;
    bool SIsMember(const Key &key, const Value &value) override;

    //


    // other
    void Del(const Key &key) override;
    void INCR(const Key &key) override; // 自增
    void DECR(const Key &key) override; // 自减
    void INCRBY(const Key &key, int32_t value) override;
    void DECRBY(const Key &key, int32_t value) override;


    static RedisClient *CreateClient(const std::string &ip, uint8_t port);
    static RedisClient *getInstance();

    void SetActive(bool active) { this->is_active_ = active; };
    bool IsActive() const { return this->is_active_; };

private:
    RedisClient() {}
    ~RedisClient();

    redisReply *Command(const char *format, ...);

private:
    static RedisClient *instance_;

    redisContext *hiredis_context_{nullptr};
    bool          is_active_{false};
};


} // namespace wutils
