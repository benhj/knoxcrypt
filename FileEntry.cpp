#include "DetailBFS.hpp"
#include "DetailFileBlock.hpp"
#include "FileEntry.hpp"

namespace bfs
{
    // for writing
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
	{
		// make the very first file block. Find out two available blocks
		// for this 'this' and the 'next'. Note if we end up not needing the next block,
		// we can overwrite it. Note, also set the size to block size which again
		// can be set in the block if it ends up being less
    	std::fstream stream(m_imagePath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
		newWritableFileBlock(stream);
		stream.close();
	}

	// for appending
    FileEntry::FileEntry(std::string const &imagePath,
              uint64_t const totalBlocks,
              std::string const &name,
              uint64_t const startBlock)
        : m_imagePath(imagePath)
        , m_totalBlocks(totalBlocks)
        , m_name(name)
        , m_fileSize(0)
        , m_fileBlocks()
        , m_buffer()
        , m_currentBlock(startBlock)
        , m_startBlock(startBlock)
    	, m_blockIndex(0)
    {
        // store all file blocks associated with file in container
        // also updates the file size as it does this
        std::fstream stream(m_imagePath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
        setBlocks(stream);
        stream.close();

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
	{
        // store all file blocks associated with file in container
        // also updates the file size as it does this
        std::fstream stream(m_imagePath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
        setBlocks(stream);
        stream.close();
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
    FileEntry::getCurrentBlockIndex() const
    {
        return m_currentBlock;
    }

    uint64_t
    FileEntry::getStartBlockIndex() const
    {
        return m_startBlock;
    }

    std::streamsize
    FileEntry::readCurrentBlockBytes()
    {

    	uint32_t size =  m_fileBlocks[m_blockIndex].getDataBytesWritten();

    	std::vector<uint8_t>().swap(m_buffer);
    	m_buffer.resize(size);
    	(void)m_fileBlocks[m_blockIndex].read((char*)&m_buffer.front(), size);

    	if(m_blockIndex + 1 < m_fileBlocks.size()) {
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
		while(read < n) {

		    uint32_t count = readCurrentBlockBytes();

		    // check that we don't read too much!
		    if(read + count >= n) {
		    	count -= (read + count - n);
		    }

			read += count;


			for(int b = 0; b < count; ++b) {
				s[offset + b] = m_buffer[b];
			}

			offset += count;
		}
		return n;
	}

	void FileEntry::newWritableFileBlock(std::fstream &stream)
	{
		std::vector<uint64_t> firstAndNext = detail::getNAvailableBlocks(stream, 2, m_totalBlocks);
		m_currentBlock = firstAndNext[0];
		FileBlock block(m_imagePath, m_totalBlocks, firstAndNext[0], firstAndNext[1]);
		m_fileBlocks.push_back(block);
	}

	void FileEntry::setBlocks(std::fstream &stream)
	{
	    // find very first block
        FileBlock block(m_imagePath,
                        m_totalBlocks,
                        m_currentBlock);

        uint64_t nextBlock = block.getNextIndex();
        m_fileSize += block.getDataBytesWritten();
        m_fileBlocks.push_back(block);

        // seek to the very end block
        while(nextBlock != m_currentBlock) {
            m_currentBlock = nextBlock;
            FileBlock newBlock(m_imagePath,
                                m_totalBlocks,
                                m_currentBlock);
            nextBlock = newBlock.getNextIndex();
            m_fileSize += newBlock.getDataBytesWritten();
            m_fileBlocks.push_back(newBlock);
        }

	}

	void
	FileEntry::writeBufferedDataToBlock(uint32_t const bytes)
	{
		int index = m_fileBlocks.size() - 1;
		m_fileBlocks[index].write((char*)&m_buffer.front(), bytes);
		std::vector<uint8_t>().swap(m_buffer);
		std::fstream stream(m_imagePath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
		detail::updateVolumeBitmapWithOne(stream, m_currentBlock, m_totalBlocks);
		newWritableFileBlock(stream);
		stream.close();
	}

    void
    FileEntry::bufferByteForWriting(char const byte)
    {
        m_buffer.push_back(byte);

        // check the buffer size and if it matches the space remaining in given
        // block, write to the block with data
    	if(m_buffer.size() == (detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META)) {
    	                       //m_fileBlocks[m_blockIndex].getDataBytesWritten())) {
    		writeBufferedDataToBlock(detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META);
    	}
    }

	std::streamsize
	FileEntry::write(const char* s, std::streamsize n)
	{
		for(int i = 0; i < n; ++i) {
			++m_fileSize;
			bufferByteForWriting(s[i]);
		}
		return n;
	}

	boost::iostreams::stream_offset
	FileEntry::seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way)
	{

	    // if at end jsut seek right to end and don't do anything else
	    if(way == std::ios_base::end) {
	        m_blockIndex = m_fileBlocks.size() - 1;
	        uint32_t blockPosition = m_fileBlocks[m_blockIndex].getDataBytesWritten();
	        m_fileBlocks[m_blockIndex].setExtraOffset(blockPosition);
	        return off;
	    }

		// find what file block the offset would relate to and set extra offset in file block
		// to that position
		uint64_t const blockSize = detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META;
		uint64_t casted = uint64_t(off);
		uint64_t const leftOver = casted % blockSize;
		uint64_t block = 0;
		uint32_t blockPosition = 0;
		if(off > blockSize) {
			if(leftOver > 0) {

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
		if(m_blockIndex >= m_fileBlocks.size()) {
			return -1; // fail
		} else {
			// set the position to seek to for given block
			// this will be the point from which we read or write
			m_fileBlocks[m_blockIndex].setExtraOffset(blockPosition);
		}
		return off;
	}

	void
	FileEntry::flush()
	{
	    writeBufferedDataToBlock(m_buffer.size());
	}
}
