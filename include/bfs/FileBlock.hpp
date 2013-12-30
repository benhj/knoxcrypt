/*
The MIT License (MIT)

Copyright (c) 2013 Ben H.D. Jones

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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
         * @param io the core bfs io (path, blocks, password)
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
         * @param io the core bfs io (path, blocks, password)
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
