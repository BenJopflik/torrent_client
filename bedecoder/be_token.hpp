#pragma once
#include "common/sub_string.hpp"
#include "common/string_wrapper.hpp"

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

    uint64_t type() const;
    uint64_t length() const;

    std::string str(const StringWrapper & source) const;
    std::string substr(const StringWrapper & source, uint64_t offset, uint64_t length) const;
    const SubString & substr() const;

    // XXX debug
    std::string type_to_string() const;

private:
    uint64_t  m_type {UNKNOWN};
    SubString m_data;

};

