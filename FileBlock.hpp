#ifndef BFS_FILE_BLOCK_HPP__
#define BFS_FILE_BLOCK_HPP__

#include <vector>

#include <boost/iostreams/categories.hpp>  // sink_tag
#include <iosfwd>                          // streamsize
#include <string>
#include <fstream>


namespace bfs
{

    class FileBlock
    {
    public:
        FileBlock(uint64_t const index)
        {

        }

        std::streamsize read(char_type * const buf, std::streamsize const n) const
        std::streamsize write(char_type const * const buf, std::streamsize const n) const;

    private:

        uint64_t m_index;
        uint64_t m_offset;

    };

}

#endif // BFS_BFS_FILE_BLOCK_HPP__
