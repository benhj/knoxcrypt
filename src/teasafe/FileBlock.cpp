/*
  Copyright (c) <2013-2014>, <BenHJ>
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
  3. Neither the name of the copyright holder nor the names of its contributors
  may be used to endorse or promote products derived from this software without
  specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "teasafe/FileBlock.hpp"
#include "teasafe/FileBlockBuilder.hpp"
#include "teasafe/FileBlockException.hpp"

#include <boost/make_shared.hpp>

namespace teasafe
{

    FileBlock::FileBlock(SharedCoreIO const &io,
                         uint64_t const index,
                         uint64_t const,// next,
                         OpenDisposition const &openDisposition,
                         SharedImageStream const &stream)
        : m_io(io)
        , m_index(index)
        , m_bytesWritten(0)
        , m_initialBytesWritten(0)
        , m_next(index)
        , m_offset(0)
        , m_seekPos(0)
        , m_positionBeforeWrite(0)
        , m_openDisposition(openDisposition)
        , m_bytesToWriteOnFlush(0)
        , m_stream(stream)
    {
        // set m_offset
        m_offset = detail::getOffsetOfFileBlock(m_index, io->blocks);
    }

    FileBlock::FileBlock(SharedCoreIO const &io,
                         uint64_t const index,
                         OpenDisposition const &openDisposition,
                         SharedImageStream const &stream)
        : m_io(io)
        , m_index(index)
        , m_bytesWritten(0)
        , m_next(0)
        , m_offset(detail::getOffsetOfFileBlock(index, io->blocks))
        , m_seekPos(0)
        , m_openDisposition(openDisposition)
        , m_bytesToWriteOnFlush(0)
        , m_stream(stream)
    {
        // set m_offset
        initImageStream();
        detail::checkAndSeekG(*m_stream, m_offset);

        // read m_bytesWritten
        uint8_t sizeDat[4];
        (void)m_stream->read((char*)sizeDat, 4);
        m_bytesWritten = detail::convertInt4ArrayToInt32(sizeDat);
        m_initialBytesWritten = m_bytesWritten;

        // read m_next
        uint8_t nextDat[8];
        (void)m_stream->read((char*)nextDat, 8);
        m_next = detail::convertInt8ArrayToInt64(nextDat);
    }

    void
    FileBlock::initImageStream() const
    {
        if(!m_stream) {
            m_stream = boost::make_shared<TeaSafeImageStream>(m_io, std::ios::in | std::ios::out | std::ios::binary);
        } else {
            if(!m_stream->is_open()) {
                m_stream->open(m_io, std::ios::in | std::ios::out | std::ios::binary);
            }
        }
    }

    SharedImageStream
    FileBlock::getStream() const
    {
        return m_stream;
    }

    boost::iostreams::stream_offset
    FileBlock::tell() const
    {
        return m_seekPos;
    }

    boost::iostreams::stream_offset
    FileBlock::seek(boost::iostreams::stream_offset off,
                    std::ios_base::seekdir)
    {
        m_seekPos = off;
        return m_seekPos;
    }

    std::streamsize
    FileBlock::read(char * const buf, std::streamsize const n) const
    {
        if (n > 0) {
            if (m_openDisposition.readWrite() == ReadOrWriteOrBoth::WriteOnly) {
                throw FileBlockException(FileBlockError::NotReadable);
            }

            // open the image stream for reading
            initImageStream();
            detail::checkAndSeekG(*m_stream, m_offset + detail::FILE_BLOCK_META + m_seekPos);
            (void)m_stream->read((char*)buf, n);

            // update the stream position
            m_seekPos += n;
        }

        return n;
    }

    std::streamsize
    FileBlock::write(char const * const buf, std::streamsize const n) const
    {

        if (m_openDisposition.readWrite() == ReadOrWriteOrBoth::ReadOnly) {
            throw FileBlockException(FileBlockError::NotWritable);
        }

        // open the image stream for writing
        this->initImageStream();


        // TODO: image could be 'sparse' in which case block doesn't actually
        // exist yet. In this case, should write out the data to the container

        if(!detail::checkAndSeekP(*m_stream, m_offset + detail::FILE_BLOCK_META + m_seekPos)) {
            // TODO: image probably sparse since couldn't seek past end; therefore
            // need to write out block data here
        }
        (void)m_stream->write((char*)buf, n);

        // do updates to file block metadata only if in append mode
        // note update to next index taken care of in FileEntry
        if (m_openDisposition.append() == AppendOrOverwrite::Append) {

            m_positionBeforeWrite = m_seekPos;

            // check if we need to update m_bytesWritten and m_next. This will be different
            // probably if the number of bytes read is not consistent with the
            // reported size stored in m_bytesWritten or if the stream has been moved
            // to a position past its start as indicated by m_extraOffset
            m_bytesWritten += uint32_t(n);

            // optimization. This number will be written on flush to record number bytes written
            //m_bytesToWriteOnFlush = m_bytesWritten;

            doSetSize(*m_stream, m_bytesWritten);
        }
        // if in overwrite mode, we still need to check if writing goes above
        // the initial bytes written and update the size accordingly
        else if (m_seekPos + n > m_initialBytesWritten) {

            m_bytesWritten += uint32_t(n);

            // optimization. This number will be written on flush to record number bytes written
            //m_bytesToWriteOnFlush = m_seekPos + n;

            // update m_bytesWritten
            doSetSize(*m_stream, m_seekPos + n);
        }

        m_stream->flush();

        // update the stream position
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
    FileBlock::getIndex() const
    {
        return m_index;
    }

    void
    FileBlock::registerBlockWithVolumeBitmap()
    {
        this->initImageStream();
        detail::updateVolumeBitmapWithOne(*m_stream, m_index, m_io->blocks);
        m_io->freeBlocks--;
        m_stream->flush();
    }

    void
    FileBlock::setSizeOnFlush() const
    {
        this->initImageStream();
        doSetSize(*m_stream, m_seekPos);
        m_stream->flush();
    }

    void
    FileBlock::setSize(std::ios_base::streamoff size) const
    {
        this->initImageStream();
        doSetSize(*m_stream, size);
        m_initialBytesWritten = size;
        m_bytesWritten = size;
        m_stream->flush();
    }

    void
    FileBlock::doSetSize(TeaSafeImageStream &stream, std::ios_base::streamoff size) const
    {
        // update m_bytesWritten
        (void)stream.seekp(m_offset);
        uint8_t sizeDat[4];
        detail::convertInt32ToInt4Array(size, sizeDat);
        (void)stream.write((char*)sizeDat, 4);
    }

    void
    FileBlock::setNextIndex(uint64_t nextIndex) const
    {
        this->initImageStream();
        doSetNextIndex(*m_stream, nextIndex);
        m_next = nextIndex;
        m_stream->flush();
    }

    void
    FileBlock::doSetNextIndex(TeaSafeImageStream &stream, uint64_t nextIndex) const
    {
        // update m_next
        (void)stream.seekp(m_offset + 4);
        uint8_t nextDat[8];
        detail::convertUInt64ToInt8Array(nextIndex, nextDat);
        (void)stream.write((char*)nextDat, 8);
    }

    void
    FileBlock::unlink()
    {
        this->initImageStream();
        detail::updateVolumeBitmapWithOne(*m_stream, m_index, m_io->blocks, false);
        m_io->blockBuilder->putBlockBack(m_index);
        doSetNextIndex(*m_stream, m_index);
        doSetSize(*m_stream, 0);
        m_stream->flush();
    }

    bool
    FileBlock::operator==(FileBlock const &other) const
    {
        return this->m_index == other.m_index;
    }
}
