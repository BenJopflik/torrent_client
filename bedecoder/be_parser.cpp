#include "be_parser.hpp"
#include <stdexcept>

std::vector<BeToken> BeParser::parse(const StringWrapper & source)
{
    const uint64_t SOURCE_SIZE = source.size();
    const char * SOURCE = source.data();

    uint64_t offset = 0;
    std::vector<BeToken> ret;
    ret.reserve(100);
    while (offset < SOURCE_SIZE)
    {
        auto res = BeParser::parse(SOURCE, offset, SOURCE_SIZE);
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

    if (*end != 'e')
        throw std::runtime_error("invalid integer format");

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

    if (*delim != ':')
        throw std::runtime_error("invalid string format");

    ++delim; // ':'
    offset += delim - data;

    ret[0].m_data.set(offset, offset + len);
    offset += len;

    return std::move(ret);
}

std::vector<BeToken> BeParser::process_struct(const char * BEGIN, uint64_t & offset, uint64_t length, bool is_list)
{
    std::vector<BeToken> ret;
    ret.reserve(100); // XXX remove
    ret.push_back(BeToken(is_list ? BeToken::LIST_START : BeToken::DICT_START));
    ret.back().m_data.set(offset, 0);

    ++offset; // struct start ('l'/ 'd')

    while (offset < length && *(BEGIN + offset) != 'e')
    {
        auto res = parse(BEGIN, offset, length);
        std::copy(res.begin(), res.end(), back_inserter(ret));
    }
    ret.push_back(BeToken(is_list ? BeToken::LIST_END : BeToken::DICT_END));
    ret.back().m_data.set(offset, 0);
    ++offset; // 'e'
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
                return process_struct(BEGIN, offset, length, true);

            case 'd':
                return process_struct(BEGIN, offset, length);

            default:
                throw std::runtime_error("invalid be data");
        }
    }
    return std::vector<BeToken>();
}

