#ifndef BFS_BFS_ENTRY_APPENDER_HPP__
#define BFS_BFS_ENTRY_APPENDER_HPP__

#include <boost/shared_ptr.hpp>

#include <vector>

#include <boost/iostreams/categories.hpp>  // sink_tag
#include <iosfwd>                          // streamsize
#include <string>
#include <fstream>


namespace bfs
{
    class BFSEntryAppender
    {
      public:

        typedef char                          char_type;
        typedef boost::iostreams::sink_tag    category;

        BFSEntryAppender(std::string const &bfsOutputPath,
                         uint64_t const totalBlocks,
                         uint64_t const startIndex,
                         uint64_t const metaBlockIndex,
                         uint64_t const offset = 0);

        /**
         * @param buf the data to be written
         * @param n number of bytes to written
         * @return the number of bytes written
         */
        std::streamsize write(char const * const buf, std::streamsize const n);

        /**
         * @brief write any remaining bytes in the buffer
         */
        void finishUp();
        ~BFSEntryAppender();

      private:

        BFSEntryAppender();

        // the image path
        std::string const m_bfsOutputPath;

        // number of blocks in fs
        uint64_t m_totalBlocks;

        // file blocks to be used to store file
        std::vector<uint64_t> m_newBlocksOccupied;

        // a buffer used to store the incoming bytes
        // will be flushed when it hits block size or is remaining data
        mutable std::vector<uint8_t> m_dataBuffer;

        // the current file block index
        uint64_t m_fileBlockInImage;

        // the metablock that stores meta info about entry
        uint64_t m_metaBlockIndex;

        // buffered stores the number of bytes ever buffered
        // will eventually end up being fsize + MAX_FILENAME_LENGTH
        // Note does not take into account file block meta data
        // (the first 20 bytes)
        uint64_t m_buffered;

        // the data space occupied in last file block making up file
        uint32_t m_positionInfileBlock;

        uint32_t getSizeOfLastBlock();

        void bufferByte(char const byte);

        void updateMetaSizeInfo();

        void writeOutChunk(std::fstream &stream, uint32_t const count);

        void writeOutChunkAndUpdate(uint32_t const);

    };

}

#endif // BFS_BFS_ENTRY_APPENDER_HPP__
