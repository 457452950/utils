#pragma once
#ifndef UTILS_CHANNEL_H
#define UTILS_CHANNEL_H

namespace wutils::network::event {

/**
 *
 */
class IOEvent {
public:
    IOEvent()          = default;
    virtual ~IOEvent() = default;

    // nocopy
    IOEvent(const IOEvent &other)            = delete;
    IOEvent &operator=(const IOEvent &other) = delete;

    virtual void IOIn()  = 0;
    virtual void IOOut() = 0;
};

class IOReadEvent : public IOEvent {
public:
    IOReadEvent()           = default;
    ~IOReadEvent() override = default;

private:
    void IOOut() final{};
};

class IOWriteEvent : public IOEvent {
public:
    IOWriteEvent()           = default;
    ~IOWriteEvent() override = default;

private:
    void IOIn() final{};
};


} // namespace wutils::network::event


#endif // UTILS_CHANNEL_H