#pragma once
#include <string>

#include "torrent_file.hpp"
#include "../jasl/common/hasher.hpp"

class DownloadManager
{
public:
    DownloadManager();
    DownloadManager(const std::string & path_to_torrent_file);
    DownloadManager(TorrentFile && torrent_file);

   ~DownloadManager();

    void add_torrent_file(const std::string & path_to_torrent_file);
    void add_torrent_file(TorrentFile && torrent_file);

    std::string generate_url() const;

private:
    std::unordered_map<TorrentFile, std::string, Hasher<TorrentFile>> m_torrents;

};
