#ifndef BFS_FILE_ENTRY_HPP__
#define BFS_FILE_ENTRY_HPP__

#include "FileBlock.hpp"

#include <vector>

#include <boost/iostreams/categories.hpp>  // sink_tag
#include <iosfwd>                          // streamsize
#include <string>
#include <fstream>


namespace bfs
{

    class FileEntry
    {
    public:
        FileEntry(std::string const &name)
        : m_name(name)
        {

        }

        FileEntry(uint64_t const startBlock)
        {

        }

        std::streamsize read(char_type * const buf, std::streamsize const n) const
        std::streamsize write(char_type const * const buf, std::streamsize const n) const;

    private:
        std::string m_name;
        uint64_t m_fileSize;

        // vector for storing file blocks
        std::vector<FileBlock> m_fileBlocks;
    };

}

#endif // BFS_BFS_FILE_ENTRY_HPP__
