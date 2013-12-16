#ifndef BFS_FILE_BLOCK_HPP__
#define BFS_FILE_BLOCK_HPP__

#include "AppendOrOverwrite.hpp"
#include "BFSImageStream.hpp"
#include "DetailBFS.hpp"
#include "DetailFileBlock.hpp"

#include <vector>

#include <boost/iostreams/categories.hpp>  // sink_tag
#include <iosfwd>                          // streamsize
#include <string>


namespace bfs
{

    class FileBlock
    {
      public:
        /**
         * @brief for when a file block needs to be written for the first time
         * use this constructor
         * @param imagePath the path of the bfs image
         * @param totalBlocks the total number of blocks in the fs image
         * @param index the index of this file block
         * @param next the index of the next file block that makes up the file
         */
        FileBlock(std::string const &imagePath,
                  uint64_t const totalBlocks,
                  uint64_t const index,
                  uint64_t const next,
                  AppendOrOverwrite const appendOrOverwrite = AppendOrOverwrite::Append);

        /**
         * @brief for when a file block needs to be read or written use this constructor
         * @param index the index of the file block
         * @note other params like size and next will be initialized when
         * the block is actually read
         */
        FileBlock(std::string const &imagePath,
                  uint64_t const totalBlocks,
                  uint64_t const index,
                  AppendOrOverwrite const appendOrOverwrite = AppendOrOverwrite::Append);

        /**
         * @brief in case needing to skip some bytes
         * @param extraOffset the amount to update m_offset by
         */
        void setExtraOffset(uint64_t const extraOffset);

        std::streamsize read(char * const buf, std::streamsize const n) const;

        std::streamsize write(char const * const buf, std::streamsize const n) const;

        uint32_t getDataBytesWritten() const;

        uint32_t getInitialDataBytesWritten() const;

        uint64_t getNextIndex() const;

        uint64_t getBlockOffset() const;

        uint64_t getIndex() const;

        void setNext(uint64_t const next);

        void registerBlockWithVolumeBitmap();

      private:

        FileBlock();

        std::string m_imagePath;
        uint64_t m_totalBlocks;
        uint64_t m_index;
        mutable uint32_t m_bytesWritten;
        mutable uint32_t m_initialBytesWritten;
        mutable uint64_t m_next;
        mutable uint64_t m_offset;
        mutable uint64_t m_extraOffset;
        AppendOrOverwrite m_writeMode;

    };

}

#endif // BFS_FILE_BLOCK_HPP__
