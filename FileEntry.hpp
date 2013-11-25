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
        boost::iostreams::stream_offset seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way);

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

        // theindex of the current file block being read from or written to
        uint64_t m_currentBlock;

        /**
         * @brief buffers a byte for writing
         * @param byte the byte to buffer for writing
         */
        void bufferByteForWriting(char const byte);

        /**
         * @brief creates and pushes back a new file block for writing
         * @param stream the bfs image stream
         */
        void newWritableFileBlock(std::fstream &stream);

        /**
         * @brief when appending, set all blocks in the block list
         * @note also updates file size as it seeks to end block
         * @param stream the bfs image stream
         */
        void setBlocks(std::fstream &stream);

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
};

}

#endif // BFS_BFS_FILE_ENTRY_HPP__
