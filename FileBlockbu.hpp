#ifndef BFS_FILE_BLOCK_HPP__
#define BFS_FILE_BLOCK_HPP__

#include "DetailBFS.hpp"

#include <vector>

#include <boost/iostreams/categories.hpp>  // sink_tag

#include <boost/shared_ptr.hpp>

#include <iosfwd>                          // streamsize
#include <string>
#include <fstream>

class FileBlock;

typedef boost::shared_ptr<FileBlock> SharedFileBlock;

namespace bfs
{

    class FileBlock
    {
    public:
    	/**
    	 * @brief for when a file block needs to be written use this constructor
    	 * @param imagePath the path of the bfs image
    	 * @param totalBlocks the total number of blocks in the fs image
    	 * @param index the index of this file block
    	 * @param next the index of the next file block that makes up the file
    	 */
        FileBlock(std::string const &imagePath,
        		  uint64_t const totalBlocks,
        	      uint64_t const index,
        		  uint64_t const next)
    		: m_imagePath(imagePath)
    		, m_totalBlocks(totalBlocks)
    	    , m_index(index)
    		, m_size(detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META)
    		, m_next(next)
    		, m_offset(0)
        {
        	// set m_offset
        	std::fstream stream(m_imagePath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
        	m_offset = detail::getOffsetOfFileBlock(m_index, m_totalBlocks);
            std::cout<<"off write "<<m_offset<<std::endl;
        	(void)stream.seekp(m_offset);

        	// write m_size (set to default can later be updated if necessary)
        	uint8_t sizeDat[4];
        	detail::convertInt32ToInt4Array(m_size, sizeDat);
        	(void)stream.write((char*)sizeDat, 4);

        	std::cout<<"write block constructor tellp: "<<stream.tellp()<<std::endl;

        	// write m_next
        	uint8_t nextDat[8];
        	detail::convertInt64ToInt8Array(m_next, nextDat);
        	assert(m_next == detail::convertInt8ArrayToInt64(nextDat));
        	(void)stream.write((char*)nextDat, 8);
        	std::cout<<"write block constructor "<<index<<"\t"<<m_next<<std::endl;
        	stream.flush();


        	stream.seekg(m_offset + 4);
        	uint8_t nextB[8];
        	(void)stream.read((char*)nextB, 8);
        	uint64_t indexRec  = detail::convertInt8ArrayToInt64(nextB);
        	assert(indexRec == m_next);
        	std::cout<<"indexRec: "<<indexRec<<std::endl;
        	stream.close();
        }

        /**
         * @brief for when a file block needs to be read use this constructor
         * @param index the index of the file block
         * @note other params like size and next will be initialized when
         * the block is actually read
         */
        FileBlock(std::string const &imagePath,
        		  uint64_t const totalBlocks,
        		  uint64_t const index)
    		: m_imagePath(imagePath)
            , m_totalBlocks(totalBlocks)
    	    , m_index(index)
    		, m_size(0)
    		, m_next(0)
    		, m_offset(0)
        {
        	// set m_offset
			std::fstream stream(m_imagePath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
			m_offset = detail::getOffsetOfFileBlock(m_index, m_totalBlocks);
            std::cout<<"off read "<<m_offset<<std::endl;
			(void)stream.seekg(m_offset);

			// read m_size
        	uint8_t sizeDat[4];
        	(void)stream.read((char*)sizeDat, 4);
        	m_size = detail::convertInt4ArrayToInt32(sizeDat);

            std::cout<<"read block constructor for index "<<index<<"\tm_size: "<<m_size<<std::endl;
            std::cout<<"read block constructor tellg: "<<stream.tellg()<<std::endl;

        	// read m_next
            uint8_t nextDat[8];
            (void)stream.read((char*)nextDat, 8);
            m_next = detail::convertInt8ArrayToInt64(nextDat);

            std::cout<<"read block constructor index and next "<<index<<"\t"<<m_next<<std::endl;


        	stream.close();

        }

        /**
         * @brief in case needing to skip some bytes
         * @param extraOffset the amount to update m_offset by
         */
        void setExtraOffset(uint64_t const extraOffset)
        {
        	m_offset += extraOffset;
        }

        std::streamsize read(char * const buf, std::streamsize const n) const
        {
        	// open the image stream for reading
        	std::fstream stream(m_imagePath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
			(void)stream.seekg(m_offset + detail::FILE_BLOCK_META);
			(void)stream.read((char*)buf, n);
			stream.close();
			return n;
        }

        std::streamsize write(char const * const buf, std::streamsize const n) const
        {
        	// open the image stream for writing
        	std::fstream stream(m_imagePath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
			(void)stream.seekp(m_offset + detail::FILE_BLOCK_META);
			(void)stream.write((char*)buf, n);

			// check if we need to update m_size and m_next
			if(n < detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META) {
				m_size = uint32_t(n);
				m_next = m_index;
				std::cout<<"set m_next to "<<m_next<<std::endl;
	        	(void)stream.seekp(m_offset);

	        	std::cout<<"tellg after correctly setting m_next: "<<stream.tellp()<<std::endl;

	        	// update m_size
	        	uint8_t sizeDat[4];
	        	detail::convertInt32ToInt4Array(m_size, sizeDat);
	        	(void)stream.write((char*)sizeDat, 4);

	        	// update m_next
	        	uint8_t nextDat[8];
	        	detail::convertInt64ToInt8Array(m_next, nextDat);
	        	(void)stream.write((char*)nextDat, 8);
			}

            stream.flush();
			stream.close();

			return n;
        }

        uint32_t getDataBytesWritten() const
        {
        	return m_size;
        }

        uint64_t getNextIndex() const
        {
        	return m_next;
        }

        uint64_t getBlockOffset() const
        {
        	return m_offset;
        }

    private:

        std::string m_imagePath;
        uint64_t m_totalBlocks;
        uint64_t m_index;
        mutable uint32_t m_size;
        mutable uint64_t m_next;
        mutable uint64_t m_offset;

    };

}

#endif // BFS_FILE_BLOCK_HPP__
