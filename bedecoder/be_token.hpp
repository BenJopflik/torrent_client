#pragma once
#include <string>
#include <map>
#include <vector>
#include <cstdint>

class BeParser;

class BeToken
{
friend BeParser;

public:
    enum Type
    {
        UNKNOWN = 0,
        INTEGER,
        STRING,
        LIST,
        DICTIONARY,

    };

public:
    ~BeToken();

    operator bool () const;
    bool operator < (const BeToken & right) const;

    template <class T>
    const BeToken & operator [](const T & value);

    int64_t to_int() const;
    operator int64_t () const;

    std::string to_string() const;
    operator std::string () const;

    void print(uint64_t tabs = 0) const;

private:
    BeToken ();

    void insert(const BeToken & key, const BeToken & value);
    void push_back(const BeToken & value);

private:
    uint64_t m_type {UNKNOWN};
// TODO
//    union
//    {
        int64_t m_int;
        std::string m_string;
        std::vector<BeToken> m_list;
        std::map<BeToken, BeToken> m_dictionary;
//    };

};

template <class T>
const BeToken & BeToken::operator [](const T & value)
{
    if (m_type == DICTIONARY)
    {
        auto iter = m_dictionary.find(value);
        if (iter == m_dictionary.end())
            return BeToken();
        return m_dictionary[value];
    }

    if (m_type == LIST)
    {
        int index = (int)value;
        if (index > m_list.size())
            return BeToken();
        return m_list[index];
    }

    throw std::runtime_error("invalid use of BeToken::operator[]");
}

