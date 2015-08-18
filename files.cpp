#include "files.hpp"

#include <vector>
#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include "torrent_file.hpp"

// we'll modify absolute_path
static void create_absolute_path(std::string absolute_path)
{
    const uint64_t SIZE = absolute_path.size();
    struct stat st;

    uint64_t delimiter_offset = 0;

    while (delimiter_offset < SIZE)
    {
        delimiter_offset = absolute_path.find('/', delimiter_offset);

        if (delimiter_offset != std::string::npos)
            absolute_path[delimiter_offset] = 0;

        if (::stat(absolute_path.c_str(), &st))
        {
            mkdir(absolute_path.c_str(), 0700);
        }
        else
        {
            if (!(st.st_mode & S_IFDIR))
                throw std::runtime_error(std::string("not a dir: ").append(absolute_path));
        }

        if (delimiter_offset == std::string::npos)
            break;
        absolute_path[delimiter_offset] = '/';
        ++delimiter_offset;
    }
}

static int open_file(const std::string & absolute_path, const std::vector<std::string> & relative_path)
{
    struct stat st;
    if (::stat(absolute_path.c_str(), &st))
        create_absolute_path(absolute_path);
    else if (!(st.st_mode & S_IFDIR))
        throw std::runtime_error(std::string("invalid absolute_path: ").append(absolute_path));

    std::string full_path = absolute_path + "/";

    const uint64_t RELATIVE_PATH_SIZE = relative_path.size() - 1;
    for (uint64_t i = 0; i < RELATIVE_PATH_SIZE; ++i)
    {
        full_path += relative_path[i];
        full_path += "/";
        if (::stat(full_path.c_str(), &st))
        {
            mkdir(full_path.c_str(), 0700);
        }
        else
        {
            if (!(st.st_mode & S_IFDIR))
                throw std::runtime_error(std::string("not a dir: ").append(full_path));
        }
    }

    full_path += relative_path.back();
    int fd = ::open(full_path.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd < 0)
        throw std::runtime_error(std::string("unable to open file: ").append(full_path).append(". reason: ").append(strerror(errno)));

    return fd;
}

class File
{
public:
    File(const FileInfo & fi, uint64_t piece_size);
    ~File();

    void write_piece(uint64_t id, const uint8_t * source, uint64_t size);
    bool is_full() const;
    const FileInfo & file_info() const {return m_fi;}

private:
    BitSet m_bitset;
    FileInfo m_fi;
    uint64_t m_piece_size {0};
    bool m_full {false};
    int  m_fd {-1};

};

File::File(const FileInfo & fi, uint64_t piece_size) : m_fi(fi), m_piece_size(piece_size)
{
    m_bitset.bind((fi.last_piece.index - fi.first_piece.index + 1) * 8);
    m_fd = open_file(fi.dir_name, fi.path);
}

File::~File()
{
    ::close(m_fd);
}

// TODO add EINTR check
void File::write_piece(uint64_t id, const uint8_t * source, uint64_t source_size)
{
    // TODO add warning
    if (id < m_fi.first_piece.index || id > m_fi.last_piece.index)
        return;

    uint64_t offset = (id == m_fi.first_piece.index) ? m_fi.first_piece.offset : 0;
    int64_t size = ((id == m_fi.last_piece.index) ? m_fi.last_piece.offset : source_size) - offset;

    uint64_t offset_in_file = 0;
    if (id != m_fi.first_piece.index)
        offset_in_file = m_piece_size - m_fi.first_piece.offset + (id - m_fi.first_piece.index) * m_piece_size;

// TODO add EINTR check
    if (::pwrite(m_fd, source + offset, size, offset_in_file) != size)
        throw std::runtime_error(std::string("error while writing: ").append(strerror(errno)));

    m_bitset[id - m_fi.first_piece.index] = true;
}


Files::Files(const TorrentFile & tf)
{
    m_files.reserve(tf.files().size());
    for (const auto & file : tf.files())
        m_files.emplace_back(file, tf.piece_size());
}

Files::~Files()
{

}

static uint64_t search(const File * data, uint64_t begin, uint64_t end, uint64_t value)
{
    uint64_t middle = (end + begin) / 2;
    if (value > data[middle].file_info().last_piece.index)
        return search(data, middle, end, value);
    else if (value < data[middle].file_info().first_piece.index)
        return search(data, begin, middle, value);

    while (middle && data[middle].file_info().first_piece.index == value)
        --middle;

    return middle;
}

void Files::write_piece(uint64_t id, const uint8_t * source, uint64_t size)
{
    const uint64_t SIZE = m_files.size();
    uint64_t file_id = search(m_files.data(), 0, SIZE, id);
    do
    {
        m_files[file_id].write_piece(id, source, size);
        ++file_id;
    }
    while (file_id < SIZE && m_files[file_id].file_info().first_piece.index == id);
}

