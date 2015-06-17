#include <cassert>
#include "bedecoder/be_parser.hpp"
#include "torrent_file.hpp"

int main(int argc, char * argv[])
{
    assert(argc > 1 && "usage: ./main torrent_file");

    TorrentFile tf(argv[1]);
    tf.print();
    return 0;
}
