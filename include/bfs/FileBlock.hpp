/*
Copyright (c) <2013-2014>, <BenHJ>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
*/

#ifndef BFS_FILE_BLOCK_HPP__
#define BFS_FILE_BLOCK_HPP__

#include "bfs/BFSImageStream.hpp"
#include "bfs/CoreBFSIO.hpp"
#include "bfs/OpenDisposition.hpp"
#include "bfs/detail/DetailBFS.hpp"
#include "bfs/detail/DetailFileBlock.hpp"

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
        mutable boost::iostreams::stream_offset m_positionBeforeWrite;
        OpenDisposition m_openDisposition;

    };

}

#endif // BFS_FILE_BLOCK_HPP__
