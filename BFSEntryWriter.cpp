#include "BFSEntryWriter.hpp"
#include "Detail.hpp"

#include <boost/make_shared.hpp>

namespace bfs
{

    namespace detail
    {
        /**
         * @brief gets number of file blocks required to store file
         * @param fsize the size of the file to store
         * @return the number of file blocks required
         */
        inline uint64_t computeBlocksRequired(uint64_t const fsize)
        {
            // file blocks to also store filename which also needs to be
            // taken in to account
            uint64_t const fSizePlusFileNameLength = fsize + MAX_FILENAME_LENGTH;
            // 20:
            // 4 for number of bytes utilized in a given block
            // 8 for next block index
            // 8 for previous block index
            uint64_t const blockSizeWithoutMetaStuff = FILE_BLOCK_SIZE - 20;

            // compute how many 512 byte blocks are required to store the file
            uint64_t blocksRequired(0);
            if (fSizePlusFileNameLength < blockSizeWithoutMetaStuff) {
                ++blocksRequired; // 1 block required
                return blocksRequired;
            } else {
                uint64_t const leftOver = fSizePlusFileNameLength % blockSizeWithoutMetaStuff;
                uint64_t const roundedDown = fSizePlusFileNameLength - leftOver;
                uint64_t const basalBlocks = roundedDown / blockSizeWithoutMetaStuff;
                blocksRequired = basalBlocks;
                if (leftOver > 0) {
                    ++blocksRequired;
                }
                return blocksRequired;
            }
        }

        /**
         * @brief write the number of bytes used in a given file block
         * @param fs the fs image stream
         * @param fsize the total size of the file
         * @param fileBlockN the nth block making up file
         * @param blockIndex the index of the block in the volume bitmap
         * @param totalBlocks the total number of blocks making up fs
         */
        inline void writeBytesUsedInFileBlockN(std::fstream &fs,
                                               uint64_t const fsize,
                                               uint64_t const fileBlockN,
                                               uint64_t const blockIndex,
                                               uint64_t const totalBlocks)
        {
            uint64_t const bytes = totalBlocks / uint64_t(8);
            uint64_t const offset = getOffsetOfFileBlock(blockIndex, totalBlocks);
            (void)fs.seekp(offset);

            // file blocks to also store filename which also needs to be
            // taken in to account
            uint64_t const fSizePlusFileNameLength = fsize + MAX_FILENAME_LENGTH;
            // 20:
            // 4 for number of bytes utilized in a given block
            // 8 for next block index
            // 8 for previous block index
            uint64_t const blockSizeWithoutMetaStuff = FILE_BLOCK_SIZE - 20;
            if (fSizePlusFileNameLength < blockSizeWithoutMetaStuff) {
                uint8_t dat[4];
                convertInt32ToInt4Array((uint32_t)fSizePlusFileNameLength, dat);
                (void)fs.write((char*)dat, 4);
            } else {
                uint64_t const leftOver = fSizePlusFileNameLength % blockSizeWithoutMetaStuff;
                uint64_t const roundedDown = fSizePlusFileNameLength - leftOver;
                uint64_t const basalBlocks = roundedDown / blockSizeWithoutMetaStuff;
                uint8_t dat[4];
                if (fileBlockN < basalBlocks) {
                    convertInt32ToInt4Array((uint32_t)blockSizeWithoutMetaStuff, dat);
                    (void)fs.write((char*)dat, 4);
                } else {
                    convertInt32ToInt4Array((uint32_t)leftOver, dat);
                    (void)fs.write((char*)dat, 4);
                }
            }
        }

        /**
         * @brief write the last and next block index to file block
         * @param fs the fs stream
         * @param lastBlockIndex the index of the last block
         * @param nextBlockIndex the index of the next block
         * @param thisBlockIndex the index of 'this' block
         * @param totalBlocks total blocks in the fs
         */
        inline void writeLastAndNextBlockIndicesToFileBlockN(std::fstream &fs,
                                                             uint64_t const lastBlockIndex,
                                                             uint64_t const nextBlockIndex,
                                                             uint64_t const thisBlockIndex,
                                                             uint64_t const totalBlocks)
        {
            uint64_t const bytes = totalBlocks / uint64_t(8);
            // note, the '4' on the end signifies skip past the 4 byte 'bytes used' block
            uint64_t const offset = getOffsetOfFileBlock(thisBlockIndex, totalBlocks) + 4;
            (void)fs.seekp(offset);
            uint8_t dat[8];
            convertInt64ToInt8Array(lastBlockIndex, dat);
            (void)fs.write((char*)dat, 8);
            convertInt64ToInt8Array(nextBlockIndex, dat);
            (void)fs.write((char*)dat, 8);
        }
    }

    BFSEntryWriter::BFSEntryWriter(std::string const &bfsOutputPath,
                                   std::string const &entryName,
                                   uint64_t const fsize,
                                   uint64_t const parentIndex)
        : m_bfsOutputPath(bfsOutputPath)
        , m_entryName(entryName)
        , m_fsize(fsize)
        , m_parentIndex(parentIndex)
        , m_bfsOutputStream(boost::make_shared<std::fstream>(bfsOutputPath.c_str(), std::ios::in | std::ios::out | std::ios::binary))
        , m_totalBlocks(detail::getBlockCount(*m_bfsOutputStream))
        , m_blocksToUse(detail::getNAvailableBlocks(*m_bfsOutputStream, detail::computeBlocksRequired(fsize), m_totalBlocks))
          //, m_metaOffset(detail::getOffsetOfNextFreeMetaSpaceBlock(*m_bfsOutputStream))
          //   , m_fileOffset(detail::getOffsetOfNextFreeFileSpaceBlock(*m_bfsOutputStream))
    {
        updateSuperBlock();
        writeMetaBlock();
        writeFileName();
    }

    /**
     * writes the filename which has a max size of 50 bytes
     */
    void
    BFSEntryWriter::writeFileName()
    {
        std::vector<uint8_t> fname;
        fname.resize(detail::MAX_FILENAME_LENGTH);
        std::string::size_type t = m_entryName.length();
        for (int i = 0; i < t; ++i) {
            fname[i] = m_entryName[i];
        }
        fname[t] = '\0';
        uint64_t fileCount = detail::getFileCount(*m_bfsOutputStream);
        //uint64_t fileOffset = detail::getOffsetOfFileN(*m_bfsOutputStream, fileCount);
        //(void)m_bfsOutputStream->seekp((std::streampos)fileOffset);
        m_bfsOutputStream->write((char*)&fname.front(), detail::MAX_FILENAME_LENGTH);
    }

    /**
     * writes the file metadata which consists of:-
     *
     * 8 bytes specifying the size
     * 8 bytes specifying the offset in the bfs image
     * 8 bytes specifying the parent folder index
     * 8 bytes specifying other data (e.g. read-only, last modified etc)
     */
    void
    BFSEntryWriter::writeMetaBlock()
    {
        uint8_t sizeBytes[8];
        detail::convertInt64ToInt8Array(m_fsize, sizeBytes);
        uint8_t posBytes[8];
        detail::convertInt64ToInt8Array(m_fileOffset, posBytes);
        uint8_t parentBytes[8];
        detail::convertInt64ToInt8Array(m_parentIndex, parentBytes);
        (void)m_bfsOutputStream->seekp((std::streampos)m_metaOffset);
        m_bfsOutputStream->write((char*)sizeBytes, 8);    // size of file
        m_bfsOutputStream->write((char*)posBytes, 8);     // position in main file space
        m_bfsOutputStream->write((char*)parentBytes, 8);  // index of parent directory
        m_bfsOutputStream->write((char*)parentBytes, 8);  // extra space for other stuff (tbd)
    }

    /**
     * Updates file count and total file size information in the superblock
     */
    void BFSEntryWriter::updateSuperBlock()
    {
        detail::incrementFileCount(*m_bfsOutputStream);
        //detail::updateFileSpaceAccumulator(*m_bfsOutputStream, m_fsize);
    }

    void BFSEntryWriter::writeVeryFirstFileBlock(std::fstream& orig) const
    {
        const uint64_t fSizePlusFileNameLength = m_fsize
            + detail::MAX_FILENAME_LENGTH;
        const uint64_t blockSizeWithoutMetaStuff = detail::FILE_BLOCK_SIZE - 20;

        // write very first file block data which includes filename
        {
            uint64_t bufferSize = computeBufferSize(0);
            uint64_t prev;
            uint64_t next;
            if (fSizePlusFileNameLength < blockSizeWithoutMetaStuff) {
                prev = 0;
                next = 0;
                // write out bytes used to block
                detail::writeBytesUsedInFileBlockN(*m_bfsOutputStream,
                                                   fSizePlusFileNameLength, 0, m_blocksToUse[0],
                                                   m_totalBlocks);
            } else {
                prev = 0;
                next = m_blocksToUse[1];
                // write out previous and next block indices
                detail::writeLastAndNextBlockIndicesToFileBlockN(*m_bfsOutputStream,
                                                                 prev, next, m_blocksToUse[0], m_totalBlocks);
            }
            // write filename portion of block. Already in correct place in stream
            (void)(m_bfsOutputStream->write((char*)(m_entryName.c_str()),
                                            m_entryName.length()));
            char eol = '\0';
            (void)(m_bfsOutputStream->write((char*)(&eol), 1));
            m_bfsOutputStream->seekp(
                detail::getOffsetOfFileBlock(0, m_totalBlocks) + 20
                + detail::MAX_FILENAME_LENGTH);
            std::vector<uint8_t> readBuffer;
            readBuffer.resize(bufferSize, 0);
            (void)(orig.read((char*)(&readBuffer.front()), bufferSize));
            (void)(m_bfsOutputStream->write((char*)(&readBuffer.front()), bufferSize));
        }
    }

    void BFSEntryWriter::writeRemainingFileBlocks(std::fstream& orig) const
    {
        // write remaining data blocks
        if (m_blocksToUse.size() > 1) {
            for (uint64_t b(1); b < m_blocksToUse.size(); ++b) {
                // write how many bytes will be occupied
                uint64_t bufferSize = computeBufferSize(b);
                // write out bytes used to block
                detail::writeBytesUsedInFileBlockN(*m_bfsOutputStream, bufferSize,
                                                   b, m_blocksToUse[b], m_totalBlocks);
                // establish the previous and next block indices
                uint64_t prev;
                uint64_t next;
                computePreviousAndNextBlockIndices(prev, next, b);
                // write out previous and next block indices
                detail::writeLastAndNextBlockIndicesToFileBlockN(*m_bfsOutputStream,
                                                                 prev, next, m_blocksToUse[b], m_totalBlocks);
                std::vector<uint8_t> readBuffer;
                readBuffer.resize(bufferSize, 0);
                (void)(orig.read((char*)(&readBuffer.front()), bufferSize));
                (void)(m_bfsOutputStream->write((char*)(&readBuffer.front()),
                                                bufferSize));
            }
        }
    }

    void
    BFSEntryWriter::writeIn(std::fstream &orig) const
    {
        // seek to beginning of input stream
        orig.seekg(0, orig.beg);

        // write the very first block which includes filename
        writeVeryFirstFileBlock(orig);

        // write remaining data blocks
        writeRemainingFileBlocks(orig);
    }

    uint64_t
    BFSEntryWriter::computeBufferSize(uint64_t const b) const
    {
        uint64_t const fSizePlusFileNameLength = m_fsize + detail::MAX_FILENAME_LENGTH;
        uint64_t const blockSizeWithoutMetaStuff = detail::FILE_BLOCK_SIZE - 20;

        if(b == 0) {
            if (fSizePlusFileNameLength < blockSizeWithoutMetaStuff) {
                return fSizePlusFileNameLength;
            } else {
                return blockSizeWithoutMetaStuff;
            }
        }

        if (b < (m_blocksToUse.size() - 1)) {
            return blockSizeWithoutMetaStuff;
        } else {
            uint64_t const leftOver = fSizePlusFileNameLength % blockSizeWithoutMetaStuff;
            return leftOver;
        }
    }
    void
    BFSEntryWriter::computePreviousAndNextBlockIndices(uint64_t &prev, uint64_t &next, uint64_t const b) const
    {
        if (b == 0) {
            prev = m_blocksToUse[b];
        } else {
            prev = m_blocksToUse[b - 1];
        }
        if (b == m_blocksToUse.size() - 1) {
            next = m_blocksToUse[b];
        } else {
            next = m_blocksToUse[b + 1];
        }
    }

    BFSEntryWriter::~BFSEntryWriter()
    {
        m_bfsOutputStream->flush();
        m_bfsOutputStream->close();
    }

}
