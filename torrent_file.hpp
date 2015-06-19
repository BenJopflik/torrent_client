#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "bedecoder/be_parser.hpp"
#include "../jasl/common/hashable.hpp"

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

class TorrentFile : public Hashable<TorrentFile>
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
    const std::string & info_hash()  const {return m_info_sha1;}

    const uint64_t & creation_date()   const {return m_creation_date;}
    const uint64_t & piece_size()      const {return m_piece_size;}
    const uint64_t & last_piece_size() const {return m_last_piece_size;}
    const uint64_t & full_size()       const {return m_full_size;}

private:
    void process_file(const std::string & path);

    void fill_announce(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);
    void fill_announce_list(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);
    void fill_creation_date(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);
    void fill_comment(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);
    void fill_created_by(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);
    void fill_encoding(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);

    void fill_info(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);
    void fill_files(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);
    void fill_pieces(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);

    void fill_piece_offsets();
    void calculate_info_sha1(const std::vector<BeToken> & tokens, const std::string & source);
    // XXX debug
    std::string get_http_url() const;

private:
    const std::unordered_map<std::string, decltype(&TorrentFile::fill_announce_list)> m_fillers {
                                                                                        {"announce", &TorrentFile::fill_announce},
                                                                                        {"announce-list", &TorrentFile::fill_announce_list},
                                                                                        {"creation date", &TorrentFile::fill_creation_date},
                                                                                        {"comment", &TorrentFile::fill_comment},
                                                                                        {"created by", &TorrentFile::fill_created_by},
                                                                                        {"encoding", &TorrentFile::fill_encoding},
                                                                                        {"info", &TorrentFile::fill_info}
                                                                                      };

    std::vector<std::string> m_announce_list;
    std::vector<std::string> m_pieces; // 20-byte sha1 for every piece
    std::vector<File>        m_files;

    std::string              m_path {""};
    std::string              m_announce {""};
    std::string              m_comment {""};
    std::string              m_created_by {""};
    std::string              m_encoding {""};
    std::string              m_dir_name {""};
    std::string              m_info_sha1 {""};

    uint64_t                 m_creation_date {0};
    uint64_t                 m_piece_size {0};
    uint64_t                 m_last_piece_size {0};
    uint64_t                 m_full_size {0};
    uint64_t                 m_private {0};
};


