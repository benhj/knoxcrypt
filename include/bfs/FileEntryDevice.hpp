#ifndef BFS_FILE_ENTRY_DEVICE_HPP__
#define BFS_FILE_ENTRY_DEVICE_HPP__

#include <bfs/FileEntry.hpp>

#include <iosfwd>                           // streamsize, seekdir
#include <boost/iostreams/categories.hpp>   // seekable_device_tag
#include <boost/iostreams/positioning.hpp>  // stream_offset

namespace bfs
{
    class FileEntryDevice
    {

        public:

            typedef char                                   char_type;
            typedef boost::iostreams::seekable_device_tag  category;

            explicit FileEntryDevice(FileEntry const &entry);

            std::streamsize read(char* s, std::streamsize n);
            std::streamsize write(const char* s, std::streamsize n);
            std::streampos seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way);

        private:
            FileEntryDevice();
            FileEntry m_entry;
    };

}

#endif // BFS_FILE_ENTRY_DEVICE_HPP__
