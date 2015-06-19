#pragma once
#include <vector>
#include "be_token.hpp"

class BeParser
{
public:
    static std::vector<BeToken> parse(const std::string & source);
    static std::vector<BeToken> parse(const char * source, uint64_t length);

private:
    static std::vector<BeToken> parse             (const char * BEGIN, uint64_t & offset, uint64_t length);
    static std::vector<BeToken> process_integer   (const char * BEGIN, uint64_t & offset, uint64_t length);
    static std::vector<BeToken> process_string    (const char * BEGIN, uint64_t & offset, uint64_t length);
    static std::vector<BeToken> process_struct    (const char * BEGIN, uint64_t & offset, uint64_t length, bool is_list = false);

};

