#include "tracker_response.hpp"
#include <iostream>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

TrackerResponse::TrackerResponse(const std::vector<BeToken> & tokens, const char * source_, uint64_t size)
{
    std::string source(source_, size); // TODO remove
    uint64_t current_token = 0;
    auto filler = m_fillers.end();
    uint64_t type = tokens[current_token++].type();

    if (type != BeToken::DICT_START)
        throw std::runtime_error("invalid tracker respons. not a dict");

    const uint64_t TOKENS_SIZE = tokens.size();
    while (current_token < TOKENS_SIZE)
    {
        type = tokens[current_token].type();
        if (type == BeToken::INT || type == BeToken::STR)
        {
            filler = m_fillers.find(tokens[current_token].str(source));
            if (filler != m_fillers.end())
            {
                (this->*(filler->second))(tokens, source, current_token);
                continue;
            }
        }

        ++current_token;
    }
}

void TrackerResponse::fill_int(uint64_t & dest, const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    ++current_token;
    uint64_t type = tokens[current_token].type();
    if (type == BeToken::INT)
        dest = std::stoll(tokens[current_token].str(source));

    ++current_token;
}

void TrackerResponse::fill_str(std::string & dest, const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    ++current_token;
    uint64_t type = tokens[current_token].type();
    if (type == BeToken::STR)
        dest = tokens[current_token].str(source);

    ++current_token;
}

void TrackerResponse::fill_failure_reason(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    fill_str(m_failure_reason, tokens, source, current_token);
}

void TrackerResponse::fill_warning(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    fill_str(m_warning, tokens, source, current_token);
}

void TrackerResponse::fill_interval(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    fill_int(m_interval, tokens, source, current_token);
}

void TrackerResponse::fill_min_interval(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    fill_int(m_min_interval, tokens, source, current_token);
}

void TrackerResponse::fill_tracker_id(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    fill_str(m_tracker_id, tokens, source, current_token);
}

void TrackerResponse::fill_complete(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    fill_int(m_complete, tokens, source, current_token);
}

void TrackerResponse::fill_incomplete(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    fill_int(m_incomplete, tokens, source, current_token);
}

void TrackerResponse::fill_peers(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    ++current_token;
    uint64_t type = tokens[current_token].type();
    if (type == BeToken::STR)
        return fill_peers_compact(tokens, source, current_token);

    current_token = tokens.size();
}

void TrackerResponse::fill_peers_compact(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token)
{
    std::string peers = tokens[current_token].str(source);

    uint64_t number_of_peers = peers.size() / 6; // 4 - ip 2 - port in network notation
    m_peers.resize(number_of_peers);

    char buf[INET_ADDRSTRLEN];
    const char * curr = peers.c_str();
    for (int i = 0; i < number_of_peers; ++i)
    {
        if (inet_ntop(AF_INET, curr, buf, INET_ADDRSTRLEN))
            m_peers[i].ip = buf;
        else
            throw std::runtime_error("invalid ip");

        curr += 4;
        m_peers[i].port = ntohs(*(uint16_t *)curr);
        curr += 2;
    }

    ++current_token;
}


void TrackerResponse::print() const
{
    std::cerr << "failure reason: " << m_failure_reason << std::endl;
    std::cerr << "warning: " << m_warning << std::endl;
    std::cerr << "tracker_id: " << m_tracker_id << std::endl;
    std::cerr << "interval: " << m_interval << std::endl;
    std::cerr << "min interval: " << m_min_interval << std::endl;
    std::cerr << "complete: " << m_complete << std::endl;
    std::cerr << "incomplete: " << m_incomplete << std::endl;

    std::cerr << "peers: " << std::endl;
    for (const auto & i : m_peers)
    {
        std::cerr << "\tid: " << i.id << std::endl;
        std::cerr << "\tip: " << i.ip << std::endl;
        std::cerr << "\tport: " << i.port << std::endl;
        std::cerr << std::endl;
    }

}
