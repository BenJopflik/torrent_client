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

    uint64_t type() const;
    uint64_t length() const;

    std::string str(const std::string & source) const;
    std::string str(const char * source, uint64_t size) const;

    std::string substr(const std::string & source, uint64_t offset, uint64_t length) const;
    std::string substr(const char * source, uint64_t size, uint64_t offset, uint64_t length) const;
    const SubString & get_substring() const;

    // XXX debug
    std::string type_to_string() const;

private:
    uint64_t  m_type {UNKNOWN};
    SubString m_data;

};

