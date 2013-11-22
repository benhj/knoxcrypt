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
        , m_positionInfileBlock(getSizeOfLastBlock())
    {
    }

    BFSEntryAppender::~BFSEntryAppender()
    {

    }

    void
    BFSEntryAppender::writeOutChunk(std::fstream &stream, uint32_t const count)
    {
        // write out the chunk of data to a position offset by where we should
        // start writing in the file block
        (void)stream.seekp(detail::getOffsetOfFileBlock(m_fileBlockInImage, m_totalBlocks)
                           + detail::FILE_BLOCK_META
                           + m_positionInfileBlock);
        (void)stream.write((char*)&m_dataBuffer.front(), m_dataBuffer.size());

        // write how many data bytes are written in this file block
        detail::writeNumberOfDataBytesWrittenToFileBlockN(stream, m_fileBlockInImage, m_totalBlocks, count);
    }

    void
    BFSEntryAppender::writeOutChunkAndUpdate(uint32_t const count)
    {
        std::fstream stream(m_bfsOutputPath.c_str(), std::ios::in | std::ios::out | std::ios::binary);

        // write out the data chunk
        writeOutChunk(stream, count);

        // position of in next file block should start at 0
        m_positionInfileBlock = 0;

        // clear the data buffer ready to be filled up with more data
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

    void
    BFSEntryAppender::bufferByte(char const byte)
    {
        m_dataBuffer.push_back(byte);

        // if the we're at the size of the buffer according to buffer size
        // offset to the position we start in the buffer, write it out
        uint32_t const occupied = (uint32_t)(m_dataBuffer.size() + m_positionInfileBlock);
        if(occupied == detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META) {
            writeOutChunkAndUpdate(occupied);
        }
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
        writeOutChunk(stream, m_dataBuffer.size());
        m_dataBuffer.clear();

        // last file block so set next index to be 'this' index
        detail::writeIndexOfNextFileBlockFromFileBlockN(stream, m_fileBlockInImage, m_totalBlocks, m_fileBlockInImage);

        // need to also update meta-block associated with this file

        // finally need to update super block and volume block

        stream.close();
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
