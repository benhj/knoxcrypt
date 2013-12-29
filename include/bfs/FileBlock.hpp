#ifndef BFS_FILE_BLOCK_HPP__
#define BFS_FILE_BLOCK_HPP__

#include "bfs/BFSImageStream.hpp"
#include "bfs/CoreBFSIO.hpp"
#include "bfs/DetailBFS.hpp"
#include "bfs/DetailFileBlock.hpp"
#include "bfs/OpenDisposition.hpp"

#include <vector>

#include <boost/iostreams/categories.hpp>  // sink_tag
#include <boost/iostreams/positioning.hpp>  // stream_offset
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
         * @param openDisposition open mode
         */
        FileBlock(CoreBFSIO const &io,
                  uint64_t const index,
                  uint64_t const next,
                  OpenDisposition const &openDisposition);

        /**
         * @brief for when a file block needs to be read or written use this constructor
         * @param index the index of the file block
         * @param openDisposition open mode
         * @note other params like size and next will be initialized when
         * the block is actually read
         */
        FileBlock(CoreBFSIO const &io,
                  uint64_t const index,
                  OpenDisposition const &openDisposition);

        std::streamsize read(char * const buf, std::streamsize const n) const;

        std::streamsize write(char const * const buf, std::streamsize const n) const;

        boost::iostreams::stream_offset seek(boost::iostreams::stream_offset off,
                                             std::ios_base::seekdir way = std::ios_base::beg);

        boost::iostreams::stream_offset tell() const;

        uint32_t getDataBytesWritten() const;

        uint32_t getInitialDataBytesWritten() const;

        uint64_t getNextIndex() const;

        uint64_t getBlockOffset() const;

        uint64_t getIndex() const;

        void registerBlockWithVolumeBitmap();

        /**
         * @brief when we want to set number of bytes written
         * useful when truncating
         * @param size the number of bytes written
         */
        void setSize(std::ios_base::streamoff size) const;

        /**
         * @brief sets the next index of 'this'
         * @param nextIndex the next index to set this to
         */
        void setNextIndex(uint64_t nextIndex) const;

      private:

        FileBlock();

        /**
         * @brief sets number of bytes written
         * @param size
         */
        void doSetSize(BFSImageStream &stream, std::ios_base::streamoff size) const;

        /**
         * @brief sets the next index of the block
         * @param stream the image stream to operate over
         * @param nextIndex the next index to set next of this to
         */
        void doSetNextIndex(BFSImageStream &stream, uint64_t nextIndex) const;

        CoreBFSIO m_io;
        uint64_t m_index;
        mutable uint32_t m_bytesWritten;
        mutable uint32_t m_initialBytesWritten;
        mutable uint64_t m_next;
        mutable uint64_t m_offset;
        mutable boost::iostreams::stream_offset m_seekPos;
        OpenDisposition m_openDisposition;

    };

}

#endif // BFS_FILE_BLOCK_HPP__
