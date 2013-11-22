#ifndef BFS_BFS_ENTRY_WRITER_HPP__
#define BFS_BFS_ENTRY_WRITER_HPP__

#include <boost/shared_ptr.hpp>

#include <vector>

#include <boost/iostreams/categories.hpp>  // sink_tag
#include <iosfwd>                          // streamsize
#include <string>
#include <fstream>


namespace bfs
{

    class BFSEntryWriter
    {
      public:

        typedef char                          char_type;
        typedef boost::iostreams::sink_tag    category;

        BFSEntryWriter(std::string const &bfsOutputPath,
                     std::string const &entryName,
                     uint64_t const fsize,
                     uint64_t const parentIndex);

        /**
         * @param buf the data to be written
         * @param n number of bytes to written
         * @return the number of bytes written
         */
        std::streamsize write(char const * const buf, std::streamsize const n);
        ~BFSEntryWriter();

      private:

        BFSEntryWriter();

        // the image path
        std::string const m_bfsOutputPath;

        // the name of the entry to add
        std::string const m_entryName;

        // size of the file to add
        uint64_t const m_fsize;

        // parent folder index (self if no parent)
        uint64_t const m_parentIndex;

        // number of blocks in fs
        uint64_t m_totalBlocks;

        // file blocks to be used to store file
        std::vector<uint64_t> m_blocksToUse;

        // a buffer used to store the incoming bytes
        // will be flushed when it hits block size or is remaining data
        mutable std::vector<uint8_t> m_dataBuffer;

        // the current file block index
        uint64_t m_currentBlockIndex;

        // buffered stores the number of bytes ever buffered
        // will eventually end up being fsize + MAX_FILENAME_LENGTH
        // Note does not take into account file block meta data
        // (the first 20 bytes)
        uint64_t m_buffered;

        /**
         * writes to the file metadata block located at some point after
         * the volume bit map (8 + volume bit map size + 8 + (metaBlock * METABLOCK_SIZE))
         */
        void writeMetaBlock();

        /**
         * updates all superblock info; the number of file blocks in use
         * (first 8 bytes), the volume bitmap (next N bytes, determined by
         * how many file blocks there are), the number of files in the bfs
         */
        void updateSuperBlock();

        /**
         * @brief calculates the next associate file block
         * @param next the next block index
         * @param b the current block
         */
        void computeNextBlockIndex(uint64_t &next, uint64_t const b);

        /**
         * @brief buffers the number of file bytes written to the current file block
         * flushed to the correct point in the image stream
         */
        void bufferBytesUsedToDescribeBytesOccupiedForFileBlockN();

        /**
         * @brief buffers the next block index
         * @param nextBlockIndex the next block index
         */
        void bufferNextBlockIndexForFileBlockN(uint64_t const nextBlockIndex);

        /**
         * @brief buffers an incoming byte from the write function. When the buffer
         * reaches a certain size, it is flushed to the correct file block offset
         * in the image stream
         * @param byte the byte to add to the byte buffer
         */
        void bufferByte(char const byte);

    };


}

#endif // BFS_BFS_ENTRY_WRITER_HPP__
