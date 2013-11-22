#include "BFSEntryAppender.hpp"
#include "DetailBFS.hpp"
#include "DetailFileBlock.hpp"
#include "DetailMeta.hpp"


namespace bfs
{

    BFSEntryAppender::BFSEntryAppender(std::string const &bfsOutputPath,
                                       uint64_t const totalBlocks,
                                       uint64_t const startIndex,
                                       uint64_t const metaBlockIndex,
                                       uint64_t const offset)
        : m_bfsOutputPath(bfsOutputPath)
        , m_totalBlocks(totalBlocks)
        , m_newBlocksOccupied()
        , m_dataBuffer()
        , m_fileBlockInImage(startIndex)
        , m_metaBlockIndex(metaBlockIndex)
        , m_buffered(0)
        , m_sizeInLastBlock(getSizeOfLastBlock())
    {
    }

    BFSEntryAppender::~BFSEntryAppender()
    {

    }

    std::streamsize
    BFSEntryAppender::write(char const * const buf, std::streamsize const n)
    {
        for(int i = 0; i < n; ++i) {
            bufferByte(buf[i]);
        }
    }

    void
    BFSEntryAppender::finishUp()
    {
        std::fstream stream(m_bfsOutputPath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
        (void)stream.seekp(detail::getOffsetOfFileBlock(m_fileBlockInImage, m_totalBlocks)
                           + detail::FILE_BLOCK_META
                           + m_sizeInLastBlock);
        (void)stream.write((char*)&m_dataBuffer.front(), m_dataBuffer.size());
        detail::writeNumberOfDataBytesWrittenToFileBlockN(stream, m_fileBlockInImage, m_totalBlocks, m_dataBuffer.size());

        m_sizeInLastBlock = 0;
        m_dataBuffer.clear();
        detail::writeIndexOfNextFileBlockFromFileBlockN(stream, m_fileBlockInImage, m_totalBlocks, m_fileBlockInImage);
        stream.close();
    }

    void
    BFSEntryAppender::bufferByte(char const byte)
    {
        m_dataBuffer.push_back(byte);

        uint32_t const occupied = (uint32_t)(m_dataBuffer.size() + m_sizeInLastBlock);
        if(occupied == detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META) {
            std::fstream stream(m_bfsOutputPath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
            (void)stream.seekp(detail::getOffsetOfFileBlock(m_fileBlockInImage, m_totalBlocks)
                               + detail::FILE_BLOCK_META
                               + m_sizeInLastBlock);
            (void)stream.write((char*)&m_dataBuffer.front(), m_dataBuffer.size());
            detail::writeNumberOfDataBytesWrittenToFileBlockN(stream, m_fileBlockInImage, m_totalBlocks, occupied);
            m_sizeInLastBlock = 0;
            m_dataBuffer.clear();

            // set next available file block
            // note returns an optional that must be dereferenced
            // todo -- what happens when no file blocks available????
            detail::OptionalBlock block = detail::getNextAvailableBlock(stream);
            if(block) {
                detail::writeIndexOfNextFileBlockFromFileBlockN(stream, m_fileBlockInImage, m_totalBlocks, *block);
                m_fileBlockInImage = *block;
                m_newBlocksOccupied.push_back(m_fileBlockInImage);

            } else {
                // throw???
            }

            stream.close();

        } else {
            if(m_dataBuffer.size() == detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META) {
                std::fstream stream(m_bfsOutputPath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
                (void)stream.seekp(detail::getOffsetOfFileBlock(m_fileBlockInImage, m_totalBlocks)
                                   + detail::FILE_BLOCK_META
                                   + m_sizeInLastBlock);
                (void)stream.write((char*)&m_dataBuffer.front(), m_dataBuffer.size());
                detail::writeNumberOfDataBytesWrittenToFileBlockN(stream, m_fileBlockInImage, m_totalBlocks, m_dataBuffer.size());

                m_sizeInLastBlock = 0;
                m_dataBuffer.clear();

                // set next available file block
                // note returns an optional that must be dereferenced
                // todo -- what happens when no file blocks available????
                detail::OptionalBlock block = detail::getNextAvailableBlock(stream);
                if(block) {
                    detail::writeIndexOfNextFileBlockFromFileBlockN(stream, m_fileBlockInImage, m_totalBlocks, *block);
                    m_fileBlockInImage = *block;
                    m_newBlocksOccupied.push_back(m_fileBlockInImage);
                } else {
                    // throw???
                }

                stream.close();
            }
        }
    }

    uint32_t
    BFSEntryAppender::getSizeOfLastBlock()
    {
        std::fstream stream(m_bfsOutputPath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
        uint64_t nextIndex = detail::getIndexOfNextFileBlockFromFileBlockN(stream, m_fileBlockInImage, m_totalBlocks);
        while(nextIndex != m_fileBlockInImage) {
            m_fileBlockInImage = nextIndex;
            nextIndex = detail::getIndexOfNextFileBlockFromFileBlockN(stream, m_fileBlockInImage, m_totalBlocks);
        }

        // set the number of data bytes occupied by last block
        uint32_t offset = detail::getNumberOfDataBytesWrittenToFileBlockN(stream, m_fileBlockInImage, m_totalBlocks);
        stream.close();
        return offset;
    }

}
