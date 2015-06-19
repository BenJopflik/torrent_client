#pragma once
#include "bedecoder/be_token.hpp"
#include <vector>
#include <unordered_map>

struct Peer
{
    std::string id {""};
    std::string ip {""};
    uint16_t    port {0};

    sockaddr    saddr;
};

class TrackerResponse
{
public:
    TrackerResponse(const std::vector<BeToken> & tokens, const char * source, uint64_t size);

    void print() const;

private:
    void fill_failure_reason(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);
    void fill_warning(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);
    void fill_interval(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);
    void fill_min_interval(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);
    void fill_tracker_id(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);
    void fill_complete(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);
    void fill_incomplete(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);
    void fill_peers(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);
    void fill_peers_compact(const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);

    void fill_str(std::string & dest, const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);
    void fill_int(uint64_t & dest, const std::vector<BeToken> & tokens, const std::string & source, uint64_t & current_token);



private:
    const std::unordered_map<std::string, decltype(&TrackerResponse::fill_failure_reason)> m_fillers {
                                                                                        {"failure reason", &TrackerResponse::fill_failure_reason},
                                                                                        {"warning message", &TrackerResponse::fill_warning},
                                                                                        {"interval", &TrackerResponse::fill_interval},
                                                                                        {"min interval", &TrackerResponse::fill_min_interval},
                                                                                        {"tracker id", &TrackerResponse::fill_tracker_id},
                                                                                        {"complete", &TrackerResponse::fill_complete},
                                                                                        {"incomplete", &TrackerResponse::fill_incomplete},
                                                                                        {"peers", &TrackerResponse::fill_peers}

                                                                                      };


    std::vector<Peer> m_peers;

    std::string m_failure_reason {""};
    std::string m_warning {""};
    std::string m_tracker_id {""};

    uint64_t m_interval {0}; // in seconds
    uint64_t m_min_interval {0}; // in seconds
    uint64_t m_complete {0};
    uint64_t m_incomplete {0};


};
