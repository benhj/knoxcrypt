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

#ifndef TeaSafe_FILE_ENTRY_HPP__
#define TeaSafe_FILE_ENTRY_HPP__

#include "teasafe/CoreTeaSafeIO.hpp"
#include "teasafe/FileBlock.hpp"
#include "teasafe/OpenDisposition.hpp"

#include <vector>

#include <iosfwd>                           // streamsize, seekdir

#include <string>

#include <deque>

namespace teasafe
{

    class FileEntry
    {
      public:
        /**
         * @brief when creating a new file this constructor should be used
         * @param io the core teasafe io (path, blocks, password)
         * @param name the name of the file entry
         * @param enforceStartBlock true if start block should be set
         */
        FileEntry(CoreTeaSafeIO const &io, std::string const &name, bool const enforceStartBlock = false);

        /**
         * @brief when reading or appending or overwriting to the end of a file
         * this constructor should be used
         * @param io the core teasafe io (path, blocks, password)
         * @param name the name of the file entry
         * @param block the starting block of the file entry
         * @param openDisposition open mode
         */
        FileEntry(CoreTeaSafeIO const &io,
                  std::string const &name,
                  uint64_t const startBlock,
                  OpenDisposition const &openDisposition);

        typedef char                                   char_type;
        typedef boost::iostreams::seekable_device_tag  category;

        std::string filename() const;

        uint64_t fileSize() const;

        uint64_t getCurrentVolumeBlockIndex();

        uint64_t getStartVolumeBlockIndex() const;

        /**
         * @brief truncates a file to new size
         * @param newSize the new fileSize
         */
        void truncate(std::ios_base::streamoff newSize);

        /**
         * @brief for reading
         * @param s
         * @param n
         * @return
         */
        std::streamsize read(char* s, std::streamsize n);

        /**
         * @brief for writing
         * @param s
         * @param n
         * @return
         */
        std::streamsize write(const char* s, std::streamsize n);

        /**
         * @brief for seeking
         * @param off
         * @param way
         * @return
         */
        boost::iostreams::stream_offset seek(boost::iostreams::stream_offset off,
                                             std::ios_base::seekdir way = std::ios_base::beg);

        /**
         * @brief indicates the stream position
         * @return the current stream position
         */
        boost::iostreams::stream_offset tell() const;

        /**
         * @brief flushes any remaining data
         */
        void flush();

        /**
         * @brief deallocates blocks associated with this file entry; used
         * in conjunction with deleting the file
         */
        void unlink();

      private:

        // the core teasafe io (path, blocks, password)
        CoreTeaSafeIO m_io;

        // the name of the file entry
        std::string m_name;

        // whether to enforce the starting file block (potentially dangerous!)
        mutable bool m_enforceStartBlock;

        // the size of the file
        uint64_t m_fileSize;

        // the file blocks making up the file
        mutable std::vector<FileBlock> m_fileBlocks;

        // a buffer used for storing chunks of data
        std::vector<uint8_t> m_buffer;

        // the index of the current file block being read from or written to
        // note, this is the position of the block in the teasafe
        mutable uint64_t m_currentVolumeBlock;

        // the start file block index
        mutable uint64_t m_startVolumeBlock;

        // the index of the block in the actual blocks container;
        // in comparison to m_currentBlock, this is where the block
        // exists in m_fileBlocks
        mutable int64_t m_blockIndex;

        // open mode
        OpenDisposition m_openDisposition;

        // the current 'stream position' of file entry
        std::streamoff m_pos;

        // make a pool to hold on to indices
        mutable std::deque<uint64_t> m_blockIndexPool;

        /**
         * @brief buffers a byte for writing
         * @param byte the byte to buffer for writing
         */
        void bufferByteForWriting(char const byte);

        /**
         * @brief creates and pushes back a new file block for writing
         */
        void newWritableFileBlock() const;

        /**
         * @brief when appending, set all blocks in the block list
         * @note also updates file size as it seeks to end block
         * @param stream the teasafe image stream
         */
        void setBlocks();

        /**
         * @brief writes data to file block
         * @param bytes the number of bytes to write
         */
        void writeBufferedDataToBlock(uint32_t const bytes);

        /**
         * @brief reads bytes from the block in to buffer
         * @return the number of bytes read
         */
        std::streamsize readCurrentBlockBytes();


        /**
         * @brief will build a new file block for writing to if there are
         * no file blocks or if there are file blocks and it is determined
         * that we're not in append mode
         */
        void checkAndCreateWritableFileBlock() const;

        /**
         * @brief sets the next index of the last block to that of the new block
         */
        void setNextOfLastBlockToIndexOfNewBlock() const;

        /**
         * @brief used in the context of discovering if currently set block
         * has enough space to write more data to
         * @return true if space availble, false otherwise
         */
        bool currentBlockHasAvailableSpace() const;
    };

}

#endif // TeaSafe_TeaSafe_FILE_ENTRY_HPP__
