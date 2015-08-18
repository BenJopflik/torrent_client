#pragma once
#include <unordered_set>
#include <memory>

#include "common/hashable.hpp"
#include "common/hasher.hpp"
#include "socket/socket.hpp"
#include "thread/spinlock.hpp"

struct LightPeerInfo : public Hashable<LightPeerInfo>
{
public:
    LightPeerInfo(uint32_t address, uint16_t port) : address(address), port(port) {}
    uint64_t hash() const {return std::hash<uint64_t>()(*(uint64_t *)(&this->address));}
    bool operator == (const LightPeerInfo & right) const {return address == right.address && port == right.port;}

public:
    uint32_t address {0};
    uint16_t port    {0};

private:
    uint16_t padding {0};
};


class ActiveSockets
{
public:
    ActiveSockets();
   ~ActiveSockets();

    bool add(const LightPeerInfo & peer);
    bool remove(const LightPeerInfo & peer);
    uint64_t size() const;

private:
    std::unordered_set<LightPeerInfo, Hasher<LightPeerInfo>> m_peers;
    mutable SpinlockYield m_lock;

};
