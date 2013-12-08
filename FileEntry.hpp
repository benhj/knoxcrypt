#ifndef BFS_FILE_ENTRY_HPP__
#define BFS_FILE_ENTRY_HPP__

#include "FileBlock.hpp"

#include <vector>

#include <iosfwd>                           // streamsize, seekdir
#include <boost/iostreams/categories.hpp>   // seekable_device_tag
#include <boost/iostreams/positioning.hpp>  // stream_offset

#include <string>
#include <fstream>

namespace bfs
{

    class FileEntry
    {
    public:
    	/**
    	 * @brief when writing a file this constructor should be used
    	 * @param imagePath the path of the bfs image
    	 * @param totalBlocks the total number of blocks in the bfs
    	 * @param name the name of the file entry
    	 */
        FileEntry(std::string const &imagePath, uint64_t const totalBlocks, std::string const &name);

        /**
         * @brief when appending to the end of a file this constructor should be used
         * @param imagePath the path of the bfs image
         * @param totalBlocks the total number of blocks in the bfs
         * @param name the name of the file entry
         * @param block the starting block of the file entry
         */
        FileEntry(std::string const &imagePath,
                  uint64_t const totalBlocks,
                  std::string const &name,
                  uint64_t const startBlock);

        /**
         * @brief when reading a file this constructor should be used
         * @param imagePath the path of the bfs image
         * @param totalBlocks the total number of blocks in the bfs
         * @param startBlock the starting block of the file to be read
         */
        FileEntry(std::string const &imagePath, uint64_t const totalBlocks, uint64_t const startBlock);

        typedef char                 				   char_type;
        typedef boost::iostreams::seekable_device_tag  category;

        std::string filename() const;

        uint64_t fileSize() const;

        uint64_t getCurrentBlockIndex() const;

        uint64_t getStartBlockIndex() const;

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
         * @brief flushes any remaining data
         */
        void flush();


    private:

        // the path of the bfs image
        std::string m_imagePath;

        // the total number of file blocks making up with bfs
        uint64_t m_totalBlocks;

        // the name of the file entry
        std::string m_name;

        // the size of the file
        uint64_t m_fileSize;

        // the file blocks making up the file
        std::vector<FileBlock> m_fileBlocks;

        // a buffer used for storing chunks of data
        std::vector<uint8_t> m_buffer;

        // the index of the current file block being read from or written to
        // note, this is the position of the block in the bfs
        uint64_t m_currentBlock;

        // the start file block index
        uint64_t m_startBlock;

        // the index of the block in the actual blocks container;
        // in comparison to m_currentBlock, this is where the block
        // exists in m_fileBlocks
        uint64_t m_blockIndex;

        /**
         * @brief buffers a byte for writing
         * @param byte the byte to buffer for writing
         */
        void bufferByteForWriting(char const byte);

        /**
         * @brief creates and pushes back a new file block for writing
         */
        void newWritableFileBlock();

        /**
         * @brief when appending, set all blocks in the block list
         * @note also updates file size as it seeks to end block
         * @param stream the bfs image stream
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
         * @brief gets the number of bytes written already to last file block
         * helpful when in append mode
         * @return bytes written
         */
        uint32_t getBytesWrittenInLastFileBlock() const;

        /**
         * @brief will build a new file block for writing to if there are
         * no file blocks or if there are file blocks and it is determined
         * that we're not in append mode
         */
        void checkAndCreateWritableFileBlock();

        /**
         * @brief sets the next index of the last block to that of the new block
         */
        void setNextOfLastBlockToIndexOfNewBlock();
};

}

#endif // BFS_BFS_FILE_ENTRY_HPP__
