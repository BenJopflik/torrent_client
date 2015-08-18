#include "client_logic.hpp"

#include "socket/socket.hpp"
#include "bedecoder/be_parser.hpp"
#include "tracker_response.hpp"
#include "socket/tcp/client.hpp"
#include "client_logic.hpp"
#include <iomanip>
#include <thread>
#include <arpa/inet.h>

ClientLogic::~ClientLogic()
{
    std::cerr << "~ClientLogic: " << std::hex << (uint64_t )this << std::dec << " from " << std::this_thread::get_id << std::endl;
}

void ClientLogic::on_read(Socket *socket)
{
    if (m_state != WAIT_FOR_READ)
        return;

    std::cerr << "onread" << std::endl;
    bool eof = false;
    int64_t readed = 0;
    int64_t processed = 0;
    while ((readed = socket->read(buffer + buffer_size, 500000 - buffer_size, eof)))
    {
        std::cerr << readed << " " << buffer << std::endl;
        buffer_size += readed;
        try
        {
            processed = m_peer_protocol.process_message(buffer, buffer_size);
        }
        catch (...)
        {
            throw 1;
        }

        if (!processed)
        {
            if (buffer_size > 5)
            {
                uint32_t msg_size = ntohl(*(uint32_t *)buffer);
                uint8_t message_id = *(buffer + 4);
                if (message_id > 8)
                    throw 1;
            }
            if (buffer_size == 500000)
            {
                throw 1;
                return;
            }
            continue;
        }

        if (processed < buffer_size)
        {
            memmove(buffer, buffer + processed, buffer_size - processed);
            buffer_size -= processed;
        }
        else
        {
            buffer_size = 0;
        }

        if (buffer_size == 500000)
        {
            throw 1;
            return;
        }
    }

    if (m_peer_protocol.has_message())
        m_state = WAIT_FOR_WRITE;
}

void ClientLogic::on_write(Socket *socket)
{
    if (m_state != WAIT_FOR_WRITE)
        return;

    std::cerr << "onwrite" << std::endl;

    bool message_sended = false;

    while (true)
    {
        auto message = m_peer_protocol.get_message();
        if (!message.size)
            break;
        message_sended = true;
        auto writed = socket->write(message.data.get(), message.size);
        std::cerr << "writed " << writed << std::endl;
    }

    m_state = message_sended ? WAIT_FOR_READ : WAIT_FOR_WRITE;
//    std::string buffer = "GET /";
//    std::string tail = m_url.tail();
//    tail = tail.substr(0, tail.size() - 1);
//    buffer += tail;
//    buffer += " HTTP/1.1\r\nHost: ";
//    buffer.append(m_url.host());
//    buffer += ":";
//    buffer += m_url.port();
//    buffer += "\r\nAccept: */*\r\nUser-Agent: asdf\r\nConnection: close\r\n\r\n";
//    std::cerr << "request: " << buffer << std::endl;
//    auto writed = socket->write((uint8_t *)buffer.data(), buffer.size());
//    std::cerr << "writed " << writed << std::endl;
}

void ClientLogic::on_error(Socket * socket)
{
    std::cerr << "error: " << socket->get_last_error() << std::endl;
//    m_active_sockets.remove(socket->get_fd());
}

//void ClientLogic::on_accept(Socket *, const NewConnection &)
//{
//    //
//}

void ClientLogic::on_close(Socket *socket, int64_t fd)
{
    std::cerr << "onclose" << std::endl;
//    m_active_sockets.remove(fd);
}

void ClientLogic::on_connected(Socket * socket)
{
    std::cerr << "connected to " << socket->get_remote_addr() << std::endl;
}

void ClientLogic::on_rearm(Socket * socket)
{
    std::cerr << "onrearm " << char(m_state + '0') << std::endl;
    if (m_state == WAIT_FOR_WRITE)
        socket->add_to_poller(Action::WRITE);
    else if (m_state == WAIT_FOR_READ)
        socket->add_to_poller(Action::READ);

}
