#include "WNetWork/WBaseSession.hpp"

namespace wlb::NetWork
{


SessionNode* CreateNewSessionNodeAndInit(WBaseSession::Listener* listener, WNetWorkHandler* handler)
{
    auto* session = CreateNewSession(listener);
    if (session == nullptr)
        return nullptr;
    
    SessionNode* node = new(std::nothrow) SessionNode();
    if (node != nullptr && session->Init(node, handler))
    {
        node->val = session;
        return node;
    }
    return nullptr;
}




}
