#pragma once
#include <string>
#include <list>
#include "files.hpp"
#include "piece.hpp"
#include "piece_list.hpp"
#include "message_types.hpp"



// http://jonas.nitro.dk/bittorrent/bittorrent-rfc.html

struct Message
{
public:
    Message() {}
    Message(uint8_t * data, uint64_t size) : data(data), size(size) {}

public:
    std::unique_ptr<uint8_t[]> data;
    uint64_t size {0};
};

class PeerProtocol
{
    enum State
    {
        HANDSHAKE,
        ACTION


    };

#define IN_ARGS const uint8_t * data, uint64_t size
#define PROCESS_IN(function_name) void function_name(IN_ARGS);

public:
    PeerProtocol(const std::string & info_hash, Files & files, PieceList & pl);
    ~PeerProtocol();

    Message get_message();
    uint64_t process_message(const uint8_t * data, uint64_t size);
    bool has_message() const {return !m_messages.empty();}

private:
    void handshake();
    void create_interested();
    bool get_piece_index();
    void create_request();

    PROCESS_IN(process_handshake)

    PROCESS_IN(process_choke)
    PROCESS_IN(process_unchoke)
    PROCESS_IN(process_interested)
    PROCESS_IN(process_uninterested)
    PROCESS_IN(process_have)
    PROCESS_IN(process_bitfield)
    PROCESS_IN(process_request)
    PROCESS_IN(process_piece)
    PROCESS_IN(process_cancel)
    PROCESS_IN(process_port)

private:
    std::string m_info_hash;

    const decltype(&PeerProtocol::process_choke) PROCESS_IN_MESSAGE[NUM_OF_TYPES] =
                                               {
                                                    &PeerProtocol::process_choke,
                                                    &PeerProtocol::process_unchoke,
                                                    &PeerProtocol::process_interested,
                                                    &PeerProtocol::process_uninterested,
                                                    &PeerProtocol::process_have,
                                                    &PeerProtocol::process_bitfield,
                                                    &PeerProtocol::process_request,
                                                    &PeerProtocol::process_piece,
                                                    &PeerProtocol::process_cancel,
                                                    &PeerProtocol::process_port
                                                };

    uint64_t m_state {HANDSHAKE};
    BitSet m_clients_pieces;
    int64_t m_piece_index {-1};
    uint64_t m_piece_size {0};
    uint64_t m_offset;
    Piece m_piece;
    Files & m_files;
    PieceList & m_piece_list;
    std::list<Message> m_messages;
    uint64_t m_number_of_pieces;
};
#undef PROCESS_IN
