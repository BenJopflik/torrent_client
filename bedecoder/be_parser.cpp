#include "be_parser.hpp"
#include <cassert>
#include <stdexcept>

std::vector<BeToken> BeParser::parse(const std::string & data)
{
    const char * tmp = data.c_str();
    uint64_t offset = 0;
    uint64_t size = data.size();
    std::vector<BeToken> ret;
    ret.reserve(100);
    while (offset < size)
    {
        auto res = parse(tmp, offset, data.size());
        std::copy(res.begin(), res.end(), back_inserter(ret));
    }
    ret.shrink_to_fit();
    return std::move(ret);
}

std::vector<BeToken> BeParser::process_integer(const char * BEGIN, uint64_t & offset, uint64_t length)
{
    std::vector<BeToken> ret(1);
    ret[0].m_type = BeToken::INT;

    ++offset; // 'i'
    const char * data = BEGIN + offset;
    char * end = nullptr;
    strtol(data, &end, 10);
    assert(*end == 'e' && "invalid integer format");
    ret[0].m_data.set(offset, end - BEGIN);
    offset = end - BEGIN;
    ++offset; // 'e'

    return std::move(ret);
}

std::vector<BeToken> BeParser::process_string(const char * BEGIN, uint64_t & offset, uint64_t length)
{
    std::vector<BeToken> ret(1);
    ret[0].m_type = BeToken::STR;

    const char * data = BEGIN + offset;
    char * delim = nullptr;
    auto len = strtol(data, &delim, 10);
    assert(*delim == ':' && "invalid string format");
    ++delim; // ':'
    offset += delim - data;

    ret[0].m_data.set(offset, offset + len);
    offset += len;

    return std::move(ret);
}

std::vector<BeToken> BeParser::process_list(const char * BEGIN, uint64_t & offset, uint64_t length)
{
    std::vector<BeToken> ret;
    ret.reserve(100); // XXX remove
    ret.push_back(BeToken(BeToken::LIST_START));

    ++offset; // 'l'

    while (offset < length && *(BEGIN + offset) != 'e')
    {
        auto res = parse(BEGIN, offset, length);
        std::copy(res.begin(), res.end(), back_inserter(ret));
    }
    ++offset; // 'e'
    ret.push_back(BeToken(BeToken::LIST_END));
    return std::move(ret);
}

std::vector<BeToken> BeParser::process_dictionary(const char * BEGIN, uint64_t & offset, uint64_t length)
{
    std::vector<BeToken> ret;
    ret.reserve(100); // XXX remove
    ret.push_back(BeToken(BeToken::DICT_START));

    ++offset; // 'l'

    while (offset < length && *(BEGIN + offset) != 'e')
    {
        auto res = parse(BEGIN, offset, length);
        std::copy(res.begin(), res.end(), back_inserter(ret));
    }
    ++offset; // 'e'
    ret.push_back(BeToken(BeToken::DICT_END));
    return std::move(ret);
}

std::vector<BeToken> BeParser::parse(const char * BEGIN, uint64_t & offset, uint64_t length)
{
    if (offset < length)
    {
        const char * data = (BEGIN + offset);
        switch (*data)
        {
            case 'i':
                return process_integer(BEGIN, offset, length);

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                return process_string(BEGIN, offset, length);

            case 'l':
                return process_list(BEGIN, offset, length);

            case 'd':
                return process_dictionary(BEGIN, offset, length);

            default:
                throw std::runtime_error("invalid be data");
        }
    }
    return std::vector<BeToken>();
}

