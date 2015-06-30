#include "piece.hpp"
#include <cstring>

Block::Block(uint64_t offset, uint64_t size) : offset(offset), size(size)
{

}

Block::~Block()
{

}

bool Block::operator < (const Block & right) const
{
    return offset < right.offset;
}

void Block::modify_on_intersection(const Block & right)
{
    uint64_t self_end = offset + size;
    uint64_t right_end = right.offset + right.size;

    if (self_end <= right.offset || offset  >= right_end)
        return;

    if (offset < right.offset)
    {
        if (right_end < self_end)
            throw Block::Error(RIGHT_IN_SELF);

        size -= self_end - right.offset;
        return;
    }

    if (self_end < right_end)
        throw Block::Error(SELF_IN_RIGHT);

    size   -= right_end - offset;
    offset += right_end - offset;
}

Piece::Piece()
{

}

Piece::~Piece()
{

}

void Piece::init(int64_t id, uint64_t size)
{
    m_id = id;
    m_data.reset(new uint8_t [size]);
    m_blocks.clear();
    m_full_piece_size = size;
}

void Piece::add(int64_t id, const uint8_t * source, uint64_t size, uint64_t offset)
{
    if (!m_data)
        throw std::runtime_error("uninitialized piece buffer. call init before add");

    if (m_id == -1)
        m_id = id;
    else if (m_id != id)
        throw std::runtime_error("invalid piece id");

    if (m_blocks.empty())
    {
        m_blocks.insert(Block(offset, size));
        memcpy(m_data.get() + offset, source, size);
        return;
    }

    Block new_block(offset, size);
    auto next_block = m_blocks.upper_bound(new_block);
    auto prev_block = next_block;

    if (prev_block == m_blocks.begin())
        prev_block = m_blocks.end();
    else
        --prev_block;

    bool prev_valid = !(prev_block == m_blocks.end());
    bool next_valid = !(next_block == m_blocks.end());

    if (prev_valid)
    {
        try
        {
            new_block.modify_on_intersection(*prev_block);
        }
        catch (const Block::Error & e)
        {
            if (e == Block::Error::SELF_IN_RIGHT)
                return;
            throw;
        }
    }

    if (next_valid)
    {
        try
        {
            new_block.modify_on_intersection(*next_block);
        }
        catch (const Block::Error & e)
        {
            if (e == Block::Error::SELF_IN_RIGHT)
                return;
            throw;
        }
    }

    if (new_block.size)
        memcpy(m_data.get() + new_block.offset, source + new_block.offset - offset, new_block.size);
    else
        return;

    if (prev_valid)
    {
        if (prev_block->offset + prev_block->size == new_block.offset)
        {
            auto tmp_block = Block(prev_block->offset, prev_block->size + new_block.size);
            m_blocks.erase(prev_block);
            prev_block = m_blocks.insert(tmp_block).first;
        }
        else
            prev_block = m_blocks.insert(new_block).first;
    }
    else if (new_block.offset + new_block.size == next_block->offset)
    {
        auto tmp_block = Block(new_block.offset, next_block->size + new_block.size);
        auto tmp_iter = m_blocks.insert(tmp_block).first;
        m_blocks.erase(next_block);
        next_block = tmp_iter;
    }
    else
    {
        next_block = m_blocks.insert(new_block).first;
    }

    if (prev_valid && next_valid)
    {
        if (prev_block->offset + prev_block->size == next_block->offset)
        {
            auto tmp_block = Block(prev_block->offset, prev_block->size + next_block->size);
            m_blocks.erase(prev_block);
            m_blocks.insert(tmp_block);
            m_blocks.erase(next_block);
        }
    }
}

bool Piece::full() const
{
    if (m_blocks.empty())
        return false;

    return m_blocks.begin()->size == m_full_piece_size;
}

std::unique_ptr<uint8_t []> Piece::get()
{
    std::unique_ptr<uint8_t []> output;
    output.swap(m_data);
    m_id = -1;
    m_full_piece_size = 0;
    m_blocks.clear();
    return std::move(output);
}

void Piece::clear()
{
    m_blocks.clear();
    m_data.reset();
    m_id = -1;
    m_full_piece_size = 0;
}

