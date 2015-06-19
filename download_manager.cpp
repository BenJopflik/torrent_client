#include "download_manager.hpp"

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
    output = "?info_hash=";
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
    output += "100";

    output += "&compact=";
    output += "1";

    output += "&event=";
    output += "started";

    return std::move(output);
}

std::string DownloadManager::generate_url() const
{
    std::string output = m_torrents.begin()->first.announce();
    return output.append(generate_init_http_request(m_torrents.begin()->first, "-2345678901234567890", 12345));
}

