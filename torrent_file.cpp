#include "torrent_file.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

static std::string get_data_from_file(const std::string & path_to_file)
{
    std::ifstream in_file(path_to_file.c_str());
    if (!in_file)
    {
        std::cerr << "unable to open: " << path_to_file;
        exit(1);
    }

    std::stringstream in;
    in << in_file.rdbuf();
    in_file.close();

    std::string source = in.str();
    return std::move(source);
}



TorrentFile::TorrentFile(const std::string & path)
{
    process_file(path);
}

void TorrentFile::process_file(const std::string & path)
{
    std::string source = get_data_from_file(path);
    auto tokens = BeParser::parse(source);

    uint64_t current_token = 0;
    auto filler = m_fillers.end();
    uint64_t type = 0;
    const uint64_t TOKENS_SIZE = tokens.size();
    while (current_token < TOKENS_SIZE)
    {
        type = tokens[current_token].get_type();
        if (type == BeToken::INT || type == BeToken::STR)
        {
            filler = m_fillers.find(tokens[current_token].get(source));
            if (filler != m_fillers.end())
            {
                (this->*(filler->second))(tokens, source, current_token);
                continue;
            }
        }

        ++current_token;
    }

}

void TorrentFile::fill_announce(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    ++current_token; // skip "announce"
    uint64_t type = tokens[current_token].get_type();
    if (type == BeToken::STR)
        m_announce = tokens[current_token].get(source);

    ++current_token;

// XXX debug
    std::cerr << "announce: " << m_announce << std::endl;
}

void TorrentFile::fill_announce_list(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    ++current_token; // skip "announce-list"
    uint64_t type = tokens[current_token].get_type();
    if (type == BeToken::LIST_START)
    {
        ++current_token;; // skip LIST_START
        type = tokens[current_token].get_type();
        const uint64_t TOKENS_SIZE = tokens.size();
        while (current_token < TOKENS_SIZE)
        {
            type = tokens[current_token].get_type();
            if (type == BeToken::LIST_END)
                break;

            if (type == BeToken::INT || type == BeToken::STR)
                m_announce_list.push_back(tokens[current_token].get(source));

            ++current_token;
        }
    }

    ++current_token;
// XXX debug
    std::cerr << "announce list : " << std::endl;
    for (const auto & i : m_announce_list)
        std::cerr << '\t' << i << std::endl;
}

void TorrentFile::fill_creation_date(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    ++current_token; // skip "creation date"
    uint64_t type = tokens[current_token].get_type();
    if (type == BeToken::INT)
    {
        m_creation_date = std::stoll(tokens[current_token].get(source), nullptr, 10);
    }

    ++current_token;
// XXX debug
    std::cerr << "creation date: " << m_creation_date << std::endl;
}

void TorrentFile::fill_comment(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    ++current_token; // skip "comment"
    uint64_t type = tokens[current_token].get_type();
    if (type == BeToken::STR)
    {
        m_comment = tokens[current_token].get(source);
    }

    ++current_token;
// XXX debug
    std::cerr << "comment: " << m_comment << std::endl;
}

void TorrentFile::fill_created_by(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    ++current_token; // skip "created by"
    uint64_t type = tokens[current_token].get_type();
    if (type == BeToken::STR)
    {
        m_created_by = tokens[current_token].get(source);
    }

    ++current_token;
// XXX debug
    std::cerr << "created by: " << m_created_by << std::endl;
}

void TorrentFile::fill_encoding(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    ++current_token; // skip "encoding"
    uint64_t type = tokens[current_token].get_type();
    if (type == BeToken::STR)
    {
        m_encoding = tokens[current_token].get(source);
    }

    ++current_token;
// XXX debug
    std::cerr << "encoding: " << m_encoding << std::endl;
}


