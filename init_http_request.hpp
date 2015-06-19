#pragma once
#include <unordered_map>
#include <memory>
#include <string>

#include "socket/socket_callbacks.hpp"
#include "socket/tcp/client.hpp"
#include "common/url.hpp"

class InitHttpRequest : public SocketCallbacks
{
    enum
    {
        WAIT_FOR_READ,
        WAIT_FOR_WRITE,

    };

public:
    InitHttpRequest(const Url & url) : m_url(url) {}
    ~InitHttpRequest();

    virtual void on_read(Socket *) override;
    virtual void on_write(Socket *) override;
    virtual void on_error(Socket *) override;

//    virtual void on_accept(Socket *, const NewConnection &);
    virtual void on_close(Socket *) override;
    virtual void on_connected(Socket *) override;
    virtual void on_rearm(Socket *) override;

private:
    uint64_t m_state {WAIT_FOR_WRITE};
    Url m_url;
    uint8_t buffer[50000];
    uint64_t offset {0};

};

