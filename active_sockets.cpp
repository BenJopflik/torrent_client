#include "active_sockets.hpp"

#include <mutex>

ActiveSockets::ActiveSockets()
{

}

ActiveSockets::~ActiveSockets()
{

}

bool ActiveSockets::add(const LightPeerInfo & lpi)
{
    std::lock_guard<decltype(m_lock)> lock(m_lock);

    return m_peers.insert(lpi).second;
}

bool ActiveSockets::remove(const LightPeerInfo & lpi)
{
    std::lock_guard<decltype(m_lock)> lock(m_lock);
    return m_peers.erase(lpi);
}

uint64_t ActiveSockets::size() const
{
    std::lock_guard<decltype(m_lock)> lock(m_lock);
    return m_peers.size();
}
