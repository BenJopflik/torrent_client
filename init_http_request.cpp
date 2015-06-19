#include "init_http_request.hpp"

#include <sys/epoll.h>
#include <cstring>
#include "socket/socket.hpp"
#include "bedecoder/be_parser.hpp"
#include "tracker_response.hpp"

InitHttpRequest::~InitHttpRequest()
{

}

void InitHttpRequest::on_read(Socket *socket)
{
    if (m_state != WAIT_FOR_READ)
        return;

    std::cerr << "onread" << std::endl;
    bool eof;
    int64_t readed = 0;
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
        std::cerr << "incomplete buffer" << std::endl;
        return;
    }

    delim += 4;
    readed = readed - ((uint8_t * )delim - buffer);

    auto tokens = BeParser::parse((const char *)delim, readed);

    TrackerResponse tr(tokens, delim, readed);
    tr.print();


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

void InitHttpRequest::on_write(Socket *socket)
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

void InitHttpRequest::on_error(Socket * socket)
{
    std::cerr << "error: " << socket->get_last_error() << std::endl;
}

//void InitHttpRequest::on_accept(Socket *, const NewConnection &)
//{
//    //
//}

void InitHttpRequest::on_close(Socket *socket)
{
    std::cerr << "onclose" << std::endl;
}

void InitHttpRequest::on_connected(Socket * socket)
{
    std::cerr << "connected to " << socket->get_remote_addr() << std::endl;
}

void InitHttpRequest::on_rearm(Socket * socket)
{
    std::cerr << "onrearm" << std::endl;
    uint64_t mask = EPOLLONESHOT;
    if (m_state == WAIT_FOR_WRITE)
        mask |= EPOLLOUT;
    else
        mask |= EPOLLIN;

    socket->add_to_poller(mask);
}
