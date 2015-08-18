#pragma once

#include "common/bitset.hpp"
#include <vector>

class File;
class TorrentFile;

class Files
{
public:
    Files(const TorrentFile & tf);
    ~Files();

    void write_piece(uint64_t id, const uint8_t * source, uint64_t size);

private:
    std::vector<File> m_files;

};

