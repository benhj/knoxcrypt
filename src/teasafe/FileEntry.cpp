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

#include "teasafe/TeaSafeImageStream.hpp"
#include "teasafe/FileEntry.hpp"
#include "teasafe/FileEntryException.hpp"
#include "teasafe/detail/DetailTeaSafe.hpp"
#include "teasafe/detail/DetailFileBlock.hpp"

#include <stdexcept>

namespace teasafe
{
    // for writing a brand new entry where start block isn't known
    FileEntry::FileEntry(SharedCoreIO const &io, std::string const &name, bool const enforceStartBlock)
        : m_io(io)
        , m_name(name)
        , m_enforceStartBlock(enforceStartBlock)
        , m_fileSize(0)
        , m_fileBlocks()
        , m_buffer()
        , m_currentVolumeBlock(0)
        , m_startVolumeBlock(0)
        , m_blockIndex(0)
        , m_openDisposition(OpenDisposition::buildAppendDisposition())
        , m_pos(0)
    {
    }

    // for appending or overwriting
    FileEntry::FileEntry(SharedCoreIO const &io,
                         std::string const &name,
                         uint64_t const startBlock,
                         OpenDisposition const &openDisposition)
        : m_io(io)
        , m_name(name)
        , m_enforceStartBlock(false)
        , m_fileSize(0)
        , m_fileBlocks()
        , m_buffer()
        , m_currentVolumeBlock(startBlock)
        , m_startVolumeBlock(startBlock)
        , m_blockIndex(0)
        , m_openDisposition(openDisposition)
        , m_pos(0)
    {
        // store all file blocks associated with file in container
        // also updates the file size as it does this
        setBlocks();

        // essentially seek right to the very last block
        m_blockIndex = m_fileBlocks.size() - 1;

        // by default seek to 0 position
        this->seek(0);

        // set up for specific write-mode
        if (m_openDisposition.readWrite() != ReadOrWriteOrBoth::ReadOnly) {

            // if in trunc, unlink
            if (m_openDisposition.trunc() == TruncateOrKeep::Truncate) {
                this->unlink();

            } else {
                // only if in append mode do we seek to end.
                if (m_openDisposition.append() == AppendOrOverwrite::Append) {
                    this->seek(0, std::ios::end);
                }
            }
        }
    }

    std::string
    FileEntry::filename() const
    {
        return m_name;
    }

    uint64_t
    FileEntry::fileSize() const
    {
        return m_fileSize;
    }

    uint64_t
    FileEntry::getCurrentVolumeBlockIndex()
    {
        if (m_fileBlocks.empty()) {
            checkAndCreateWritableFileBlock();
            m_startVolumeBlock = m_currentVolumeBlock;
        }
        return m_currentVolumeBlock;
    }

    uint64_t
    FileEntry::getStartVolumeBlockIndex() const
    {
        if (m_fileBlocks.empty()) {
            checkAndCreateWritableFileBlock();
            m_startVolumeBlock = m_currentVolumeBlock;
        } else {
            m_startVolumeBlock = m_fileBlocks[0].getIndex();
        }
        return m_startVolumeBlock;
    }

    std::streamsize
    FileEntry::readCurrentBlockBytes()
    {

        // need to take into account the currently seeked-to position and
        // subtract that because we then only want to read from the told position
        uint32_t size =  m_fileBlocks[m_blockIndex].getDataBytesWritten() -
            m_fileBlocks[m_blockIndex].tell();

        std::vector<uint8_t>().swap(m_buffer);
        m_buffer.resize(size);
        (void)m_fileBlocks[m_blockIndex].read((char*)&m_buffer.front(), size);

        if (m_blockIndex + 1 < m_fileBlocks.size()) {
            ++m_blockIndex;
            m_currentVolumeBlock = m_fileBlocks[m_blockIndex].getIndex();
        }

        return size;
    }

    void FileEntry::newWritableFileBlock() const
    {
        teasafe::TeaSafeImageStream stream(m_io, std::ios::in | std::ios::out | std::ios::binary);

        // note building a new block to write to should always be in append mode
        uint64_t id;

        // if the starting file block is enforced, set to root block specified in m_io
        if (m_enforceStartBlock) {
            m_enforceStartBlock = false;
            id = m_io->rootBlock;
        } else {
            id = *(detail::getNextAvailableBlock(stream, m_io->blocks));
        }
        //uint64_t id = *detail::getNextAvailableBlock(stream, m_io->blocks);
        stream.close();
        FileBlock block(m_io, id, id, teasafe::OpenDisposition::buildAppendDisposition());

        m_fileBlocks.push_back(block);
        m_blockIndex = m_fileBlocks.size() - 1;
        m_currentVolumeBlock = id;
        m_fileBlocks[m_blockIndex].registerBlockWithVolumeBitmap();
    }

    void FileEntry::setBlocks()
    {
        // find very first block
        FileBlock block(m_io,
                        m_currentVolumeBlock,
                        m_openDisposition);

        uint64_t nextBlock = block.getNextIndex();

        m_fileSize += block.getDataBytesWritten();

        m_fileBlocks.push_back(block);

        // seek to the very end block
        while (nextBlock != m_currentVolumeBlock) {

            m_currentVolumeBlock = nextBlock;
            FileBlock newBlock(m_io,
                               m_currentVolumeBlock,
                               m_openDisposition);

            nextBlock = newBlock.getNextIndex();

            m_fileSize += newBlock.getDataBytesWritten();
            m_fileBlocks.push_back(newBlock);
        }
    }

    void
    FileEntry::writeBufferedDataToBlock(uint32_t const bytes)
    {
        m_fileBlocks[m_blockIndex].write((char*)&m_buffer.front(), bytes);
        std::vector<uint8_t>().swap(m_buffer);
    }

    bool
    FileEntry::currentBlockHasAvailableSpace() const
    {
        // use tell to get bytes written so far as the read/write head position
        // is always updates after reads/writes
        uint32_t const bytesWritten = m_fileBlocks[m_blockIndex].tell();

        if (bytesWritten < detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META) {
            return true;
        }
        return false;

    }

    void
    FileEntry::checkAndCreateWritableFileBlock() const
    {
        // first case no file blocks so absolutely need one to write to
        if (m_fileBlocks.empty()) {
            newWritableFileBlock();
            return;
        }

        // in this case the current block is exhausted so we need a new one
        if (!currentBlockHasAvailableSpace()) {

            // if in overwrite mode, maybe we want to overwrite current bytes
            if (m_openDisposition.append() == AppendOrOverwrite::Overwrite) {

                // if the reported stream position in the block is less that
                // the block's total capacity, then we don't create a new block
                // we simply overwrite
                if (m_fileBlocks[m_blockIndex].tell() < detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META) {
                    return;
                }

                // edge case; if right at the very end of the block, need to
                // iterate the block index and return if possible
                if (m_fileBlocks[m_blockIndex].tell() == detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META) {
                    if (m_blockIndex < m_fileBlocks.size() - 1) {
                        ++m_blockIndex;
                        return;
                    }
                }
            }
            newWritableFileBlock();

            return;
        }

    }

    void
    FileEntry::bufferByteForWriting(char const byte)
    {
        m_buffer.push_back(byte);

        // make a new block to write to. Not necessarily the case that
        // we want a new file block if in append mode. Won't be in append
        // mode if no data bytes have yet been written
        checkAndCreateWritableFileBlock();

        // if given the stream position no more bytes can be written
        // then write out buffer
        uint32_t streamPosition(m_fileBlocks[m_blockIndex].tell());

        if (m_buffer.size() == (detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META)
            - streamPosition) {

            // write the data
            writeBufferedDataToBlock((detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META)
                                     - streamPosition);
        }
    }

    std::streamsize
    FileEntry::read(char* s, std::streamsize n)
    {

        if (m_openDisposition.readWrite() == ReadOrWriteOrBoth::WriteOnly) {
            throw FileEntryException(FileEntryError::NotReadable);
        }

        // read block data
        uint32_t read(0);
        uint64_t offset(0);
        while (read < n) {

            uint32_t count = readCurrentBlockBytes();

            // check that we don't read too much!
            if (read + count >= n) {
                count -= (read + count - n);
            }

            read += count;

            for (int b = 0; b < count; ++b) {
                s[offset + b] = m_buffer[b];
            }

            offset += count;
        }

        // update stream position
        m_pos += n;

        return n;
    }

    std::streamsize
    FileEntry::write(const char* s, std::streamsize n)
    {

        if (m_openDisposition.readWrite() == ReadOrWriteOrBoth::ReadOnly) {
            throw FileEntryException(FileEntryError::NotWritable);
        }

        for (int i = 0; i < n; ++i) {

            // if in overwrite mode, filesize won't be updated
            // since we're simply overwriting bytes that already exist
            // NOTE: need to fix for when we start increasing size of file at end

            // this is the fix!
            if (this->tell() == m_fileSize) {
                m_openDisposition = OpenDisposition::buildAppendDisposition();
            }

            if (m_openDisposition.append() == AppendOrOverwrite::Append) {
                ++m_fileSize;
            }

            bufferByteForWriting(s[i]);
        }

        // update stream position
        m_pos += n;

        return n;
    }

    void
    FileEntry::truncate(std::ios_base::streamoff newSize)
    {
        uint64_t blockCount = m_fileBlocks.size();

        // compute number of block required
        uint16_t const blockSize = detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META;

        // edge case
        if (newSize < blockSize) {
            m_fileBlocks[0].setSize(newSize);
            m_fileBlocks[0].setNextIndex(m_fileBlocks[0].getIndex());
            return;
        }

        boost::iostreams::stream_offset const leftOver = newSize % blockSize;

        boost::iostreams::stream_offset const roundedDown = newSize - leftOver;

        uint64_t blocksRequired = roundedDown / blockSize;

        // edge case
        if (leftOver == 0) {
            --blocksRequired;
            m_fileBlocks[blocksRequired].setSize(blockSize);
        } else {
            m_fileBlocks[blocksRequired].setSize(leftOver);
        }

        m_fileBlocks[blocksRequired].setNextIndex(m_fileBlocks[blocksRequired].getIndex());

        std::vector<FileBlock> tempBlocks(m_fileBlocks.begin(), m_fileBlocks.begin() + blocksRequired);
        tempBlocks.swap(m_fileBlocks);
    }

    typedef std::pair<int64_t, boost::iostreams::stream_offset> SeekPair;
    SeekPair
    getPositionFromBegin(boost::iostreams::stream_offset off)
    {
        // find what file block the offset would relate to and set extra offset in file block
        // to that position
        uint16_t const blockSize = detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META;
        boost::iostreams::stream_offset casted = off;
        boost::iostreams::stream_offset const leftOver = casted % blockSize;
        int64_t block = 0;
        boost::iostreams::stream_offset blockPosition = 0;
        if (off > blockSize) {
            if (leftOver > 0) {

                // round down casted
                casted -= leftOver;

                // set the position of the stream in block to leftOver
                blockPosition = leftOver;

                ++block;

            } else {
                blockPosition = 0;
            }

            // get exact number of blocks after round-down
            block = off / blockSize;

        } else {
            // offset is smaller than the first block so keep block
            // index at 0 and the position for the zero block at offset
            blockPosition = off;
        }
        return std::make_pair(block, blockPosition);
    }

    SeekPair
    getPositionFromEnd(boost::iostreams::stream_offset off, int64_t endBlockIndex,
                       boost::iostreams::stream_offset bytesWrittenToEnd)
    {
        // treat like begin and then 'inverse'
        SeekPair treatLikeBegin = getPositionFromBegin(abs(off));

        int64_t block = endBlockIndex - treatLikeBegin.first;
        boost::iostreams::stream_offset blockPosition = bytesWrittenToEnd - treatLikeBegin.second;

        if (blockPosition < 0) {
            uint16_t const blockSize = detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META;
            blockPosition = blockSize + blockPosition;
            --block;
        }

        return std::make_pair(block, blockPosition);

    }

    SeekPair
    getPositionFromCurrent(boost::iostreams::stream_offset off,
                           int64_t blockIndex,
                           boost::iostreams::stream_offset indexedBlockPosition)
    {

        // find what file block the offset would relate to and set extra offset in file block
        // to that position
        uint16_t const blockSize = detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META;
        boost::iostreams::stream_offset const casted = off;

        boost::iostreams::stream_offset addition = casted + indexedBlockPosition;

        if (addition >= 0 && addition <= blockSize) {
            return std::make_pair(blockIndex, addition);
        } else {

            boost::iostreams::stream_offset const leftOver = abs(addition) % blockSize;

            int64_t sumValue = 0;

            boost::iostreams::stream_offset roundedDown = addition - leftOver;

            if (abs(roundedDown) > (blockSize)) {
                sumValue = abs(roundedDown) / blockSize;

                // hacky bit to get working
                if ((addition < 0) && ((blockSize - leftOver) > indexedBlockPosition)) {
                    sumValue++;
                }
            } else {
                sumValue = 1;
            }

            uint16_t const theBlock = (addition < 0) ? (blockIndex - sumValue) :
                (blockIndex + sumValue);
            boost::iostreams::stream_offset offset = (addition < 0) ? (blockSize - leftOver) :
                leftOver;

            return std::make_pair(theBlock, offset);
        }
    }

    boost::iostreams::stream_offset
    FileEntry::seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way)
    {
        // reset any offset values to zero but only if not seeking from the current
        // position. When seeking from the current position, we need to keep
        // track of the original block offset

        if (way != std::ios_base::cur) {
            std::vector<FileBlock>::iterator it = m_fileBlocks.begin();
            for (; it != m_fileBlocks.end(); ++it) {
                it->seek(0);
            }
        }

        // if at end just seek right to end and don't do anything else
        SeekPair seekPair;
        if (way == std::ios_base::end) {

            size_t endBlock = m_fileBlocks.size() - 1;
            seekPair = getPositionFromEnd(off, endBlock,
                                          m_fileBlocks[endBlock].getDataBytesWritten());

        }

        // pass in the current offset, the current block and the current
        // block position. Note both of these latter two params will be 0
        // if seeking from the beginning

        if (way == std::ios_base::beg) {
            seekPair = getPositionFromBegin(off);
        }
        // seek relative to the current position
        if (way == std::ios_base::cur) {
            seekPair = getPositionFromCurrent(off, m_blockIndex,
                                              m_fileBlocks[m_blockIndex].tell());
        }

        // check bounds and error if too big
        if (seekPair.first >= m_fileBlocks.size() || seekPair.first < 0) {
            return -1; // fail
        } else {

            // update block where we start reading/writing from
            m_blockIndex = seekPair.first;

            // set the position to seek to for given block
            // this will be the point from which we read or write
            m_fileBlocks[m_blockIndex].seek(seekPair.second);

            switch (way) {
              case std::ios_base::cur:
                m_pos = m_pos + off;
                break;
              case std::ios_base::end:
                m_pos = m_fileSize + off;
                break;
              case std::ios_base::beg:
                m_pos = off;
                break;
              default:
                break;
            }

        }

        return off;
    }

    boost::iostreams::stream_offset
    FileEntry::tell() const
    {
        return m_pos;
    }

    void FileEntry::setBlockNextIndices()
    {
        // rather than set the index after each file block which *might*
        // slow things down, wait until end. Will hopefully make writing more
        // efficient
        std::vector<FileBlock>::iterator it = m_fileBlocks.begin();
        for (; it != m_fileBlocks.end() - 1; ++it) {
            it->setNextIndex((it + 1)->getIndex());
        }
    }

    void
    FileEntry::flush()
    {
        writeBufferedDataToBlock(m_buffer.size());

        // rather than set the index after each file block which *might*
        // slow things down, wait until end. Will hopefully make writing more
        // efficient
        setBlockNextIndices();
    }

    void
    FileEntry::unlink()
    {
        // loop over all file blocks and update the volume bitmap indicating
        // that block is no longer in use
        std::vector<FileBlock>::iterator it = m_fileBlocks.begin();
        for (; it != m_fileBlocks.end(); ++it) {
            it->unlink();
            m_io->freeBlocks++;
        }
        std::vector<FileBlock>().swap(m_fileBlocks);
        m_fileSize = 0;
    }
}
