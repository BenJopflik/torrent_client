#pragma once

#include <vector>
#include <string>
#include <list>
#include "thread/spinlock.hpp"

class TorrentFile;

class PieceList
{
public:
    PieceList(const TorrentFile & tf);
    ~PieceList();

    uint64_t get_piece(int64_t & piece_index); // return piece size and piece_index
    void     return_piece(uint64_t piece);
    uint64_t number_of_pieces() const {return MAX_PIECE_NUM - 1;}

private:
    const uint64_t MAX_PIECE_NUM;
    const uint64_t LAST_PIECE_SIZE;
    const uint64_t PIECE_SIZE;

    uint64_t m_next_piece {0};
    std::list<int64_t> m_returned_pieces;
    SpinlockYield m_lock;

};
