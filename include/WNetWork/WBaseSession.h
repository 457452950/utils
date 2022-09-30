#pragma once
#ifndef _UTILS_WBASESESSION_H

#include <memory>

#include "WChannel.h"

namespace wlb::network {

class WBaseSession : public WChannel::Listener {
public:
    virtual ~WBaseSession() {}

protected:
    std::unique_ptr<WChannel> channel_;
};


} // namespace wlb::network

#endif
