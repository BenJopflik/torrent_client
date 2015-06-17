#include "torrent_file.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <cassert>

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

    fill_piece_offsets();
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
        type = tokens[current_token].type();
        if (type == BeToken::INT || type == BeToken::STR)
        {
            filler = m_fillers.find(tokens[current_token].str(source));
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
    uint64_t type = tokens[current_token].type();
    if (type == BeToken::STR)
        m_announce = tokens[current_token].str(source);

    ++current_token;
}

static std::vector<std::string> read_lists(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    std::vector<std::string> output;
    uint64_t type = tokens[current_token].type();
    const uint64_t TOKENS_SIZE = tokens.size();
    uint64_t list_depth = 0;

    type = tokens[current_token].type();
    if (type == BeToken::LIST_START)
        ++list_depth;

    ++current_token;

    while (list_depth && current_token < TOKENS_SIZE)
    {
        type = tokens[current_token].type();
        if (type == BeToken::LIST_START)
            ++list_depth;
        else if (type == BeToken::LIST_END)
            --list_depth;
        else if (type == BeToken::STR || type == BeToken::INT)
            output.push_back(tokens[current_token].str(source));
        else
            break;

        ++current_token;
    }

    return std::move(output);
}

void TorrentFile::fill_announce_list(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    ++current_token; // skip "announce-list"
    m_announce_list.clear();
    m_announce_list = read_lists(tokens, source, current_token);
}

void TorrentFile::fill_creation_date(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    ++current_token; // skip "creation date"
    uint64_t type = tokens[current_token].type();
    if (type == BeToken::INT)
        m_creation_date = std::stoll(tokens[current_token].str(source), nullptr, 10);

    ++current_token;
}

void TorrentFile::fill_comment(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    ++current_token; // skip "comment"
    uint64_t type = tokens[current_token].type();
    if (type == BeToken::STR)
    {
        m_comment = tokens[current_token].str(source);
    }

    ++current_token;
}

void TorrentFile::fill_created_by(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    ++current_token; // skip "created by"
    uint64_t type = tokens[current_token].type();
    if (type == BeToken::STR)
    {
        m_created_by = tokens[current_token].str(source);
    }

    ++current_token;
}

void TorrentFile::fill_encoding(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    ++current_token; // skip "encoding"
    uint64_t type = tokens[current_token].type();
    if (type == BeToken::STR)
    {
        m_encoding = tokens[current_token].str(source);
    }

    ++current_token;
}

void TorrentFile::fill_info(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    ++current_token; // skip "info"

    const uint64_t TOKENS_SIZE = tokens.size();

    uint64_t type = tokens[current_token].type();
    ++current_token;

    if (type != BeToken::DICT_START)
    {
        current_token = TOKENS_SIZE;
        return;
    }

    std::string key   = "";
    std::string value = "";
    uint64_t value_type = 0;

    m_files.push_back(File());

    while (current_token < TOKENS_SIZE)
    {
        type = tokens[current_token].type();
        if (type == BeToken::STR)
        {
            key   = tokens[current_token].str(source);
            ++current_token;
            value_type = tokens[current_token].type();

            if (key == "piece length" && value_type == BeToken::INT)
                m_piece_size = std::stoll(tokens[current_token].str(source), nullptr, 10);
            else if (key == "private" && value_type == BeToken::INT)
                m_private = std::stoll(tokens[current_token].str(source), nullptr, 10);
            else if (key == "pieces" && value_type == BeToken::STR)
                fill_pieces(tokens, source, current_token);

            else if (key == "name" && value_type == BeToken::STR)
                m_dir_name = tokens[current_token].str(source);
            else if (key == "md5sum" && value_type == BeToken::STR)
                m_files[0].md5_sum = tokens[current_token].str(source);
            else if (key == "length" && value_type == BeToken::INT)
                m_files[0].size = std::stoll(tokens[current_token].str(source), nullptr, 10);
            else if (key == "files" && value_type == BeToken::LIST_START)
                fill_files(tokens, source, current_token);
        }
        else if (type == BeToken::DICT_END)
        {
            ++current_token;
            break;
        }
        else
        {
            current_token = TOKENS_SIZE;
            return;
        }

        ++current_token;
    }

    if (m_files.size() == 1)
    {
        m_files[0].path = m_dir_name;
    }
}

void TorrentFile::fill_pieces(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    auto & pieces = tokens[current_token];
    if (pieces.length() % SHA1_PIECE_LENGTH)
        throw std::runtime_error("invalid pieces length");

    const uint64_t NUMBER_OF_PIECES = pieces.length() / SHA1_PIECE_LENGTH;
    m_pieces.reserve(NUMBER_OF_PIECES);
    for (uint64_t i = 0; i < NUMBER_OF_PIECES; ++i)
        m_pieces.push_back(pieces.substr(source, i * SHA1_PIECE_LENGTH, SHA1_PIECE_LENGTH));

    ++current_token;
}

void TorrentFile::fill_files(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    uint64_t type = 0;
    const uint64_t TOKENS_SIZE = tokens.size();
    ++current_token; // skip LIST_START

    std::string key = "";
    std::string value = "";
    uint64_t value_type = 0;
    bool first_iter = true;

    while (current_token < TOKENS_SIZE)
    {
        type = tokens[current_token].type();

        if (type != BeToken::DICT_START)
            break;
        else if (!first_iter)
            m_files.push_back(File());

        first_iter = false;

        ++current_token; // skip DICT_START

        while (current_token < TOKENS_SIZE)
        {
            type = tokens[current_token].type();
            if (type == BeToken::LIST_END)
            {
                break;
            }
            else if (type == BeToken::STR)
            {
                key   = tokens[current_token].str(source);
                ++current_token;
                value_type = tokens[current_token].type();

                if (key == "path" && value_type == BeToken::LIST_START)
                {
                    auto path = read_lists(tokens, source, current_token);
                    for (const auto & i : path)
                    {
                        m_files.back().path += '/';
                        m_files.back().path += i;
                    }
                }
                else if (key == "md5sum" && value_type == BeToken::STR)
                    m_files.back().md5_sum = tokens[current_token].str(source);
                else if (key == "length" && value_type == BeToken::INT)
                    m_files.back().size = std::stoll(tokens[current_token].str(source), nullptr, 10);
            }
            else
            {
                break;
            }
            ++current_token;
        }
    }
}

void TorrentFile::fill_piece_offsets()
{
    uint64_t offset = 0;
    uint64_t piece_index = 0;
    const uint64_t NUMBER_OF_FILES = m_files.size();
    for (uint64_t i = 0; i < NUMBER_OF_FILES; ++i)
    {
        m_full_size += m_files[i].size;
        m_files[i].first_piece.piece_index = piece_index;
        m_files[i].first_piece.offset = offset;

        offset += m_files[i].size;
        piece_index += offset / m_piece_size;

        offset %= m_piece_size;

        m_files[i].last_piece.piece_index = piece_index;
        m_files[i].last_piece.offset = !offset ? m_piece_size : offset;

        if (!offset)
            ++piece_index;
    }

    assert(!m_pieces.empty());
    m_last_piece_size = m_full_size % m_piece_size;
}

void TorrentFile::print() const
{
    std::cerr << "announce: " << m_announce << std::endl;
    std::cerr << "announce list: " << std::endl;
    for (const auto & i : m_announce_list)
        std::cerr << '\t' << i << std::endl;

    std::cerr << "comment: " << m_comment << std::endl;
    std::cerr << "created by: " << m_created_by << std::endl;
    std::cerr << "creation date: " << m_creation_date << std::endl;
    std::cerr << "piece size: " << m_piece_size << std::endl;
    std::cerr << "last piece size: " << m_last_piece_size << std::endl;
    std::cerr << "number of pieces: " << m_pieces.size() << std::endl;
    std::cerr << "encoding: " << m_encoding << std::endl;
    std::cerr << "private: " << m_private << std::endl;
    std::cerr << "full size: " << m_full_size << std::endl;
    std::cerr << "dir name: " << m_dir_name << std::endl;

    std::cerr << "files: " << std::endl;

    for (const auto & i : m_files)
    {
        std::cerr << '\t' << "path: " << i.path << std::endl;
        std::cerr << '\t' << "size: " << i.size << std::endl;
        std::cerr << '\t' << "md5_sum: " << i.md5_sum << std::endl;
        std::cerr << '\t' << "first_piece.offset: " << i.first_piece.offset << std::endl;
        std::cerr << '\t' << "first_piece.piece_index: " << i.first_piece.piece_index << std::endl;
        std::cerr << '\t' << "last_piece.offset: " << i.last_piece.offset << std::endl;
        std::cerr << '\t' << "last_piece.piece_index: " << i.last_piece.piece_index << std::endl;
        std::cerr << std::endl;
    }

    std::cerr << std::endl;
}
