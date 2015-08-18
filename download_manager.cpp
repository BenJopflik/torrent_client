#include "download_manager.hpp"
#include "common/url.hpp"
#include "common/ip_addr.hpp"

#include <iostream>

static uint64_t PEER_ID_LENGTH = 20;

DownloadManager::DownloadManager()
{

}

DownloadManager::DownloadManager(TorrentFile && torrent_file)
{
    add_torrent_file(std::move(torrent_file));
}

DownloadManager::DownloadManager(const std::string & path_to_torrent_file)
{
    add_torrent_file(path_to_torrent_file);
}


DownloadManager::~DownloadManager()
{

}

void DownloadManager::add_torrent_file(const std::string & path_to_torrent_file)
{
    try
    {
        TorrentFile tf(path_to_torrent_file);
        add_torrent_file(std::move(tf));
    }
    catch (const std::exception & e)
    {
        std::cerr << e.what() << std::endl;
        return;
    }
}

void DownloadManager::add_torrent_file(TorrentFile && torrent_file)
{
    // TODO add checks
    m_torrents.insert(std::make_pair(torrent_file, ""));
}

static std::string generate_init_http_request(const TorrentFile & tf, const std::string & peer_id, uint16_t port)
{
    if (peer_id.size() != PEER_ID_LENGTH)
        throw std::runtime_error(std::string("peer_id string must be ").append(std::to_string(PEER_ID_LENGTH)).append(" char long"));

    std::string output;
    output = "info_hash=";
    output += tf.info_hash();

    output += "&peer_id=";
    output += peer_id;

    output += "&port=";
    output += std::to_string(port);

    output += "&uploaded=";
    output += "0";

    output += "&downloaded=";
    output += "0";

    output += "&left=";
    output += std::to_string(tf.full_size());

    output += "&numwant=";
    output += "1000";

    output += "&compact=";
    output += "1";

    output += "&event=";
    output += "started";

    return std::move(output);
}

std::string DownloadManager::generate_url() const
{
//    std::string output = m_torrents.begin()->first.announce();
    std::vector<std::string> available_announces;
    available_announces.push_back(m_torrents.begin()->first.announce());
    auto announce_list = m_torrents.begin()->first.announce_list();
    std::copy(announce_list.begin(), announce_list.end(), std::back_inserter(available_announces));


    std::string valid_url;
    IpAddr ip_addr;
    for (const auto & i : available_announces)
    {
        auto local = i.find("local");
        if (local != std::string::npos)
            continue;

        Url url(i);
        if (!url)
            continue;

        if (!url.proto().empty() && url.proto() != "http")
            continue;

        try
        {
            ip_addr.clear();
            ip_addr.set(url.host(), 0);
            valid_url = i;
            break;
        }
        catch (const std::exception & e)
        {
            std::cerr << e.what() << std::endl;
            continue;
        }
    }

    if (valid_url.empty())
        throw std::runtime_error("no valid announce");


    Url url(valid_url);
    std::string output;

    output = url.host();

    if (!url.port().empty())
        output += ":" + url.port();

    if (!url.tail().empty())
    {
        std::string tail = "/";
        tail += url.tail().substr(0, url.tail().size() - 1); // XXX fix it
        auto delim = tail.rfind('?');
        output += tail;
        output += (delim == std::string::npos ? '?' : '&');
    }
    else
        output += '/';

    return output.append(generate_init_http_request(m_torrents.begin()->first, "-2345678901234567890", 12345));
}

