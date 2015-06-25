#pragma once
#include "bedecoder/be_token.hpp"
#include <vector>
#include <unordered_map>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "token_list_processor.hpp"

struct Peer
{
    std::string id   {""};
    std::string ip   {""};
    uint16_t    port {0};

    sockaddr_in    saddr;
};

class TrackerResponse : public TokenListProcessor<TrackerResponse>
{
public:
    TrackerResponse(const std::vector<BeToken> & tokens, const char * source, uint64_t size);

    operator bool () const {return m_valid;}

    void print() const;

    uint64_t interval() const {return m_interval;}
    uint64_t min_interval() const {return m_min_interval;}
    uint64_t complete() const {return m_complete;}
    uint64_t incomplete() const {return m_incomplete;}

    const std::vector<Peer> & peers() const {return m_peers;}

    const std::string & failure_reason() const {return m_failure_reason;}
    const std::string & warning() const {return m_warning;}
    const std::string & traker_id() const {return m_tracker_id;}

private:
    MEMBER_DECLARE(fill_failure_reason)
    MEMBER_DECLARE(fill_warning)
    MEMBER_DECLARE(fill_interval)
    MEMBER_DECLARE(fill_min_interval)
    MEMBER_DECLARE(fill_tracker_id)
    MEMBER_DECLARE(fill_complete)
    MEMBER_DECLARE(fill_incomplete)
    MEMBER_DECLARE(fill_peers)
    MEMBER_DECLARE(fill_peers_compact)
    MEMBER_DECLARE(fill_peers_detail)

private:
    std::vector<Peer> m_peers;

    std::string m_failure_reason {""};
    std::string m_warning        {""};
    std::string m_tracker_id     {""};

    uint64_t m_interval     {0}; // in seconds
    uint64_t m_min_interval {0}; // in seconds
    uint64_t m_complete     {0};
    uint64_t m_incomplete   {0};

    bool m_valid {false};
};
#undef MEMBER_DECLARE

