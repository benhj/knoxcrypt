#ifndef BFS_BFS_IMAGE_STREAM_HPP__
#define BFS_BFS_IMAGE_STREAM_HPP__

#include "cipher/StreamCipher.hpp"

#include <fstream>
#include <string>

namespace bfs
{

    class BFSImageStream
    {
      public:
        explicit BFSImageStream(std::string const &path,
                                std::ios::openmode mode = std::ios::out | std::ios::binary);

        BFSImageStream& read(char * const buf, std::streamsize const n);

        BFSImageStream& write(char const * buf, std::streamsize const n);

        BFSImageStream& seekg(std::streampos pos);
        BFSImageStream& seekg(std::streamoff off, std::ios_base::seekdir way);
        BFSImageStream& seekp(std::streampos pos);
        BFSImageStream& seekp(std::streamoff off, std::ios_base::seekdir way);
        std::streampos tellg();
        std::streampos tellp();

        void flush();

        void close();

      private:
        BFSImageStream();
        std::fstream m_stream;
        cipher::StreamCipher m_cipher;
    };

}

#endif // BFS_BFS_IMAGE_STREAM_HPP__
