#pragma once

class WTimerHandle
{
public:
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void OnTimer();
    };

};


