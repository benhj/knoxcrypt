
#include "FileBlock.hpp"

namespace bfs
{

    FileBlock::FileBlock(std::string const &imagePath,
                         uint64_t const totalBlocks,
                         uint64_t const index,
                         uint64_t const next,
                         AppendOrOverwrite const appendOrOverwrite)
        : m_imagePath(imagePath)
        , m_totalBlocks(totalBlocks)
        , m_index(index)
        , m_bytesWritten(0)
        , m_next(next)
        , m_offset(0)
        , m_seekPos(0)
        , m_initialBytesWritten(0)
        , m_writeMode(appendOrOverwrite)
    {
        // set m_offset
        BFSImageStream stream(m_imagePath, std::ios::in | std::ios::out | std::ios::binary);
        m_offset = detail::getOffsetOfFileBlock(m_index, m_totalBlocks);
        (void)stream.seekp(m_offset);

        // write m_bytesWritten
        uint8_t sizeDat[4];
        uint32_t size = 0;
        detail::convertInt32ToInt4Array(size, sizeDat);
        (void)stream.write((char*)sizeDat, 4);

        // write m_next
        uint8_t nextDat[8];
        detail::convertUInt64ToInt8Array(m_next, nextDat);
        assert(m_next == detail::convertInt8ArrayToInt64(nextDat));
        (void)stream.write((char*)nextDat, 8);
        stream.flush();
        stream.close();
    }

    FileBlock::FileBlock(std::string const &imagePath,
                         uint64_t const totalBlocks,
                         uint64_t const index,
                         AppendOrOverwrite const appendOrOverwrite)
        : m_imagePath(imagePath)
        , m_totalBlocks(totalBlocks)
        , m_index(index)
        , m_bytesWritten(0)
        , m_next(0)
        , m_offset(0)
        , m_seekPos(0)
        , m_writeMode(appendOrOverwrite)
    {
        // set m_offset
        BFSImageStream stream(m_imagePath, std::ios::in | std::ios::out | std::ios::binary);
        m_offset = detail::getOffsetOfFileBlock(m_index, m_totalBlocks);
        (void)stream.seekg(m_offset);

        // read m_bytesWritten
        uint8_t sizeDat[4];
        (void)stream.read((char*)sizeDat, 4);
        m_bytesWritten = detail::convertInt4ArrayToInt32(sizeDat);
        m_initialBytesWritten = m_bytesWritten;

        // read m_next
        uint8_t nextDat[8];
        (void)stream.read((char*)nextDat, 8);
        m_next = detail::convertInt8ArrayToInt64(nextDat);

        stream.close();
    }

    boost::iostreams::stream_offset
    FileBlock::tell() const
    {
        return m_seekPos;
    }

    boost::iostreams::stream_offset
    FileBlock::seek(boost::iostreams::stream_offset off,
                    std::ios_base::seekdir way)
    {
        m_seekPos = off;
        return m_seekPos;
    }

    std::streamsize
    FileBlock::read(char * const buf, std::streamsize const n) const
    {
        // open the image stream for reading
        BFSImageStream stream(m_imagePath, std::ios::in | std::ios::out | std::ios::binary);
        (void)stream.seekg(m_offset + detail::FILE_BLOCK_META + m_seekPos);
        (void)stream.read((char*)buf, n);
        stream.close();
        return n;
    }

    std::streamsize
    FileBlock::write(char const * const buf, std::streamsize const n) const
    {
        // open the image stream for writing
        BFSImageStream stream(m_imagePath, std::ios::in | std::ios::out | std::ios::binary);
        (void)stream.seekp(m_offset + detail::FILE_BLOCK_META + m_seekPos);
        (void)stream.write((char*)buf, n);

        // do updates to file block metadata only if in append mode
        if (m_writeMode == AppendOrOverwrite::Append) {

            // check if we need to update m_bytesWritten and m_next. This will be different
            // probably if the number of bytes read is not consistent with the
            // reported size stored in m_bytesWritten or if the stream has been moved
            // to a position past its start as indicated by m_extraOffset
            m_bytesWritten += uint32_t(n);

            // update m_bytesWritten
            (void)stream.seekp(m_offset);
            uint8_t sizeDat[4];
            detail::convertInt32ToInt4Array(m_bytesWritten, sizeDat);
            (void)stream.write((char*)sizeDat, 4);

            if (n < detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META || m_seekPos > 0) {

                m_next = m_index;
                // update m_next
                uint8_t nextDat[8];
                detail::convertUInt64ToInt8Array(m_next, nextDat);
                (void)stream.write((char*)nextDat, 8);
            }
        }

        stream.flush();
        stream.close();

        m_seekPos += n;

        return n;
    }

    uint32_t
    FileBlock::getDataBytesWritten() const
    {
        return m_bytesWritten;
    }

    uint32_t
    FileBlock::getInitialDataBytesWritten() const
    {
        return m_initialBytesWritten;
    }

    uint64_t
    FileBlock::getNextIndex() const
    {
        return m_next;
    }

    uint64_t
    FileBlock::getBlockOffset() const
    {
        return m_offset;
    }

    uint64_t
    FileBlock::getIndex() const
    {
        return m_index;
    }

    void
    FileBlock::setNext(uint64_t const next)
    {
        m_next = next;
        BFSImageStream stream(m_imagePath, std::ios::in | std::ios::out | std::ios::binary);
        (void)stream.seekp(m_offset+4); // 4 indicate bytes written
        // update m_next
        uint8_t nextDat[8];
        detail::convertUInt64ToInt8Array(m_next, nextDat);
        (void)stream.write((char*)nextDat, 8);
        stream.flush();
        stream.close();
    }

    void
    FileBlock::registerBlockWithVolumeBitmap()
    {
        BFSImageStream stream(m_imagePath, std::ios::in | std::ios::out | std::ios::binary);
        detail::updateVolumeBitmapWithOne(stream, m_index, m_totalBlocks);
        stream.close();
    }
}
