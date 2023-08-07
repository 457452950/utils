#include "Redis.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>

namespace wutils {

IRedisClient *CreateRedisClient(const char *ip, uint port) { return RedisClient::CreateClient(ip, port); }
IRedisClient *GetRedisClient() { return RedisClient::getInstance(); }

void connectCallback(const redisAsyncContext *c, int status) {
    if(status != REDIS_OK) {
        return;
    }
    RedisClient::getInstance()->SetActive(true);
}

void disconnectCallback(const redisAsyncContext *c, int status) {
    if(status != REDIS_OK) {
        std::cout << "errstr : " << c->errstr << std::endl;
        return;
    }
    RedisClient::getInstance()->SetActive(false);
    // ::exit(-1);
    std::cout << "Redis disconnected..." << std::endl;
}

void pushCallback(redisAsyncContext *c, void *r, void *privdata) {
    std::cout << "pushCallback " << std::endl;
    redisReply *reply = (redisReply *)r;
    if(reply == NULL)
        return;
    if(reply->type == 3) {
        std::cout << "reply integer " << reply->integer << std::endl;
    } else if(reply->type == 4) {
        std::cout << "reply nil " << reply->integer << std::endl;
    } else
        std::cout << "reply str" << reply->str << std::endl;
}

RedisClient *RedisClient::instance_ = nullptr;

RedisClient *RedisClient::CreateClient(const std::string &ip, uint8_t port) {
    if(instance_ != nullptr) {
        return instance_;
    }

    std::mutex       m;
    std::unique_lock lock(m);

    if(instance_ == nullptr) {
        instance_ = new(std::nothrow) RedisClient();

        if(instance_ == nullptr) {
            return nullptr;
        }

        timeval t{1, 0}; // time out set 1s
        instance_->hiredis_context_ = redisConnectWithTimeout(ip.c_str(), port, t);

        if(instance_->hiredis_context_ == nullptr || instance_->hiredis_context_->err != 0) {
            return nullptr;
        }
    }

    return instance_;
}

RedisClient *RedisClient::getInstance() { return instance_; }

RedisClient::~RedisClient() {
    if(hiredis_context_ != nullptr) {
        redisFree(hiredis_context_);
        hiredis_context_ = nullptr;
    }
}

redisReply *RedisClient::Command(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    auto *reply = static_cast<redisReply *>(::redisvCommand(hiredis_context_, format, ap));
    va_end(ap);
    return reply;
}

void RedisClient::Set(const Key &key, const Field &value, int time_out_sec) {
    if(time_out_sec <= 0) {
        auto *reply =
                static_cast<redisReply *>(::redisCommand(hiredis_context_, "SET %s %s ", key.c_str(), value.c_str()));
        if(reply == nullptr) {
            std::cout << "reply nullptr" << std::endl;
            return;
        } else if(reply->type == REDIS_REPLY_ERROR || reply->type == REDIS_REPLY_STRING) {
            std::cout << "reply : " << reply->str << std::endl;
        } else if(reply->type == REDIS_REPLY_INTEGER) {
            std::cout << "reply : " << reply->integer << std::endl;
        }
        freeReplyObject(reply);
        return;
    } else {
        auto *reply = static_cast<redisReply *>(
                ::redisCommand(hiredis_context_, "SETEX %s %d %s ", key.c_str(), time_out_sec, value.c_str()));
        if(reply == nullptr) {
            std::cout << "reply nullptr" << std::endl;
            return;
        } else if(reply->type == REDIS_REPLY_NIL) {
            std::cout << "reply nil" << std::endl;
        } else if(reply->type == REDIS_REPLY_INTEGER) {
            std::cout << "reply : " << reply->integer << std::endl;
        } else {
            std::cout << "reply : " << reply->str << std::endl;
        }
        freeReplyObject(reply);
        return;
    }
}
void RedisClient::Get(const Key &key, Field &value) {
    value.clear();

    std::cout << "key : " << key << std::endl;
    auto *reply = static_cast<redisReply *>(::redisCommand(hiredis_context_, "GET %s ", key.c_str()));
    if(reply == nullptr) {
        std::cout << "reply nullptr" << std::endl;
        return;
    } else if(reply->type == REDIS_REPLY_NIL) {
        std::cout << "reply nil" << std::endl;
    } else if(reply->type == REDIS_REPLY_INTEGER) {
        std::cout << "reply : " << reply->integer << std::endl;
    } else {
        std::cout << "reply : " << reply->str << std::endl;
    }

    if(reply->type == REDIS_REPLY_STRING)
        value = std::string(reply->str);
    freeReplyObject(reply);
}

void RedisClient::SAdd(const Key &key, const Value &value) {
    std::cout << "key : " << key << " value : " << value << std::endl;
    auto *reply =
            static_cast<redisReply *>(::redisCommand(hiredis_context_, "SADD %s %s ", key.c_str(), value.c_str()));
    if(reply == nullptr) {
        std::cout << "reply nullptr" << std::endl;
        return;
    } else if(reply->type == REDIS_REPLY_NIL) {
        std::cout << "reply nil" << std::endl;
    } else if(reply->type == REDIS_REPLY_INTEGER) {
        std::cout << "reply : " << reply->integer << std::endl;
    } else {
        std::cout << "reply : " << reply->str << std::endl;
    }
    freeReplyObject(reply);
}


// Hash
bool RedisClient::HSetNX(const Key &key, const Field &field, const Value &value) {
    redisReply *reply = static_cast<redisReply *>(
            ::redisCommand(hiredis_context_, "HSETNX %s %s %s ", key.c_str(), field.c_str(), value.c_str()));
    if(reply == nullptr) {
        std::cout << "reply nullptr" << std::endl;
        return false;
    } else if(reply->type == REDIS_REPLY_NIL) {
        std::cout << "reply nil" << std::endl;
    } else if(reply->type == REDIS_REPLY_INTEGER) {
        std::cout << "reply : " << reply->integer << std::endl;
    } else {
        std::cout << "reply : " << reply->str << std::endl;
    }

    bool ok = false;
    if(reply->type == REDIS_REPLY_INTEGER)
        ok = reply->integer;
    freeReplyObject(reply);
    return ok;
}
bool RedisClient::HSetNX(const Key &key, int field, const Value &value) {
    redisReply *reply = static_cast<redisReply *>(
            ::redisCommand(hiredis_context_, "HSETNX %s %d %s ", key.c_str(), field, value.c_str()));
    if(reply == nullptr) {
        std::cout << "reply nullptr" << std::endl;
        return false;
    } else if(reply->type == REDIS_REPLY_NIL) {
        std::cout << "reply nil" << std::endl;
    } else if(reply->type == REDIS_REPLY_INTEGER) {
        std::cout << "reply : " << reply->integer << std::endl;
    } else {
        std::cout << "reply : " << reply->str << std::endl;
    }

    bool ok = false;
    if(reply->type == REDIS_REPLY_INTEGER)
        ok = reply->integer;
    freeReplyObject(reply);
    return ok;
}

void RedisClient::HDEL(const Key &key, int field) {
    redisReply *reply = static_cast<redisReply *>(::redisCommand(hiredis_context_, "HDEL %s %d ", key.c_str(), field));
    printf("HDEL %s %d \n", key.c_str(), field);
    if(reply == nullptr) {
        std::cout << "reply nullptr" << std::endl;
        return;
    } else if(reply->type == REDIS_REPLY_NIL) {
        std::cout << "reply nil" << std::endl;
    } else if(reply->type == REDIS_REPLY_INTEGER) {
        std::cout << "reply : " << reply->integer << std::endl;
    } else {
        std::cout << "reply : " << reply->str << std::endl;
    }

    bool ok = false;
    if(reply->type == REDIS_REPLY_INTEGER)
        ok = reply->integer;
    freeReplyObject(reply);
    return;
}

void RedisClient::HGetAll(const Key &key, std::vector<std::tuple<Value, Value>> &values) {
    std::cout << "HGetAll " << key << std::endl;
    redisReply *reply = static_cast<redisReply *>(::redisCommand(hiredis_context_, "HGetAll %s ", key.c_str()));
    if(reply == nullptr) {
        std::cout << "reply nullptr" << std::endl;
        return;
    } else if(reply->type == REDIS_REPLY_NIL) {
        std::cout << "reply nil" << std::endl;
    } else if(reply->type == REDIS_REPLY_INTEGER) {
        std::cout << "reply : " << reply->integer << std::endl;
    } else if(reply->type != REDIS_REPLY_ARRAY) {
        std::cout << "reply : " << reply->str << std::endl;
    }

    if(reply->type == REDIS_REPLY_ARRAY) {
        for(size_t i = 0; i < reply->elements; i += 2) {
            //     std::cout << reply->element[i]->str << " " << reply->element[i+1]->str << std::endl;
            values.push_back(std::make_tuple(reply->element[i]->str, reply->element[i + 1]->str));
        }
    }

    // std::cout << values.size() << std::endl;
    freeReplyObject(reply);
}


// Set
bool RedisClient::SIsMember(const Key &key, const Value &value) {
    redisReply *reply =
            static_cast<redisReply *>(::redisCommand(hiredis_context_, "SISMEMBER %s %s ", key.c_str(), value.c_str()));
    if(reply == nullptr) {
        std::cout << "reply nullptr" << std::endl;
        return false;
    } else if(reply->type == REDIS_REPLY_NIL) {
        std::cout << "reply nil" << std::endl;
    } else if(reply->type == REDIS_REPLY_INTEGER) {
        std::cout << "reply : " << reply->integer << std::endl;
    } else {
        std::cout << "reply : " << reply->str << std::endl;
    }
    bool ok = false;
    if(reply->type == REDIS_REPLY_INTEGER)
        ok = reply->integer;
    freeReplyObject(reply);
    return ok;
}

bool RedisClient::SAdd(const Key &key, int32_t value) {
    std::cout << "key : " << key << " value : " << value << std::endl;
    redisReply *reply = static_cast<redisReply *>(::redisCommand(hiredis_context_, "SADD %s %s ", key.c_str(), value));
    if(reply == nullptr) {
        std::cout << "reply nullptr" << std::endl;
        return false;
    } else if(reply->type == REDIS_REPLY_NIL) {
        std::cout << "reply nil" << std::endl;
    } else if(reply->type == REDIS_REPLY_INTEGER) {
        std::cout << "reply : " << reply->integer << std::endl;
    } else {
        std::cout << "reply : " << reply->str << std::endl;
    }
    bool ok = false;
    if(reply->type == REDIS_REPLY_INTEGER)
        ok = reply->integer;
    freeReplyObject(reply);
    return ok;
}

void RedisClient::Del(const Key &key) {
    std::cout << "key : " << key << std::endl;
    redisReply *reply = static_cast<redisReply *>(::redisCommand(hiredis_context_, "DEL %s ", key.c_str()));
    if(reply == nullptr) {
        std::cout << "reply nullptr" << std::endl;
        return;
    } else if(reply->type == REDIS_REPLY_NIL) {
        std::cout << "reply nil" << std::endl;
    } else if(reply->type == REDIS_REPLY_INTEGER) {
        std::cout << "reply : " << reply->integer << std::endl;
    } else {
        std::cout << "reply : " << reply->str << std::endl;
    }
    freeReplyObject(reply);
    return;
}


void RedisClient::INCR(const Key &key) {
    std::cout << "key : " << key << std::endl;
    redisReply *reply = static_cast<redisReply *>(::redisCommand(hiredis_context_, "INCR %s ", key.c_str()));
    if(reply == nullptr) {
        std::cout << "reply nullptr" << std::endl;
        return;
    } else if(reply->type == REDIS_REPLY_NIL) {
        std::cout << "reply nil" << std::endl;
    } else if(reply->type == REDIS_REPLY_INTEGER) {
        std::cout << "reply : " << reply->integer << std::endl;
    } else {
        std::cout << "reply : " << reply->str << std::endl;
    }
    freeReplyObject(reply);
    return;
}
void RedisClient::DECR(const Key &key) {
    std::cout << "key : " << key << std::endl;
    redisReply *reply = static_cast<redisReply *>(::redisCommand(hiredis_context_, "DECR %s ", key.c_str()));
    if(reply == nullptr) {
        std::cout << "reply nullptr" << std::endl;
        return;
    } else if(reply->type == REDIS_REPLY_NIL) {
        std::cout << "reply nil" << std::endl;
    } else if(reply->type == REDIS_REPLY_INTEGER) {
        std::cout << "reply : " << reply->integer << std::endl;
    } else {
        std::cout << "reply : " << reply->str << std::endl;
    }
    freeReplyObject(reply);
    return;
}

void RedisClient::INCRBY(const Key &key, int32_t value) {
    std::cout << "key : " << key << std::endl;
    redisReply *reply =
            static_cast<redisReply *>(::redisCommand(hiredis_context_, "INCRBY %s %d ", key.c_str(), value));
    if(reply == nullptr) {
        std::cout << "reply nullptr" << std::endl;
        return;
    } else if(reply->type == REDIS_REPLY_NIL) {
        std::cout << "reply nil" << std::endl;
    } else if(reply->type == REDIS_REPLY_INTEGER) {
        std::cout << "reply : " << reply->integer << std::endl;
    } else {
        std::cout << "reply : " << reply->str << std::endl;
    }
    freeReplyObject(reply);
    return;
}
void RedisClient::DECRBY(const Key &key, int32_t value) {
    std::cout << "key : " << key << std::endl;
    redisReply *reply =
            static_cast<redisReply *>(::redisCommand(hiredis_context_, "DECRBY %s %d ", key.c_str(), value));
    if(reply == nullptr) {
        std::cout << "reply nullptr" << std::endl;
        return;
    } else if(reply->type == REDIS_REPLY_NIL) {
        std::cout << "reply nil" << std::endl;
    } else if(reply->type == REDIS_REPLY_INTEGER) {
        std::cout << "reply : " << reply->integer << std::endl;
    } else {
        std::cout << "reply : " << reply->str << std::endl;
    }
    freeReplyObject(reply);
    return;
}

} // namespace wutils
