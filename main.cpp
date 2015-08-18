#include <cassert>
#include <iostream>
#include <memory>

#include "bedecoder/be_parser.hpp"
#include "torrent_file.hpp"
#include "download_manager.hpp"
#include "poller/poller.hpp"
#include "socket/tcp/client.hpp"
#include "common/url.hpp"
#include "common/ip_addr.hpp"
#include "common/timer.hpp"
#include "init_http_request.hpp"
#include "thread/thread_pool.hpp"
#include "common/to_string.hpp"
#include "active_sockets.hpp"
#include "files.hpp"
#include "piece_list.hpp"

const uint64_t NUMBER_OF_WORKERS = 1;

static void perform(Event event)
try
{
    if (event.mask & Event::ERROR)
    {
        reinterpret_cast<Socket *>(event.user_data)->error();
        return;
    }

    if (event.mask & Event::READ)
        reinterpret_cast<Socket *>(event.user_data)->read();

    if (event.mask & Event::WRITE)
        reinterpret_cast<Socket *>(event.user_data)->write();

    if (event.mask & Event::CLOSE)
        reinterpret_cast<Socket *>(event.user_data)->close();

    if (event.mask & Event::REARM)
        reinterpret_cast<Socket *>(event.user_data)->rearm();
}
catch (...)
{
    reinterpret_cast<Socket *>(event.user_data)->close();
    std::cout << "exception" << std::endl;
}

int main(int argc, char * argv[])
{
    assert(argc > 1 && "usage: ./main torrent_file");

    ActiveSockets active_sockets;

    TorrentFile tf(argv[1]);
    std::string info_hash = tf.info_hash_bin();
    Files files(tf);
    PieceList pl(tf);

    DownloadManager dm;
    dm.add_torrent_file(std::move(tf));
    dm.generate_url();

    Url url(dm.generate_url());

    uint16_t port = 80;
    if (!url.port().empty())
        port = std::stoll(url.port());

    IpAddr ip_addr;
    ip_addr.set(url.host(), port);

    std::cerr << ip_addr.full_addr << std::endl;


    Poller::Params params;
    params.poll_interval = 100;
    params.queue_size = 10;

    Poller poller(params);

    Socket * socket = create_tcp_client(ip_addr.ip, ip_addr.port);
    socket->set_callbacks<TrackerConnection>(url, std::ref(active_sockets), std::ref(info_hash), std::ref(files), std::ref(pl));
    socket->add_to_poller(Action::WRITE, &poller);

    ThreadPool pool(NUMBER_OF_WORKERS);

    Timer timer;

    Event event;
    auto poller_worker = poller.run_poll_in_thread();
    for (;;)
    {
        if (poller.get_event(event))
        {
            if (event.mask & Event::STOP)
                break;
            std::cerr << "event.what: " << event_mask_to_string(event.mask) << std::endl;
            pool.add_task(std::bind(&perform, event));
        }

//        if (timer.elapsed_seconds() > 30)
//        {
//            timer.reset();
//            Socket * socket = create_tcp_client(ip_addr.ip, ip_addr.port);
//            //    active_sockets.add(std::move(socket_));
//            socket->set_callbacks<TrackerConnection>(url, std::ref(active_sockets), std::ref(info_hash), std::ref(files), std::ref(pl));
//            socket->add_to_poller(Action::WRITE, &poller);
//        }

    }
    std::cerr << "EXIT" << std::endl;
    poller.stop();

    return 0;
}
