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
    {
        //updateSuperBlock();
        //writeMetaBlock();
        //writeFileName();
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


    void
    BFSEntryWriter::bufferBytesUsedForFileBlockN()
    {
        uint64_t const bytes = m_totalBlocks / uint64_t(8);
        uint64_t const offset = detail::getOffsetOfFileBlock(m_blocksToUse[m_currentBlockIndex], m_totalBlocks);

        // file blocks to also store filename which also needs to be
        // taken in to account
        uint64_t const fSizePlusFileNameLength = m_fsize + detail::MAX_FILENAME_LENGTH;
        // 20:
        // 4 for number of bytes utilized in a given block
        // 8 for next block index
        // 8 for previous block index
        uint64_t const blockSizeWithoutMetaStuff = detail::FILE_BLOCK_SIZE - 20;
        if (fSizePlusFileNameLength < blockSizeWithoutMetaStuff) {
            uint8_t dat[4];
            detail::convertInt32ToInt4Array((uint32_t)fSizePlusFileNameLength, dat);
            for(int i = 0; i < 4; ++i) {m_dataBuffer.push_back(dat[i]);}
        } else {
            uint64_t const leftOver = fSizePlusFileNameLength % blockSizeWithoutMetaStuff;
            uint64_t const roundedDown = fSizePlusFileNameLength - leftOver;
            uint64_t const basalBlocks = roundedDown / blockSizeWithoutMetaStuff;
            uint8_t dat[4];
            if (m_currentBlockIndex < basalBlocks) {
                detail::convertInt32ToInt4Array((uint32_t)blockSizeWithoutMetaStuff, dat);
                for(int i = 0; i < 4; ++i) {m_dataBuffer.push_back(dat[i]);}
            } else {
                detail::convertInt32ToInt4Array((uint32_t)leftOver, dat);
                for(int i = 0; i < 4; ++i) {m_dataBuffer.push_back(dat[i]);}
            }
        }
    }

    void
    BFSEntryWriter::bufferLastAndNextBlockIndicesForFileBlockN(uint64_t const lastBlockIndex,
                                                              uint64_t const nextBlockIndex)
    {
        uint64_t const bytes = m_totalBlocks / uint64_t(8);
        // note, the '4' on the end signifies skip past the 4 byte 'bytes used' block
        uint64_t const offset = detail::getOffsetOfFileBlock(m_blocksToUse[m_currentBlockIndex], m_totalBlocks) + 4;
        uint8_t dat[8];
        detail::convertInt64ToInt8Array(m_blocksToUse[lastBlockIndex], dat);
        for(int i = 0; i < 8; ++i) {m_dataBuffer.push_back(dat[i]);}
        detail::convertInt64ToInt8Array(m_blocksToUse[nextBlockIndex], dat);
        for(int i = 0; i < 8; ++i) {m_dataBuffer.push_back(dat[i]);}
    }

    void
    BFSEntryWriter::bufferByte(char const byte)
    {
        m_dataBuffer.push_back(byte);
        ++m_buffered;
        if(m_buffered == m_fsize + detail::MAX_FILENAME_LENGTH) {

            // write out remaining bytes
            uint64_t const bytes = m_totalBlocks / uint64_t(8);
            uint64_t const offset = detail::getOffsetOfFileBlock(m_blocksToUse[m_currentBlockIndex], m_totalBlocks);
            (void)m_bfsOutputStream->seekp(offset);
            (void)m_bfsOutputStream->write((char*)&m_dataBuffer.front(), m_dataBuffer.size());
            m_dataBuffer.clear();

        } else {
            if(m_dataBuffer.size() == detail::FILE_BLOCK_SIZE) {

                // write
                // clear
                // update current block index
                // start again

                uint64_t const bytes = m_totalBlocks / uint64_t(8);
                uint64_t const offset = detail::getOffsetOfFileBlock(m_blocksToUse[m_currentBlockIndex], m_totalBlocks);
                (void)m_bfsOutputStream->seekp(offset);
                (void)m_bfsOutputStream->write((char*)&m_dataBuffer.front(), m_dataBuffer.size());

                m_dataBuffer.clear();
                ++m_currentBlockIndex;

                // write how many bytes will be occupied in the next block
                uint64_t bufferSize = computeBufferSize(m_currentBlockIndex);
                bufferBytesUsedForFileBlockN();
                uint64_t prev;
                uint64_t next;
                computePreviousAndNextBlockIndices(prev, next, m_currentBlockIndex);
                bufferLastAndNextBlockIndicesForFileBlockN(prev, next);
            }
        }
    }

    std::streamsize
    BFSEntryWriter::write(char const * const buf, std::streamsize const n)
    {

        if(m_currentBlockIndex == 0 && m_dataBuffer.empty()){
            const uint64_t fSizePlusFileNameLength = m_fsize + detail::MAX_FILENAME_LENGTH;
            const uint64_t blockSizeWithoutMetaStuff = detail::FILE_BLOCK_SIZE - 20;

            // write very first file block data which includes filename
            {
                uint64_t bufferSize = computeBufferSize(0);
                uint64_t prev;
                uint64_t next;
                bufferBytesUsedForFileBlockN();
                if (fSizePlusFileNameLength < blockSizeWithoutMetaStuff) {
                    prev = 0;
                    next = 0;
                } else {
                    prev = 0;
                    next = 1;
                }

                // write out previous and next block indices
                bufferLastAndNextBlockIndicesForFileBlockN(prev, next);

                // write filename portion of block. Already in correct place in stream
                for(int i = 0; i<m_entryName.length(); ++i) {m_dataBuffer.push_back(m_entryName[i]);}
                char eol = '\0';
                m_dataBuffer.push_back(eol);
            }
            m_buffered = m_dataBuffer.size() - 20; // don't accumulate meta info
        }
        for(int i = 0; i < n; ++i) {
            bufferByte(buf[i]);
        }
    }

    uint64_t
    BFSEntryWriter::computeBufferSize(uint64_t const b)
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
    BFSEntryWriter::computePreviousAndNextBlockIndices(uint64_t &prev, uint64_t &next, uint64_t const b)
    {
        if (b == 0) {
            prev = b;
        } else {
            prev = b - 1;
        }
        if (b == m_blocksToUse.size() - 1) {
            next = b;
        } else {
            next = b + 1;
        }
    }

    BFSEntryWriter::~BFSEntryWriter()
    {
        m_bfsOutputStream->flush();
        m_bfsOutputStream->close();
    }

}
