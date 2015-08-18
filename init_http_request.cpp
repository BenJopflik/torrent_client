#include "init_http_request.hpp"

#include <cstring>
#include "socket/socket.hpp"
#include "bedecoder/be_parser.hpp"
#include "tracker_response.hpp"
#include "socket/tcp/client.hpp"
#include "client_logic.hpp"

TrackerConnection::~TrackerConnection()
{

}

void TrackerConnection::on_read(Socket *socket)
{
    if (m_state != WAIT_FOR_READ)
        return;

    std::cerr << "onread" << std::endl;
    bool eof = false;
    int64_t readed = 0;
    offset = 0;
    memset(buffer, 0, 50000); // for valgrind
    while ((readed = socket->read(buffer + offset, 50000 - offset, eof)))
    {
        std::cerr << readed << " " << buffer << std::endl;
        offset += readed;
    }
    buffer[offset] = 0;
    readed = offset;
    char * delim = strstr((char *)buffer, "\r\n\r\n");

    if (!delim)
    {
        delim = strstr((char *)buffer, "\n\n");
        if (!delim)
        {
            std::cerr << "incomplete buffer" << std::endl;
            return;
        }
        delim += 2;
    }
    else
        delim += 4;

    readed = readed - ((uint8_t * )delim - buffer);

    auto tokens = BeParser::parse({(const char *)delim, readed});

    TrackerResponse tr(tokens, delim, readed);

    if (tr)
        tr.print();
    else
        std::cerr << tr.failure_reason() << std::endl;

    for (const auto & peer : tr.peers())
    {
        try
        {
//            std::unique_ptr<Socket> client_(new TcpClient(peer.saddr));
            Socket * client = create_tcp_client(peer.saddr);
            auto remote_ip = client->get_remote_ip();
            m_active_sockets.remove(LightPeerInfo(remote_ip.addr.sin_addr.s_addr, remote_ip.port));
            client->set_callbacks<ClientLogic>(std::ref(m_active_sockets), std::ref(m_files), std::ref(m_piece_list), std::ref(m_info_hash));
            client->add_to_poller(Action::WRITE, socket->get_poller());
//            break; // XXX use first client
        }
        catch (const std::exception & e)
        {
            std::cerr << e.what() << std::endl;
        }
    }

//    for (const auto & token : tokens)
//    {
//        switch (token.type())
//        {
//            case BeToken::LIST_START:
//                std::cerr << "LIST_START" << std::endl;
//                break;
//
//            case BeToken::LIST_END:
//                std::cerr << "LIST_END" << std::endl;
//                break;
//
//            case BeToken::DICT_START:
//                std::cerr << "DICT_START" << std::endl;
//                break;
//
//            case BeToken::DICT_END:
//                std::cerr << "DICT_END" << std::endl;
//                break;
//
//            default:
//                std::cerr << token.str((const char *)delim, readed) << std::endl;
//                break;
//        }
//    }

}

void TrackerConnection::on_write(Socket *socket)
{
    if (m_state != WAIT_FOR_WRITE)
        return;

    std::cerr << "onwrite" << std::endl;
    std::string buffer = "GET /";
    std::string tail = m_url.tail();
    tail = tail.substr(0, tail.size() - 1);
    buffer += tail;
    buffer += " HTTP/1.1\r\nHost: ";
    buffer.append(m_url.host());
    buffer += ":";
    buffer += m_url.port();
    buffer += "\r\nAccept: */*\r\nUser-Agent: asdf\r\nConnection: close\r\n\r\n";
    std::cerr << "request: " << buffer << std::endl;
    auto writed = socket->write((uint8_t *)buffer.data(), buffer.size());
    std::cerr << "writed " << writed << std::endl;
    m_state = WAIT_FOR_READ;
}

void TrackerConnection::on_error(Socket * socket)
{
    std::cerr << "error: " << socket->get_last_error() << std::endl;
}

//void TrackerConnection::on_accept(Socket *, const NewConnection &)
//{
//    //
//}

void TrackerConnection::on_close(Socket *socket, int64_t fd)
{
    std::cerr << "onclose" << std::endl;
    auto remote_ip = socket->get_remote_ip();
    m_active_sockets.remove(LightPeerInfo(remote_ip.addr.sin_addr.s_addr, remote_ip.port));
}

void TrackerConnection::on_connected(Socket * socket)
{
    std::cerr << "connected to " << socket->get_remote_addr() << std::endl;
}

void TrackerConnection::on_rearm(Socket * socket)
{
    std::cerr << "onrearm" << std::endl;
    if (m_state == WAIT_FOR_WRITE)
        socket->add_to_poller(Action::WRITE);
    else
        socket->add_to_poller(Action::READ);

}
