#include "peer_protocol.hpp"
#include <vector>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <arpa/inet.h>
#include <assert.h>

#include "common/bitset.hpp"

static const std::string PROTOCOL_NAME {"BitTorrent protocol"};
static const uint64_t    PROTOCOL_NAME_SIZE {PROTOCOL_NAME.size()};
static const uint64_t    HANDSHAKE_SIZE {68}; // according to protocol specification
static const uint64_t    BLOCK_SIZE {1 << 14};


PeerProtocol::PeerProtocol(const std::string & info_hash,
                           Files & files,
                           PieceList & pl) : m_info_hash(info_hash),
                                             m_files(files),
                                             m_piece_list(pl),
                                             m_number_of_pieces(m_piece_list.number_of_pieces())
{
    m_clients_pieces.bind(m_number_of_pieces / 8 + 1);
    handshake();
}

PeerProtocol::~PeerProtocol()
{

}

static std::vector<uint8_t> generate_random(uint64_t length)
{
    std::vector<uint8_t> random_string;
    random_string.reserve(length);

    srand(time(NULL));
    for (uint64_t i = 0; i < length; ++i)
        random_string[i] = '0' + (rand() % 10);

    return std::move(random_string);
}

void PeerProtocol::handshake()
{
    uint8_t * handshake = new uint8_t[HANDSHAKE_SIZE];
    uint8_t * data = handshake;
    *data = PROTOCOL_NAME_SIZE;
    ++data;
    memcpy(data, PROTOCOL_NAME.data(), PROTOCOL_NAME_SIZE);
    data += PROTOCOL_NAME_SIZE;
    data += 8; // reserve

    memcpy(data, m_info_hash.data(), m_info_hash.size());
    data += 20; // info hash size
    auto random_string = generate_random(20);
    memcpy(data, random_string.data(), 20);
    m_messages.emplace_back(handshake, HANDSHAKE_SIZE);
}

Message PeerProtocol::get_message()
{
    if (m_messages.empty())
        return Message();

    Message output;
    std::swap(output, m_messages.front());
    m_messages.pop_front();
    return std::move(output);
}

#define PROCESS_IN(function_name) void PeerProtocol::function_name(IN_ARGS)

uint64_t PeerProtocol::process_message(const uint8_t * data, uint64_t size)
{
    const uint8_t * curr = data;
    const uint8_t * const END = data + size;


    uint32_t message_size = 0;
    static const uint64_t MESSAGE_LENGTH_SIZE = sizeof(message_size);

    uint8_t message_id = 0;

    if (m_state == HANDSHAKE)
    {
        if (curr + HANDSHAKE_SIZE > END)
            return curr - data;

        process_handshake(curr, HANDSHAKE_SIZE);
        curr += HANDSHAKE_SIZE;
        m_state = ACTION;
    }

    while (curr + MESSAGE_LENGTH_SIZE < END)
    {
        message_size = ntohl(*(uint32_t * )(curr));
        if (!message_size)
        {
            std::cerr << "keep alive message. skip" << std::endl;
            curr += MESSAGE_LENGTH_SIZE;
            continue;
        }

        if (curr + message_size + MESSAGE_LENGTH_SIZE > END)
            break;

        message_id = *(curr + MESSAGE_LENGTH_SIZE);
        if (message_id < NUM_OF_TYPES)
            (this->*(PROCESS_IN_MESSAGE[message_id]))(curr, message_size + MESSAGE_LENGTH_SIZE);
        else
            throw 1;

        curr += message_size + MESSAGE_LENGTH_SIZE;
    }

    return curr - data;
}

PROCESS_IN(process_handshake)
{
    std::cerr << "PROCESS_HANDSHAKE" << std::endl;
}

PROCESS_IN(process_choke)
{
    std::cerr << "PROCESS_CHOKE" << std::endl;
    m_piece_list.return_piece(m_piece_index);
    m_piece_index = -1;
    m_piece_size = 0;
    m_piece.clear();
}

PROCESS_IN(process_unchoke)
{
    std::cerr << "PROCESS_UNCHOKE" << std::endl;
    if (get_piece_index())
        create_request();
}

PROCESS_IN(process_interested)
{
    std::cerr << "PROCESS_INTERESTED" << std::endl;
}

PROCESS_IN(process_uninterested)
{
    std::cerr << "PROCESS_UNINTERESTED" << std::endl;
}

PROCESS_IN(process_have)
{
    std::cerr << "PROCESS_HAVE" << std::endl;
    const HaveMessage * message = reinterpret_cast<const HaveMessage *>(data);
    uint32_t piece_id = ntohl(message->index);
    m_clients_pieces[piece_id] = true;
}

PROCESS_IN(process_bitfield)
{
    std::cerr << "PROCESS_BITFIELD" << std::endl;
    const BitfieldMessage * message = reinterpret_cast<const BitfieldMessage *>(data);
    m_clients_pieces.bind(size - sizeof(BaseMessage), message->data);
    std::cerr << m_clients_pieces.to_string() << std::endl;
    create_interested();
}

PROCESS_IN(process_request)
{
    std::cerr << "PROCESS_REQUEST" << std::endl;
}

PROCESS_IN(process_piece)
{
    std::cerr << "PROCESS_PIECE" << std::endl;
    const PieceMessage * message = reinterpret_cast<const PieceMessage *>(data);
    if (ntohl(message->index) != m_piece_index)
        return;

    m_piece.add(ntohl(message->index), message->data, size - sizeof(BaseMessage) - 8, ntohl(message->offset));
    if (m_piece.full())
    {
        m_files.write_piece(m_piece_index, m_piece.get().get(), m_piece_size);

        m_offset = 0;
        m_piece_index = -1;
        std::cerr << "FULL_PIECE" << std::endl;

        if (get_piece_index())
            create_request();
    }
}

PROCESS_IN(process_cancel)
{
    std::cerr << "PROCESS_CANCEL" << std::endl;
}

PROCESS_IN(process_port)
{
    std::cerr << "PROCESS_PORT" << std::endl;
}


void PeerProtocol::create_interested()
{
    static const uint32_t INTERESTED_SIZE = sizeof(InterestedMessage);
    uint8_t * interested = new uint8_t[INTERESTED_SIZE];
    new (interested) InterestedMessage();
    m_messages.emplace_back(interested, INTERESTED_SIZE);
}

bool PeerProtocol::get_piece_index()
{
  // XXX 20 - number of attempts. remove
    uint64_t i = 0;
    for (; i < 20; ++i)
    {
        m_piece_size = m_piece_list.get_piece(m_piece_index);
        if (m_piece_size && m_clients_pieces[m_piece_index])
            break;

        m_piece_size = 0;
        m_piece_list.return_piece(m_piece_index);
        m_piece_index = -1;
    }

    if (!m_piece_size)
        return false;

    m_piece.init(m_piece_index, m_piece_size);
    return true;
}

void PeerProtocol::create_request()
{
    const uint32_t REQUEST_SIZE = sizeof(RequestMessage);
    m_offset = 0;
    while (m_offset < m_piece_size)
    {
        uint8_t * request = new uint8_t[REQUEST_SIZE];
        RequestMessage * message = new (request) RequestMessage(m_piece_index, m_offset, (m_offset + BLOCK_SIZE > m_piece_size) ? m_piece_size - m_offset : BLOCK_SIZE);
        m_messages.emplace_back(request, REQUEST_SIZE);
        m_offset += BLOCK_SIZE;
    }
}

#undef PROCESS_IN
