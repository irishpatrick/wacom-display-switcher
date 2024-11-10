#ifndef INSTANCEMUTEX_HPP
#define INSTANCEMUTEX_HPP

#include <string>

class InstanceMutex
{
public:
    InstanceMutex()
    {
        Acquire();
    }
    ~InstanceMutex()
    {
        Release();
    }

    void Acquire();
    void Release();
    bool IsHeld() const;

private:
    int fd;
    bool acquired = false;
    std::string fn;
};

#endif /* INSTANCEMUTEX */