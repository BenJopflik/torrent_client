#include "be_token.hpp"
#include <iostream>
#include <iomanip>

BeToken::BeToken()
{

}

BeToken::~BeToken()
{

}

BeToken::operator bool () const
{
    return m_type != UNKNOWN;
}

bool BeToken::operator < (const BeToken & right) const
{
    switch (m_type)
    {
        case Type::INTEGER:
            return m_int < right.m_int;

        case Type::STRING:
            return m_string < right.m_string;

        default:
            throw std::runtime_error("using map, list or unknown as a key");
    }
}


int64_t BeToken::to_int() const
{
    if (m_type == INTEGER)
        return m_int;

    throw std::runtime_error("BeToken is not an int");
}

BeToken::operator int64_t () const
{
    return to_int();
}

std::string BeToken::to_string() const
{
    if (m_type == STRING)
        return m_string;

    throw std::runtime_error("BeToken is not an string");
}

BeToken::operator std::string () const
{
    return to_string();
}

// XXX Debug
void BeToken::print(uint64_t tabs) const
{
    switch (m_type)
    {
        case INTEGER:
            std::cerr << std::setfill('\t') << std::setw(tabs) << m_int;
            break;

        case STRING:
            if (m_string.size() < 100)
                std::cerr << std::setfill('\t') << std::setw(tabs) << m_string;
            break;

        case LIST:
            for (const auto & i : m_list)
            {
                i.print(tabs + 1);
                std::cerr << std::endl;
            }
            break;

        case DICTIONARY:
            for (const auto & i : m_dictionary)
            {
                i.first.print(tabs);
                i.second.print(tabs);
                std::cerr << std::endl;
            }
            break;

        default:
            break;
    }

    std::cerr << std::endl;
}

void BeToken::insert(const BeToken & key, const BeToken & value)
{
    if (m_type != DICTIONARY)
        throw std::runtime_error("token is not a dictionary");

    m_dictionary.insert(std::make_pair(key, value));
}

void BeToken::push_back(const BeToken & value)
{
    if (m_type != LIST)
        throw std::runtime_error("token is not a list");

    m_list.push_back(value);
}



