#include "be_token.hpp"

BeToken::BeToken()
{

}

BeToken::BeToken(uint64_t type) : m_type(type)
{

}

BeToken::~BeToken()
{

}

std::string BeToken::str(const std::string & source) const
{
    return m_data.str(source);
}

std::string BeToken::substr(const std::string & source, uint64_t offset, uint64_t length) const
{
    return m_data.substr(source, offset, length);
}

uint64_t BeToken::type() const
{
    return m_type;
}

uint64_t BeToken::length() const
{
    return m_data.length();
}

std::string BeToken::type_to_string() const
{
    switch (m_type)
    {
        case UNKNOWN:
            return "UNKNOWN";

        case INT:
            return "INT";

        case STR:
            return "STR";

        case LIST_START:
            return "LIST_START";

        case LIST_END:
            return "LIST_END";

        case DICT_START:
            return "DICT_START";

        case DICT_END:
            return "DICT_END";
    }
    return "";
}

