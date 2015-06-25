#include "active_sockets.hpp"

#include <mutex>

ActiveSockets::ActiveSockets()
{

}

ActiveSockets::~ActiveSockets()
{

}

bool ActiveSockets::add(std::unique_ptr<Socket> socket)
{
    std::lock_guard<decltype(m_lock)> lock(m_lock);

    uint64_t fd = socket->get_fd();

    return m_sockets.emplace(fd, std::move(socket)).second;
}

bool ActiveSockets::remove(uint64_t fd)
{
    std::lock_guard<decltype(m_lock)> lock(m_lock);
    return m_sockets.erase(fd);
}
