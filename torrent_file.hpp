#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "bedecoder/be_parser.hpp"
#include "common/hashable.hpp"
#include "token_list_processor.hpp"

//https://wiki.theory.org/BitTorrentSpecification
//http://bittorrent.org/beps/bep_0003.html
//http://jonas.nitro.dk/bittorrent/bittorrent-rfc.html


// XXX store all pieces in corresponding File?
struct File
{
    struct OffsetInPiece
    {
        uint64_t piece_index {0};
        uint64_t offset {0};
    };

    std::string path {""};
    std::string md5_sum {""};
    uint64_t size {0};


    OffsetInPiece first_piece;
    OffsetInPiece last_piece;
};

class TorrentFile : public Hashable<TorrentFile>, public TokenListProcessor<TorrentFile>
{
    enum
    {
        SHA1_PIECE_LENGTH = 20,

    };

public:
    TorrentFile(const std::string & path);
    void print() const;

    uint64_t hash() const;
    bool operator == (const TorrentFile &) const;

    const std::vector<std::string> & announce_list() const {return m_announce_list;}
    const std::vector<std::string> & pieces()        const {return m_pieces;}
    const std::vector<File>        & files()         const {return m_files;}

    const std::string & path()       const {return m_path;}
    const std::string & announce()   const {return m_announce;}
    const std::string & comment()    const {return m_comment;}
    const std::string & created_by() const {return m_created_by;}
    const std::string & encoding()   const {return m_encoding;}
    const std::string & dir_name()   const {return m_dir_name;}
    const std::string & info_hash()  const {return m_info_sha1_string;}

    const std::string & info_hash_bin() const {return m_info_sha1;}

    const uint64_t & creation_date()   const {return m_creation_date;}
    const uint64_t & piece_size()      const {return m_piece_size;}
    const uint64_t & last_piece_size() const {return m_last_piece_size;}
    const uint64_t & full_size()       const {return m_full_size;}

private:
    void process_file(const std::string & path);

    MEMBER_DECLARE(fill_announce)
    MEMBER_DECLARE(fill_announce_list)
    MEMBER_DECLARE(fill_creation_date)
    MEMBER_DECLARE(fill_comment)
    MEMBER_DECLARE(fill_created_by)
    MEMBER_DECLARE(fill_encoding)
    MEMBER_DECLARE(fill_info)
    MEMBER_DECLARE(fill_files)
    MEMBER_DECLARE(fill_pieces)

    void fill_piece_offsets();
    void calculate_info_sha1(const std::vector<BeToken> & tokens, const std::string & source);
    // XXX debug
    std::string get_http_url() const;

private:
    std::vector<std::string> m_announce_list;
    std::vector<std::string> m_pieces; // 20-byte sha1 for every piece
    std::vector<File>        m_files;

    std::string              m_path       {""};
    std::string              m_announce   {""};
    std::string              m_comment    {""};
    std::string              m_created_by {""};
    std::string              m_encoding   {""};
    std::string              m_dir_name   {""};
    std::string              m_info_sha1  {""};
    std::string              m_info_sha1_string  {""};

    uint64_t                 m_creation_date   {0};
    uint64_t                 m_piece_size      {0};
    uint64_t                 m_last_piece_size {0};
    uint64_t                 m_full_size       {0};
    uint64_t                 m_private         {0};
};
#undef MEMBER_DECLARE
