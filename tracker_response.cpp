#include "tracker_response.hpp"
#include <iostream>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "common/ip_addr.hpp"

TrackerResponse::TrackerResponse(const std::vector<BeToken> & tokens, const char * source, uint64_t size)
{
    m_fillers = {
                    {"peers",           &TrackerResponse::fill_peers},
                    {"interval",        &TrackerResponse::fill_interval},
                    {"complete",        &TrackerResponse::fill_complete},
                    {"incomplete",      &TrackerResponse::fill_incomplete},
                    {"tracker id",      &TrackerResponse::fill_tracker_id},
                    {"min interval",    &TrackerResponse::fill_min_interval},
                    {"failure reason",  &TrackerResponse::fill_failure_reason},
                    {"warning message", &TrackerResponse::fill_warning}
                };

    uint64_t type = tokens[0].type();

    if (type != BeToken::DICT_START)
        throw std::runtime_error("invalid tracker respons. not a dict");

    process(tokens, {source, size});

    if (m_failure_reason.empty() && m_warning.empty() && !m_peers.empty())
        m_valid = true;
}

MEMBER_DEFINE(TrackerResponse, fill_failure_reason)
{
    fill_str(m_failure_reason, tokens, source, current_token);
}

MEMBER_DEFINE(TrackerResponse, fill_warning)
{
    fill_str(m_warning, tokens, source, current_token);
}

MEMBER_DEFINE(TrackerResponse, fill_interval)
{
    fill_int(m_interval, tokens, source, current_token);
}

MEMBER_DEFINE(TrackerResponse, fill_min_interval)
{
    fill_int(m_min_interval, tokens, source, current_token);
}

MEMBER_DEFINE(TrackerResponse, fill_tracker_id)
{
    fill_str(m_tracker_id, tokens, source, current_token);
}

MEMBER_DEFINE(TrackerResponse, fill_complete)
{
    fill_int(m_complete, tokens, source, current_token);
}

MEMBER_DEFINE(TrackerResponse, fill_incomplete)
{
    fill_int(m_incomplete, tokens, source, current_token);
}

MEMBER_DEFINE(TrackerResponse, fill_peers)
{
    ++current_token;
    uint64_t type = tokens[current_token].type();
    if (type == BeToken::STR)
        return fill_peers_compact(tokens, source, current_token);
    else
        return fill_peers_detail(tokens, source, current_token);
}

MEMBER_DEFINE(TrackerResponse, fill_peers_compact)
{
    uint64_t number_of_peers = tokens[current_token].substr().size() / 6; // 4 - ip 2 - port in network notation
    m_peers.resize(number_of_peers);

    char buf[INET_ADDRSTRLEN];
    const char * curr = source.data() + tokens[current_token].substr().start();
    sockaddr_in * si = nullptr;

    for (uint64_t i = 0; i < number_of_peers; ++i)
    {
        if (inet_ntop(AF_INET, curr, buf, INET_ADDRSTRLEN))
            m_peers[i].ip = buf;
        else
            throw std::runtime_error("invalid ip");

        si = &m_peers[i].saddr;
        si->sin_family = AF_INET;
        si->sin_addr.s_addr = *(uint32_t *)curr;

        curr += 4;

        m_peers[i].port = ntohs(*(uint16_t *)curr);
        si->sin_port = *(uint16_t *)curr;

        curr += 2;
    }

    ++current_token;
}

MEMBER_DEFINE(TrackerResponse, fill_peers_detail)
{
    uint64_t type = tokens[current_token].type();
    if (type != BeToken::LIST_START)
        throw std::runtime_error("invalid betype. expected list_start");

    ++current_token;

    std::string key = "";
    std::string value = "";

    const uint64_t TOKEN_SIZE = tokens.size();
    while (current_token < TOKEN_SIZE)
    {
        type = tokens[current_token].type();
        switch (type)
        {
            case BeToken::DICT_START:
                m_peers.push_back(Peer());
                break;

            case BeToken::DICT_END:
                m_peers.back().saddr = IpAddr::get_sockaddr_in(m_peers.back().ip, m_peers.back().port);
                break;

            case BeToken::STR:
                key = tokens[current_token].str(source);
                ++current_token;
                value = tokens[current_token].str(source);

                if (key == "peer id")
                        m_peers.back().id = value;
                else if (key == "ip")
                        m_peers.back().ip = value;
                else if (key == "port")
                        m_peers.back().port = stol(value);

                break;
        }

        ++current_token;
    }
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
#undef MEMBER_DEFINE
