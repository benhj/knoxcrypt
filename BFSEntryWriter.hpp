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
        std::string const m_bfsOutputPath;                    // the image path
        std::string const m_entryName;                        // the name of the entry to add
        uint64_t const m_fsize;                               // size of the file to add
        uint64_t const m_parentIndex;                         // parent folder index
        boost::shared_ptr<std::fstream> m_bfsOutputStream;    // the image stream (shared for copyability)
        uint64_t const m_totalBlocks;                         // number of blocks in fs
        std::vector<uint64_t> m_blocksToUse;                  // file blocks to be used to store file

        // a buffer used to store the incoming bytes
        // will be flushed when it hits block size of is remaining data
        mutable std::vector<uint8_t> m_dataBuffer;

        // the current file block index
        uint64_t m_currentBlockIndex;

        // buffered stores the number of bytes ever buffered
        // will eventually end up being fsize + MAX_FILENAME_LENGTH
        uint64_t m_buffered;

        uint64_t m_metaOffset;                                // point where meta data should be stored
        uint64_t m_fileOffset;                                // point where file data should be stored

        void writeMetaBlock();                                // the file metadata
        void updateSuperBlock();                              // the first 24 bytes
        void computePreviousAndNextBlockIndices(uint64_t &prev, uint64_t &next, uint64_t const b);
        uint64_t computeBufferSize(uint64_t const b);


        void bufferBytesUsedForFileBlockN();
        void bufferLastAndNextBlockIndicesForFileBlockN(uint64_t const lastBlockIndex,
                                                       uint64_t const nextBlockIndex);
        void bufferByte(char const byte);

    };


}

#endif // BFS_BFS_ENTRY_WRITER_HPP__
