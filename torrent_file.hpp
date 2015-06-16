#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "bedecoder/be_parser.hpp"

//https://wiki.theory.org/BitTorrentSpecification
//http://bittorrent.org/beps/bep_0003.html
//http://jonas.nitro.dk/bittorrent/bittorrent-rfc.html

class TorrentFile
{
public:
    TorrentFile(const std::string & path);

private:
    void process_file(const std::string & path);

    void fill_announce(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);
    void fill_announce_list(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);
    void fill_creation_date(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);
    void fill_comment(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);
    void fill_created_by(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);
    void fill_encoding(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);

    void fill_info(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token) {}

private:
    const std::unordered_map<std::string, decltype(&TorrentFile::fill_announce_list)> m_fillers {
                                                                                        {"announce", &TorrentFile::fill_announce},
                                                                                        {"announce-list", &TorrentFile::fill_announce_list},
                                                                                        {"creation date", &TorrentFile::fill_creation_date},
                                                                                        {"comment", &TorrentFile::fill_comment},
                                                                                        {"created by", &TorrentFile::fill_created_by},
                                                                                        {"encoding", &TorrentFile::fill_encoding}
                                                                                      };

    std::string              m_announce {""};
    std::vector<std::string> m_announce_list;
    uint64_t                 m_creation_date {0};
    std::string              m_comment {""};
    std::string              m_created_by {""};
    std::string              m_encoding {""};

};


