#include "be_parser.hpp"
#include <cassert>

BeToken BeParser::parse(const std::string & data)
{
    const char * tmp = data.c_str();
    return parse(tmp, tmp + data.size());
}

BeToken BeParser::process_integer(const char *& data, const char * END)
{
    BeToken ret;
    ret.m_type = BeToken::Type::INTEGER;

    ++data; // 'i'
    char * delim = nullptr;
    auto value = strtol(data, &delim, 10);
    assert(*delim == 'e' && "invalid integer format");
    ret.m_int = value;
    data = delim;
    ++data; // 'e'

    return std::move(ret);
}

BeToken BeParser::process_string(const char *& data, const char * END)
{
    BeToken ret;
    ret.m_type = BeToken::Type::STRING;

    char * delim = nullptr;
    auto len = strtol(data, &delim, 10);
    assert(*delim == ':' && "invalid string format");
    data = ++delim; // ':'
    std::string str(data, len);
    ret.m_string = str;
    data += len;

    return std::move(ret);
}

BeToken BeParser::process_list(const char *& data, const char * END)
{
    BeToken ret;
    ret.m_type = BeToken::Type::LIST;

    ++data; // 'l'
    while (data < END && *data != 'e')
    {
        ret.push_back(parse(data, END));
    }
    ++data; // 'e'
    return std::move(ret);
}

BeToken BeParser::process_dictionary(const char *& data, const char * END)
{
    BeToken ret;
    ret.m_type = BeToken::Type::DICTIONARY;

    ++data; // 'd'
    while (data < END && *data != 'e')
    {
        auto key = parse(data, END);
        if (data >= END)
            throw std::runtime_error("invalid BeToken::dictionary format");
        auto value = parse(data, END);
        ret.insert(key, value);
    }
    ++data; // 'e'
    return std::move(ret);
}

BeToken BeParser::parse(const char *& data, const char * END)
{
    if (data < END)
    {
        switch (*data)
        {
            case 'i':
                return process_integer(data, END);

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
                return process_string(data, END);

            case 'l':
                return process_list(data, END);

            case 'd':
                return process_dictionary(data, END);

            default:
                throw std::runtime_error("invalid be data");
        }
    }
    return BeToken();
}

