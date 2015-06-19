#include <cassert>
#include <iostream>

#include "bedecoder/be_parser.hpp"
#include "torrent_file.hpp"
#include "download_manager.hpp"
#include "poller/poller.hpp"
#include "socket/tcp/client.hpp"
#include "common/url.hpp"
#include "common/ip_addr.hpp"
#include "init_http_request.hpp"
#include "thread/thread_pool.hpp"
#include "common/to_string.hpp"

const uint64_t NUMBER_OF_WORKERS = 5;

static void perform(Event event)
{

    if (event.mask & Event::ERROR)
        reinterpret_cast<Socket *>(event.user_data)->error();

    if (event.mask & Event::READ)
        reinterpret_cast<Socket *>(event.user_data)->read();

    if (event.mask & Event::WRITE)
        reinterpret_cast<Socket *>(event.user_data)->write();

    if (event.mask & Event::CLOSE)
        reinterpret_cast<Socket *>(event.user_data)->close();

    if (event.mask & Event::REARM)
        reinterpret_cast<Socket *>(event.user_data)->rearm();
}





int main(int argc, char * argv[])
{
    assert(argc > 1 && "usage: ./main torrent_file");


    DownloadManager dm;
    dm.add_torrent_file(argv[1]);
    dm.generate_url();

    Url url(dm.generate_url());

    IpAddr ip_addr;
    ip_addr.set(url.host(), std::stoll(url.port()));

    std::cerr << ip_addr.full_addr << std::endl;


    Poller::Params params;
    params.poll_interval = 100;
    params.queue_size = 10;

    Poller poller(params);

    TcpClient client(ip_addr.ip, ip_addr.port);
    client.set_callbacks<InitHttpRequest>(url);
    client.add_to_poller(EPOLLOUT | EPOLLONESHOT, &poller);

    ThreadPool pool(NUMBER_OF_WORKERS);

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
    }



    return 0;
}
