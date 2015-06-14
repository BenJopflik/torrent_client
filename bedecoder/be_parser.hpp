#pragma once
#include "be_token.hpp"

class BeParser
{
public:
    static BeToken parse(const std::string & data);

private:
    static BeToken parse(const char *& data, const char * END);
    static BeToken process_integer(const char *& data, const char * END);
    static BeToken process_string(const char *& data, const char * END);
    static BeToken process_list(const char *& data, const char * END);
    static BeToken process_dictionary(const char *& data, const char * END);

};

