#include <cstring>
#include <string>
#include <mutex>

#include <vector>
#include <set>

#include <hiredis/hiredis.h>
#include <hiredis/async.h>

namespace wlb
{


class CRedisClient;

class IRedisClient
{
public:

// Async
    // String
    virtual void Set(const char* key, const char* value) = 0;

    // List

    // Set

    // 

// Sync




    virtual ~IRedisClient() = default;
private:
};

IRedisClient* CreateRedisClient(const char* ip, uint port);
IRedisClient* GetRedisClient();


}


