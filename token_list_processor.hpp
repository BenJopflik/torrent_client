#pragma once

#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <typeinfo>
#include <iostream>

#include "bedecoder/be_token.hpp"
#include "common/string_wrapper.hpp"

#define INPUT_ARGS (const std::vector<BeToken> & tokens, const StringWrapper & source, uint64_t & current_token)
#define MEMBER_DECLARE(function_name) void function_name INPUT_ARGS;
#define MEMBER_DEFINE(class_name, function_name) void class_name::function_name INPUT_ARGS

template <class T>
class TokenListProcessor
{

using TMemberFunction = void (T::*)(const std::vector<BeToken> & tokens, const StringWrapper & source, uint64_t & current_token);

protected:
    TokenListProcessor();
    virtual ~TokenListProcessor() = 0;

    void process(const std::vector<BeToken> & tokens, const StringWrapper & source);
    void fill_str(std::string & dest, const std::vector<BeToken> & tokens, const StringWrapper & source, uint64_t & current_token);
    void fill_int(uint64_t & dest, const std::vector<BeToken> & tokens, const StringWrapper & source, uint64_t & current_token);

protected:
    std::unordered_map<std::string, TMemberFunction> m_fillers;

};

template <class T>
TokenListProcessor<T>::TokenListProcessor()
{

}

template <class T>
TokenListProcessor<T>::~TokenListProcessor()
{

}

template <class T>
void TokenListProcessor<T>::process(const std::vector<BeToken> & tokens, const StringWrapper & source)
{
    if (m_fillers.empty())
        throw std::runtime_error(std::string("m_fillers is empty for class: ").append(typeid(T).name()));

    uint64_t current_token = 0;
    auto filler = m_fillers.end();
    uint64_t type = 0;
    const uint64_t TOKENS_SIZE = tokens.size();

    T * derived = dynamic_cast<T*>(this);
    if (!derived)
        throw std::runtime_error(std::string("dynamic_cast failed to class: ").append(typeid(T).name()));

    while (current_token < TOKENS_SIZE)
    {
        type = tokens[current_token].type();
        if (type == BeToken::INT || type == BeToken::STR)
        {
            filler = m_fillers.find(tokens[current_token].str(source));
            if (filler != m_fillers.end())
            {
                std::cerr << filler->first << std::endl;
                (derived->*(filler->second))(tokens, source, current_token);
                continue;
            }
        }

        ++current_token;
    }
}

template <class T>
void TokenListProcessor<T>::fill_int(uint64_t & dest, const std::vector<BeToken> & tokens, const StringWrapper & source, uint64_t & current_token)
{
    ++current_token;
    uint64_t type = tokens[current_token].type();
    if (type == BeToken::INT)
        dest = std::stoll(tokens[current_token].str(source));

    ++current_token;
}

template <class T>
void TokenListProcessor<T>::fill_str(std::string & dest, const std::vector<BeToken> & tokens, const StringWrapper & source, uint64_t & current_token)
{
    ++current_token;
    uint64_t type = tokens[current_token].type();
    if (type == BeToken::STR)
        dest = tokens[current_token].str(source);

    ++current_token;
}

