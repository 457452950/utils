#include <string>

namespace wlb::NetWork
{

class WBaseService
{
public:
    virtual ~WBaseService() {}
    virtual void OnMessage(const std::string& recv_message, std::string& send_message) = 0;
    virtual void OnConnected();
    virtual void OnDisconnected();
    virtual void OnError(const std::string& error_message);
};







}
