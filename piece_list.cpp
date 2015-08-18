#include "piece_list.hpp"
#include "torrent_file.hpp"
#include <mutex>


PieceList::PieceList(const TorrentFile & tf) : MAX_PIECE_NUM(tf.pieces().size() - 1),
                                               LAST_PIECE_SIZE(tf.last_piece_size()),
                                               PIECE_SIZE(tf.piece_size())
{
}

PieceList::~PieceList()
{

}

uint64_t PieceList::get_piece(int64_t & piece_index)
{
    std::lock_guard<decltype(m_lock)> lock(m_lock);
    piece_index = -1;
    if (!m_returned_pieces.empty())
    {
        piece_index = m_returned_pieces.front();
        m_returned_pieces.pop_front();
        return (piece_index == MAX_PIECE_NUM) ? LAST_PIECE_SIZE : PIECE_SIZE;
    }

    if (m_next_piece <= MAX_PIECE_NUM)
    {
        piece_index = m_next_piece++;
        return (piece_index == MAX_PIECE_NUM) ? LAST_PIECE_SIZE : PIECE_SIZE;
    }

    return 0;
}

void PieceList::return_piece(uint64_t piece)
{
    std::lock_guard<decltype(m_lock)> lock(m_lock);
    m_returned_pieces.push_back(piece);
}
