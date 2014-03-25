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
*/

#ifndef TeaSafe_FILE_BLOCK_HPP__
#define TeaSafe_FILE_BLOCK_HPP__

#include "teasafe/TeaSafeImageStream.hpp"
#include "teasafe/CoreTeaSafeIO.hpp"
#include "teasafe/OpenDisposition.hpp"
#include "teasafe/detail/DetailTeaSafe.hpp"
#include "teasafe/detail/DetailFileBlock.hpp"

#include <vector>

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/positioning.hpp>
#include <iosfwd>
#include <string>

namespace teasafe
{

    class FileBlock
    {
      public:
        /**
         * @brief for when a file block needs to be written for the first time
         *        use this constructor
         * @param io the core teasafe io (path, blocks, password)
         * @param index the index of this file block
         * @param next the index of the next file block that makes up the file
         * @param openDisposition open mode
         */
        FileBlock(SharedCoreIO const &io,
                  uint64_t const index,
                  uint64_t const next,
                  OpenDisposition const &openDisposition);

        /**
         * @brief for when a file block needs to be read or written use this constructor
         * @param io the core teasafe io (path, blocks, password)
         * @param index the index of the file block
         * @param openDisposition open mode
         * @note  other params like size and next will be initialized when
         * the block is actually read
         */
        FileBlock(SharedCoreIO const &io,
                  uint64_t const index,
                  OpenDisposition const &openDisposition);

        /**
         * @brief  reads from the current file block
         * @param  buf the buffer to store the read data in
         * @param  n the number of bytes to read
         * @return the number of bytes read
         */
        std::streamsize read(char * const buf, std::streamsize const n) const;

        /**
         * @brief  writes to the current file block
         * @param  buf the data to write
         * @param  n the number of bytes to write
         * @return the number of bytes written
         */
        std::streamsize write(char const * const buf, std::streamsize const n) const;

        /**
         * @brief  seeks to a position in this file block
         * @param  off where to seek to given the seek-from type
         * @param  way the seek from type (beg, cur, or end)
         * @return the position offset to
         */
        boost::iostreams::stream_offset seek(boost::iostreams::stream_offset off,
                                             std::ios_base::seekdir way = std::ios_base::beg);

        /**
         * @brief  tell reports the current position of the read or write head
         * @return the current position of the read or write head
         */
        boost::iostreams::stream_offset tell() const;

        /**
         * @brief  gets the data bytes written to the block so far
         * @return the number of bytes written so far
         */
        uint32_t getDataBytesWritten() const;

        /**
         * @brief  accesses the number of bytes occupied by the block before any
         *         writing took place
         * @return the initial number of bytes written
         */
        uint32_t getInitialDataBytesWritten() const;

        /**
         * @brief  retrieves the pointer index to the next file block
         * @return the index of the next file block
         */
        uint64_t getNextIndex() const;

        /**
         * @brief  retrieves the index of the file block which signifies the block's
         *         position in the set of file blocks
         * @return the index of the file block
         */
        uint64_t getIndex() const;

        /**
         * @brief when the block has been used, it registers itself with the
         *        volume bitmap indicating that it's in use. The block can then
         *        only be re-used once its been deallocated
         */
        void registerBlockWithVolumeBitmap();

        /**
         * @brief when we want to set number of bytes written
         * useful when truncating
         * @param size the number of bytes written
         */
        void setSize(std::ios_base::streamoff size) const;

        void setSizeOnFlush() const;

        /**
         * @brief sets the next index of 'this'
         * @param nextIndex the next index to set this to
         */
        void setNextIndex(uint64_t nextIndex) const;

        /**
         * @brief this unregisters the block in the volume bitmap and sets the
         *        next block pointer to the current block pointer
         */
        void unlink();

      private:

        FileBlock();

        /**
         * @brief sets number of bytes written
         * @param size
         */
        void doSetSize(TeaSafeImageStream &stream, std::ios_base::streamoff size) const;

        /**
         * @brief sets the next index of the block
         * @param stream the image stream to operate over
         * @param nextIndex the next index to set next of this to
         */
        void doSetNextIndex(TeaSafeImageStream &stream, uint64_t nextIndex) const;

        SharedCoreIO m_io;
        uint64_t m_index;
        mutable uint32_t m_bytesWritten;
        mutable uint32_t m_initialBytesWritten;
        mutable uint64_t m_next;
        mutable uint64_t m_offset;
        mutable boost::iostreams::stream_offset m_seekPos;
        mutable boost::iostreams::stream_offset m_positionBeforeWrite;
        OpenDisposition m_openDisposition;

        // an optimization so that size can be written at end
        mutable uint64_t m_bytesToWriteOnFlush;

    };

}

#endif // TeaSafe_FILE_BLOCK_HPP__
