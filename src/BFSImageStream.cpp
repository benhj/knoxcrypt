#include "BFSImageStream.hpp"

namespace bfs
{

    BFSImageStream::BFSImageStream(std::string const &path, std::ios::openmode mode)
        : m_stream(path.c_str(), mode)
    {
    }

    BFSImageStream&
    BFSImageStream::read(char * const buf, std::streamsize const n)
    {
        (void)m_stream.read(buf, n);
        return *this;
    }

    BFSImageStream&
    BFSImageStream::write(char const * buf, std::streamsize const n)
    {
        (void)m_stream.write(buf, n);
        return *this;
    }

    BFSImageStream&
    BFSImageStream::seekg(std::streampos pos)
    {
        (void)m_stream.seekg(pos);
        return *this;
    }
    BFSImageStream&
    BFSImageStream::seekg(std::streamoff off, std::ios_base::seekdir way)
    {
        (void)m_stream.seekg(off, way);
        return *this;
    }

    BFSImageStream&
    BFSImageStream::seekp(std::streampos pos)
    {
        (void)m_stream.seekp(pos);
        return *this;
    }
    BFSImageStream&
    BFSImageStream::seekp(std::streamoff off, std::ios_base::seekdir way)
    {
        (void)m_stream.seekp(off, way);
        return *this;
    }

    std::streampos
    BFSImageStream::tellg()
    {
        return m_stream.tellg();
    }
    std::streampos
    BFSImageStream::tellp()
    {
        return m_stream.tellp();
    }

    void
    BFSImageStream::close()
    {
        m_stream.close();
    }

    void
    BFSImageStream::flush()
    {
        m_stream.flush();
    }

}
