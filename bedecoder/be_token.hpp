#pragma once
#include "../jasl/common/sub_string.hpp" // XXX tmp

class BeParser;

class BeToken
{
friend BeParser;

public:
    enum
    {
        UNKNOWN = 0,
        INT,
        STR,
        LIST_START,
        LIST_END,
        DICT_START,
        DICT_END,

    };

public:
    BeToken();
    BeToken(uint64_t type);

    ~BeToken();

    uint64_t get_type() const;
    std::string get(const std::string & source) const;

    // XXX debug
    std::string type_to_string() const;

private:
    uint64_t  m_type {UNKNOWN};
    SubString m_data;

};

