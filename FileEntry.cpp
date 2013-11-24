#include "DetailBFS.hpp"
#include "DetailFileBlock.hpp"
#include "FileEntry.hpp"

namespace bfs
{
	FileEntry::FileEntry(std::string const &imagePath, uint64_t const totalBlocks, std::string const &name)
		: m_imagePath(imagePath)
		, m_totalBlocks(totalBlocks)
		, m_name(name)
		, m_fileSize(0)
		, m_fileBlocks()
		, m_buffer()
		, m_currentBlock(0)
	{
		// make the very first file block. Find out two available blocks
		// for this 'this' and the 'next'. Note if we end up not needing the next block,
		// we can overwrite it. Note, also set the size to block size which again
		// can be set in the block if it ends up being less
    	std::fstream stream(m_imagePath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
		newWritableFileBlock(stream);
		stream.close();
	}

	FileEntry::FileEntry(std::string const &imagePath, uint64_t const totalBlocks, uint64_t const startBlock)
		: m_imagePath(imagePath)
		, m_totalBlocks(totalBlocks)
		, m_name()
		, m_fileSize(0)
		, m_fileBlocks()
		, m_buffer()
		, m_currentBlock(startBlock)
	{
		// create the very first block making up file
		uint64_t const extraOffset = detail::MAX_FILENAME_LENGTH;
        FileBlock block(m_imagePath,
        		  	    m_totalBlocks,
        		  	    m_currentBlock);

        // retrieve the file name
        std::vector<char> name;
        block.read(&name.front(), detail::MAX_FILENAME_LENGTH);
        char byte = name[0];
        while(byte != '\0') {
        	m_name.push_back(byte);
        	++byte;
        }

        // ensure that during subseuqnet writes, filename is skipped
        block.setExtraOffset(extraOffset);

        // update file size which also takes in to account name length
        m_fileSize += detail::MAX_FILENAME_LENGTH;

        // store block
        m_fileBlocks.push_back(block);
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

    std::streamsize
    FileEntry::readCurrentBlockBytes()
    {
    	int index = m_fileBlocks.size() - 1;
    	uint32_t size =  m_fileBlocks[index].getDataBytesWritten();
    	m_buffer.resize(size);
    	(void)m_fileBlocks[index].read((char*)&m_buffer.front(), size);
    	if(m_currentBlock != m_fileBlocks[index].getNextIndex()) {
    		m_currentBlock = m_fileBlocks[index].getNextIndex();
			FileBlock block(m_imagePath,
							m_totalBlocks,
							m_currentBlock);
			m_fileBlocks.push_back(block);
    	}
    	return size;
    }

	std::streamsize
	FileEntry::read(char* s, std::streamsize n)
	{
		// read block data
		uint32_t read(0);
		while(read < n) {
			read += readCurrentBlockBytes();
			uint64_t offset(0);
			for(int b = 0; b < read; ++b) {
				s[offset + b] = m_buffer[b];
			}
			offset += read;
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

	void
	FileEntry::writeBufferedDataToBlock(uint32_t const bytes)
	{
		int index = m_fileBlocks.size() - 1;
		m_fileBlocks[index].write((char*)&m_buffer.front(), detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META);
		m_buffer.clear();
		std::fstream stream(m_imagePath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
		detail::updateVolumeBitmapWithOne(stream, m_currentBlock, m_totalBlocks);
		newWritableFileBlock(stream);
		stream.close();
	}

    void
    FileEntry::bufferByteForWriting(char const byte)
    {
    	m_buffer.push_back(byte);
    	if(m_buffer.size() == detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META) {
    		writeBufferedDataToBlock(detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META);
    	} else if(byte == '\0') {
			writeBufferedDataToBlock(m_buffer.size());
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
		return off;
	}
}
