#pragma once
#ifndef UTILS_WBASESESSION_H
#define UTILS_WBASESESSION_H

#include <memory>

#include "WChannel.h"

namespace wutils::network {

class WBaseSession : public WChannel::Listener {
public:
    virtual ~WBaseSession() {}

protected:
    std::unique_ptr<WChannel> channel_;
};


} // namespace wutils::network

#endif // UTILS_WBASESESSION_H
