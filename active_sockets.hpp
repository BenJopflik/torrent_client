#pragma once
#include <unordered_map>
#include <memory>

#include "socket/socket.hpp"
#include "thread/spinlock.hpp"

class ActiveSockets
{
public:
    ActiveSockets();
   ~ActiveSockets();

    bool add(std::unique_ptr<Socket>);
    bool remove(uint64_t fd);

private:
    std::unordered_map<uint64_t, std::unique_ptr<Socket>> m_sockets;
    SpinlockYield m_lock;

};
