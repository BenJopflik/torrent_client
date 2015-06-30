#pragma once
#include <set>
#include <memory>
#include <cstdint>


struct Block
{
public:
    enum Error
    {
        OK = 0,
        SELF_IN_RIGHT,
        RIGHT_IN_SELF
    };

public:
    Block(uint64_t offset, uint64_t size);
    ~Block();

    bool operator < (const Block & right) const;
    void modify_on_intersection(const Block & right);
    bool full() const;

public:
    uint64_t offset {0};
    uint64_t size {0};

};

class Piece
{
public:
    Piece();
    ~Piece();

    void init(int64_t id, uint64_t size);
    void add(int64_t id, const uint8_t * source, uint64_t size, uint64_t offset);
    bool full() const;
    void clear();
    std::unique_ptr<uint8_t []> get();

private:
    std::unique_ptr<uint8_t []> m_data;
    int64_t m_id {-1};
    uint64_t m_full_piece_size {0};
    std::set<Block> m_blocks;
};
