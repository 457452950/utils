#pragma once
#ifndef UTILS_BASESESSION_H
#define UTILS_BASESESSION_H

#include <memory>

#include "wutils/SharedPtr.h"

#include "Channel.h"

namespace wutils::network {

class BaseSession : public Channel::Listener {
public:
    ~BaseSession() override = default;

protected:
    unique_ptr<Channel> channel_;
};


} // namespace wutils::network

#endif // UTILS_BASESESSION_H
