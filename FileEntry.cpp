#include "BFSImageStream.hpp"
#include "DetailBFS.hpp"
#include "DetailFileBlock.hpp"
#include "FileEntry.hpp"

namespace bfs
{
    // for writing a brand new entry where start block isn't known
    FileEntry::FileEntry(std::string const &imagePath, uint64_t const totalBlocks, std::string const &name)
        : m_imagePath(imagePath)
        , m_totalBlocks(totalBlocks)
        , m_name(name)
        , m_fileSize(0)
        , m_fileBlocks()
        , m_buffer()
        , m_currentBlock(0)
        , m_startBlock(0)
        , m_blockIndex(0)
        , m_writeMode(AppendOrOverwrite::Append)
    {
    }

    // for appending or overwriting
    FileEntry::FileEntry(std::string const &imagePath,
                         uint64_t const totalBlocks,
                         std::string const &name,
                         uint64_t const startBlock,
                         AppendOrOverwrite const appendOrOverwrite)
        : m_imagePath(imagePath)
        , m_totalBlocks(totalBlocks)
        , m_name(name)
        , m_fileSize(0)
        , m_fileBlocks()
        , m_buffer()
        , m_currentBlock(startBlock)
        , m_startBlock(startBlock)
        , m_blockIndex(0)
        , m_writeMode(appendOrOverwrite)
    {
        // store all file blocks associated with file in container
        // also updates the file size as it does this
        setBlocks();

        // essentially seek right to the very last block
        m_blockIndex = m_fileBlocks.size() - 1;

        // update the starting write position of the end block to how many
        // bytes have been written to it so far so that we start from this
        // position when appending extra bytes
        this->seek(0, std::ios_base::end);
    }

    // for reading
    FileEntry::FileEntry(std::string const &imagePath, uint64_t const totalBlocks, uint64_t const startBlock)
        : m_imagePath(imagePath)
        , m_totalBlocks(totalBlocks)
        , m_name()
        , m_fileSize(0)
        , m_fileBlocks()
        , m_buffer()
        , m_currentBlock(startBlock)
        , m_startBlock(startBlock)
        , m_blockIndex(0)
        , m_writeMode(AppendOrOverwrite::Append)
    {
        // store all file blocks associated with file in container
        // also updates the file size as it does this
        setBlocks();
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
    FileEntry::getCurrentBlockIndex()
    {
        if (m_fileBlocks.empty()) {
            checkAndCreateWritableFileBlock();
            m_startBlock = m_currentBlock;
        }
        return m_currentBlock;
    }

    uint64_t
    FileEntry::getStartBlockIndex() const
    {
        if (m_fileBlocks.empty()) {
            checkAndCreateWritableFileBlock();
            m_startBlock = m_currentBlock;
        } else {
            m_startBlock = m_fileBlocks[0].getIndex();
        }
        return m_startBlock;
    }

    std::streamsize
    FileEntry::readCurrentBlockBytes()
    {

        uint32_t size =  m_fileBlocks[m_blockIndex].getDataBytesWritten();

        std::vector<uint8_t>().swap(m_buffer);
        m_buffer.resize(size);
        (void)m_fileBlocks[m_blockIndex].read((char*)&m_buffer.front(), size);

        if (m_blockIndex + 1 < m_fileBlocks.size()) {
            ++m_blockIndex;
            m_currentBlock = m_fileBlocks[m_blockIndex].getIndex();
        }

        return size;
    }

    std::streamsize
    FileEntry::read(char* s, std::streamsize n)
    {
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
        return n;
    }

    void FileEntry::newWritableFileBlock() const
    {
        bfs::BFSImageStream stream(m_imagePath, std::ios::in | std::ios::out | std::ios::binary);
        std::vector<uint64_t> firstAndNext = detail::getNAvailableBlocks(stream, 2, m_totalBlocks);
        stream.close();
        FileBlock block(m_imagePath, m_totalBlocks, firstAndNext[0], firstAndNext[0]);
        m_fileBlocks.push_back(block);
        m_blockIndex = m_fileBlocks.size() - 1;
        m_currentBlock = firstAndNext[0];
    }

    void FileEntry::setBlocks()
    {
        // find very first block
        FileBlock block(m_imagePath,
                        m_totalBlocks,
                        m_currentBlock,
                        m_writeMode);

        uint64_t nextBlock = block.getNextIndex();

        m_fileSize += block.getDataBytesWritten();

        m_fileBlocks.push_back(block);

        // seek to the very end block
        while (nextBlock != m_currentBlock) {

            m_currentBlock = nextBlock;
            FileBlock newBlock(m_imagePath,
                               m_totalBlocks,
                               m_currentBlock,
                               m_writeMode);
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

    void
    FileEntry::setNextOfLastBlockToIndexOfNewBlock() const
    {
        uint64_t lastIndex(m_blockIndex - 1);
        m_fileBlocks[lastIndex].setNext(m_currentBlock);
    }

    bool
    FileEntry::currentBlockHasAvailableSpace() const
    {
        uint32_t const bytesWritten = getBytesWrittenSoFarToCurrentFileBlock();

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
            if(m_writeMode == AppendOrOverwrite::Overwrite) {

                // if the reported stream position in the block is less that
                // the block's total capacity, then we don't create a new block
                // we simple overwrite
                if(m_fileBlocks[m_blockIndex].tell() < detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META) {
                    return;
                }

            }
            newWritableFileBlock();

            // update next index of last block
            setNextOfLastBlockToIndexOfNewBlock();

            return;
        }

    }

    uint32_t
    FileEntry::getBytesWrittenSoFarToCurrentFileBlock() const
    {
        if (!m_fileBlocks.empty()) {
            return m_fileBlocks[m_blockIndex].getDataBytesWritten();
        }
        return uint32_t(0);
    }

    uint32_t
    FileEntry::getInitialBytesWrittenToCurrentFileBlock() const
    {
        if (!m_fileBlocks.empty()) {
            return m_fileBlocks[m_blockIndex].getInitialDataBytesWritten();
        }
        return uint32_t(0);
    }

    void
    FileEntry::bufferByteForWriting(char const byte)
    {
        m_buffer.push_back(byte);

        // make a new block to write to. Not necessarily the case that
        // we want a new file block if in append mode. Won't be in append
        // mode if no data bytes have yet been written
        checkAndCreateWritableFileBlock();


        // if the buffer is full, then write
        uint32_t initialBytesWritten(getInitialBytesWrittenToCurrentFileBlock());

        if (m_buffer.size() == (detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META)
            - initialBytesWritten) {

            // write the data
            writeBufferedDataToBlock((detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META)
                                     - initialBytesWritten);

            m_fileBlocks[m_blockIndex].registerBlockWithVolumeBitmap();
        }
    }

    std::streamsize
    FileEntry::write(const char* s, std::streamsize n)
    {
        for (int i = 0; i < n; ++i) {

            // if in overwrite mode, filesize won't be updated
            // since we're simply overwriting bytes that already exist
            // NOTE: need to fix for when we start increasing size of file at end
            if (m_writeMode == AppendOrOverwrite::Append) {
                ++m_fileSize;
            }

            bufferByteForWriting(s[i]);
        }

        return n;
    }

    boost::iostreams::stream_offset
    FileEntry::seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way)
    {
        // reset any offset values to zero. An offset value determines
        // an offset point within a file block from which reading or writing
        // or appending etc. starts
        std::vector<FileBlock>::iterator it = m_fileBlocks.begin();
        for (; it != m_fileBlocks.end(); ++it) {
            it->seek(0);
        }

        // if at end just seek right to end and don't do anything else
        if (way == std::ios_base::end) {
            m_blockIndex = m_fileBlocks.size() - 1;
            uint32_t blockPosition = m_fileBlocks[m_blockIndex].getDataBytesWritten();
            m_fileBlocks[m_blockIndex].seek(blockPosition);
            return off;
        }

        // find what file block the offset would relate to and set extra offset in file block
        // to that position
        uint64_t const blockSize = detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META;
        uint64_t casted = uint64_t(off);
        uint64_t const leftOver = casted % blockSize;
        uint64_t block = 0;
        uint32_t blockPosition = 0;
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

        // update block where we start reading/writing from
        m_blockIndex = block;

        // check bounds and error if too big
        if (m_blockIndex >= m_fileBlocks.size()) {
            return -1; // fail
        } else {
            // set the position to seek to for given block
            // this will be the point from which we read or write
            m_fileBlocks[m_blockIndex].seek(blockPosition);
        }

        return off;
    }

    void
    FileEntry::flush()
    {
        checkAndCreateWritableFileBlock();
        writeBufferedDataToBlock(m_buffer.size());
        m_fileBlocks[m_blockIndex].registerBlockWithVolumeBitmap();
    }

    void
    FileEntry::unlink()
    {
        // loop over all file blocks and update the volume bitmap indicating
        // that block is no longer in use
        bfs::BFSImageStream stream(m_imagePath, std::ios::in | std::ios::out | std::ios::binary);
        std::vector<FileBlock>::iterator it = m_fileBlocks.begin();
        for (; it != m_fileBlocks.end(); ++it) {
            uint64_t blockIndex = it->getIndex();
            // false means to deallocate
            detail::updateVolumeBitmapWithOne(stream, blockIndex, m_totalBlocks, false);
        }
        m_fileSize = 0;
        stream.close();
    }
}
