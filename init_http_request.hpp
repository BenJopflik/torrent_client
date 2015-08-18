#pragma once
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>
#include <memory>

#include "socket/socket_callbacks.hpp"
#include "socket/tcp/client.hpp"
#include "common/url.hpp"
#include "active_sockets.hpp"

class PieceList;
class Files;

class TrackerConnection : public SocketCallbacks
{
    enum
    {
        WAIT_FOR_READ,
        WAIT_FOR_WRITE,

    };

public:
    TrackerConnection(const Url & url,
                      ActiveSockets & active_sockets,
                      const std::string & info_hash,
                      Files & files,
                      PieceList & pl) : m_url(url),
                                      m_active_sockets(active_sockets),
                                      m_info_hash(info_hash),
                                      m_files(files),
                                      m_piece_list(pl){}
    ~TrackerConnection();

    virtual void on_read(Socket *) override;
    virtual void on_write(Socket *) override;
    virtual void on_error(Socket *) override;

//    virtual void on_accept(Socket *, const NewConnection &);
    virtual void on_close(Socket *, int64_t fd) override;
    virtual void on_connected(Socket *) override;
    virtual void on_rearm(Socket *) override;

private:
    uint64_t m_state {WAIT_FOR_WRITE};
    Url m_url;
    ActiveSockets & m_active_sockets;
    uint8_t buffer[50000];
    uint64_t offset {0};
    std::string m_info_hash;
    Files & m_files;
    PieceList & m_piece_list;

};

