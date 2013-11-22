#include "BFSEntryWriter.hpp"
#include "DetailBFS.hpp"
#include "DetailFileBlock.hpp"
#include "DetailMeta.hpp"

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
            // 12 (FILE_BLOCK_META)
            // 4 for number of bytes utilized in a given block
            // 8 for next block index
            uint64_t const blockSizeWithoutMetaStuff = FILE_BLOCK_SIZE - FILE_BLOCK_META;

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
        , m_totalBlocks(0)
        , m_blocksToUse()
        , m_currentBlockIndex(0)
        , m_buffered(0)
    {
        std::fstream stream(m_bfsOutputPath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
        m_totalBlocks = detail::getBlockCount(stream);
        m_blocksToUse = detail::getNAvailableBlocks(stream, detail::computeBlocksRequired(fsize), m_totalBlocks);
        stream.close();
    }


    /**
     * writes the file metadata which consists of 25 bytes of data made up of:-
     *
     * 1 byte for extra info (only the first bit which indicates if the metablock is in use
     * is currently utilized; tbd. regarding the remaining bits, maybe file permissions)
     * 8 bytes specifying the size of the file
     * 8 bytes specifying the index of the first file block used to construct file
     * 8 bytes specifying the parent index
     */
    void
    BFSEntryWriter::writeMetaBlock()
    {
        std::fstream stream(m_bfsOutputPath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
        uint64_t const nextAvailableMetaBlock = detail::getNextAvailableMetaBlock(stream, m_totalBlocks);
        uint64_t offset = detail::getOffsetOfMetaBlock(nextAvailableMetaBlock, m_totalBlocks);
        (void)stream.seekg(offset);

        //Êfirst set metablog in use bit (very first bit of block)
        uint8_t firstByte = 0;
        (void)stream.read((char*)&firstByte, 1);
        detail::setBitInByte(firstByte, 0);
        (void)stream.seekp(offset);
        (void)stream.write((char*)&firstByte, 1);

        // compute other meta information
        uint8_t sizeBytes[8];
        detail::convertInt64ToInt8Array(m_fsize, sizeBytes);
        uint8_t firstFileBlockIndex[8];
        detail::convertInt64ToInt8Array(m_blocksToUse[0], firstFileBlockIndex);
        uint8_t parentBytes[8];
        detail::convertInt64ToInt8Array(m_blocksToUse[0], parentBytes);

        // write out remaining meta information
        stream.write((char*)sizeBytes, 8);
        stream.write((char*)firstFileBlockIndex, 8);
        stream.write((char*)parentBytes, 8);
        stream.close();
    }

    /**
     * Updates blocks used, the volume bitmap, and the file count
     */
    void BFSEntryWriter::updateSuperBlock()
    {
        std::fstream stream(m_bfsOutputPath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
        detail::setBlocksUsed(stream, m_blocksToUse.size());
        detail::updateVolumeBitmap(stream, m_blocksToUse, m_totalBlocks);
        detail::incrementFileCount(stream, m_totalBlocks);
        stream.close();
    }


    void
    BFSEntryWriter::bufferBytesUsedToDescribeBytesOccupiedForFileBlockN()
    {
        uint64_t const bytes = m_totalBlocks / uint64_t(8);

        // file blocks to also store filename which also needs to be
        // taken in to account
        uint64_t const fSizePlusFileNameLength = m_fsize + detail::MAX_FILENAME_LENGTH;
        // 12:
        // 4 for number of bytes utilized in a given block
        // 8 for next block index
        uint64_t const blockSizeWithoutMetaStuff = detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META;
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
    BFSEntryWriter::bufferNextBlockIndexForFileBlockN(uint64_t const nextBlockIndex)
    {
        uint8_t dat[8];
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
            std::fstream stream(m_bfsOutputPath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
            (void)stream.seekp(offset);

            (void)stream.write((char*)&m_dataBuffer.front(), m_dataBuffer.size());
            m_dataBuffer.clear();

            // write the metablock info associated with this file
            writeMetaBlock();

            // finally update super block with number of file blocks in use
            // (first 8 bytes), the volume bitmap with newly allocated file blocks
            // (the next m_totalBlocks bytes), and the number of files (the next
            // 8 bytes)
            updateSuperBlock();

            // finished writing data!
            stream.close();


        } else {
            if(m_dataBuffer.size() == detail::FILE_BLOCK_SIZE) {

                // write
                // clear
                // update current block index
                // start again

                uint64_t const bytes = m_totalBlocks / uint64_t(8);
                uint64_t const offset = detail::getOffsetOfFileBlock(m_blocksToUse[m_currentBlockIndex], m_totalBlocks);
                std::fstream stream(m_bfsOutputPath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
                (void)stream.seekp(offset);
                (void)stream.write((char*)&m_dataBuffer.front(), m_dataBuffer.size());

                m_dataBuffer.clear();

                ++m_currentBlockIndex;

                // write how many bytes will be occupied in the next block
                bufferBytesUsedToDescribeBytesOccupiedForFileBlockN();
                uint64_t next;
                computeNextBlockIndex(next, m_currentBlockIndex);
                bufferNextBlockIndexForFileBlockN(next);
                stream.close();
            }
        }
    }

    std::streamsize
    BFSEntryWriter::write(char const * const buf, std::streamsize const n)
    {
        if(m_currentBlockIndex == 0){
            if(m_dataBuffer.empty()) {
                const uint64_t fSizePlusFileNameLength = m_fsize + detail::MAX_FILENAME_LENGTH;
                const uint64_t blockSizeWithoutMetaStuff = detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META;

                // write very first file block data which includes filename
                uint64_t next;
                bufferBytesUsedToDescribeBytesOccupiedForFileBlockN();
                if (fSizePlusFileNameLength < blockSizeWithoutMetaStuff) {
                    next = 0;
                } else {
                    next = 1;
                }

                // write out previous and next block indices
                bufferNextBlockIndexForFileBlockN(next);

                // write filename portion of block. Already in correct place in stream
                for(int i = 0; i < m_entryName.length(); ++i) {m_dataBuffer.push_back(m_entryName[i]);}
                char eol = '\0';
                m_dataBuffer.push_back(eol);
                for(int i = 0; i < detail::MAX_FILENAME_LENGTH - m_entryName.length() - 1; ++i) {
                    m_dataBuffer.push_back(eol);
                }


                m_buffered = m_dataBuffer.size() - detail::FILE_BLOCK_META; // don't accumulate meta
            }

        }
        for(int i = 0; i < n; ++i) {
            bufferByte(buf[i]);
        }
    }

    void
    BFSEntryWriter::computeNextBlockIndex(uint64_t &next, uint64_t const b)
    {
        if (b == m_blocksToUse.size() - 1) {
            next = b;
        } else {
            next = b + 1;
        }
    }

    BFSEntryWriter::~BFSEntryWriter()
    {
    }

}
