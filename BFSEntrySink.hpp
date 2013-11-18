#ifndef BFS_BFS_ENTRY_SINK_HPP__
#define BFS_BFS_ENTRY_SINK_HPP__

#include <boost/shared_ptr.hpp>

#include <vector>

#include <iosfwd>                          // streamsize
#include <string>
#include <fstream>


namespace bfs
{

    class BFSEntrySink
    {
      public:

        BFSEntrySink(std::string const &bfsOutputPath,
                     std::string const &entryName,
                     uint64_t const fsize,
                     uint64_t const parentIndex);

        /**
         * @param buf the data to be written
         * @param n number of bytes to written
         * @return the number of bytes written
         */
        void writeIn(std::fstream &orig) const;
        ~BFSEntrySink();

      private:
        std::string const m_bfsOutputPath;                    // the image path
        std::string const m_entryName;                        // the name of the entry to add
        uint64_t const m_fsize;                               // size of the file to add
        uint64_t const m_parentIndex;                         // parent folder index
        boost::shared_ptr<std::fstream> m_bfsOutputStream;    // the image stream (shared for copyability)
        uint64_t const m_totalBlocks;                         // number of blocks in fs
        std::vector<uint64_t> m_blocksToUse;                  // file blocks to be used to store file
        uint64_t m_metaOffset;                                // point where meta data should be stored
        uint64_t m_fileOffset;                                // point where file data should be stored

        void writeFileName();
        void writeMetaBlock();                                // the file metadata
        void updateSuperBlock();                              // the first 24 bytes
        void computePreviousAndNextBlockIndices(uint64_t &prev, uint64_t &next, uint64_t const b) const;
        uint64_t computeBufferSize(uint64_t const b) const;
    void writeVeryFirstFileBlock(std::fstream& orig) const;
};


}

#endif // BFS_BFS_ENTRY_SINK_HPP__
