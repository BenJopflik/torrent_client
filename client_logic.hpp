#pragma once

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>

//#include "socket/socket_callbacks.hpp"
#include "socket/tcp/client.hpp"
#include "common/url.hpp"
#include "active_sockets.hpp"
#include "peer_protocol.hpp"
#include "files.hpp"
#include "piece_list.hpp"

class ClientLogic: public SocketCallbacks
{
    enum
    {
        ERROR,
        WAIT_FOR_READ,
        WAIT_FOR_WRITE,

    };

public:
    ClientLogic(ActiveSockets & active_sockets,
                Files & files,
                PieceList & pl,
                const std::string & info_hash) : m_active_sockets(active_sockets),
                                                 m_peer_protocol(info_hash, files, pl)
    {}
    ~ClientLogic();

    virtual void on_read(Socket *) override;
    virtual void on_write(Socket *) override;
    virtual void on_error(Socket *) override;

//    virtual void on_accept(Socket *, const NewConnection &);
    virtual void on_close(Socket *, int64_t fd) override;
    virtual void on_connected(Socket *) override;
    virtual void on_rearm(Socket *) override;

private:
    ActiveSockets & m_active_sockets;
    uint64_t m_state {WAIT_FOR_WRITE};
    uint8_t buffer[500000];
    uint64_t buffer_size = 0;
    PeerProtocol m_peer_protocol;
};

