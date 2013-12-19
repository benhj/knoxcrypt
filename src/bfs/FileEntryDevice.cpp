#include "bfs/FileEntryDevice.hpp"

namespace bfs
{

    FileEntryDevice::FileEntryDevice(FileEntry const &entry)
        : m_entry(entry)
    {
    }

    std::streamsize
    FileEntryDevice::read(char* s, std::streamsize n)
    {
        return m_entry.read(s, n);
    }

    std::streamsize
    FileEntryDevice::write(const char* s, std::streamsize n)
    {
        std::streamsize wrote = m_entry.write(s, n);
        m_entry.flush();
        return wrote;
    }

    std::streampos
    FileEntryDevice::seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way)
    {
        return m_entry.seek(off, way);
    }
}

