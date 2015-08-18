#pragma once

#include <cstdint>
#include <arpa/inet.h>

enum MessageType
{
    CHOKE = 0,
    UNCHOKE,
    INTERESTED,
    NOT_INTERESTED,
    HAVE,
    BITFIELD,
    REQUEST,
    PIECE,
    CANCEL,
    PORT,

    NUM_OF_TYPES
};

#pragma pack(push, 1)
// no need in virtual destructor
struct BaseMessage
{
    BaseMessage(uint32_t length_, uint8_t id) : length(htonl(length_)), id(id) {}

    uint32_t length;
    uint8_t  id;
};

struct ChokeMessage : BaseMessage
{
    ChokeMessage() : BaseMessage(1, CHOKE) {}
};

struct UnchokeMessage : BaseMessage
{
    UnchokeMessage() : BaseMessage(1, UNCHOKE) {}
};

struct InterestedMessage : BaseMessage
{
    InterestedMessage() : BaseMessage(1, INTERESTED) {}
};

struct NotInterestedMessage : BaseMessage
{
    NotInterestedMessage() : BaseMessage(1, NOT_INTERESTED) {}
};

struct HaveMessage : BaseMessage
{
    HaveMessage(uint32_t index) : BaseMessage(5, HAVE), index(index) {}

    uint32_t index;
};

struct BitfieldMessage : BaseMessage
{
    BitfieldMessage(uint32_t length) : BaseMessage(length, BITFIELD) {}

    uint8_t data[];
};

struct RequestMessage : BaseMessage
{
    RequestMessage(uint32_t index_, uint32_t offset_, uint32_t block_length_) : BaseMessage(13, REQUEST),
                                                                               index(htonl(index_)),
                                                                               offset(htonl(offset_)),
                                                                               block_length(htonl(block_length_))
                                                                               {}
    uint32_t index;
    uint32_t offset;
    uint32_t block_length;
};

struct PieceMessage : BaseMessage
{
    PieceMessage(uint32_t length, uint32_t index_, uint32_t offset_) : BaseMessage(length, PIECE),
                                                                       index(htonl(uint32_t(index_))),
                                                                       offset(htonl(uint32_t(offset_)))
                                                                       {}
    uint32_t index;
    uint32_t offset;

    uint8_t data[];
};

struct CancelMessage : BaseMessage
{
    CancelMessage(uint32_t index_, uint32_t offset_, uint32_t block_length_) : BaseMessage(13, CANCEL),
                                                                               index(htonl(uint32_t(index_))),
                                                                               offset(htonl(uint32_t(offset_))),
                                                                               block_length(htonl(uint32_t(block_length_)))
                                                                               {}
    uint32_t index;
    uint32_t offset;
    uint32_t block_length;
};

struct PortMessage : BaseMessage
{
    PortMessage(uint16_t port_) : BaseMessage(3, PORT), port(htons(port_)) {}

    uint16_t port;
};


#pragma pack(pop)


