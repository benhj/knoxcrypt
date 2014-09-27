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

#include "teasafe/FileBlockBuilder.hpp"
#include "teasafe/TeaSafeImageStream.hpp"
#include "teasafe/detail/DetailTeaSafe.hpp"

namespace teasafe
{

    namespace
    {

        teasafe::BlockDeque populateBlockDeque(SharedCoreIO const &io)
        {
            // obtain all available blocks and store in a map for quick lookup
            teasafe::TeaSafeImageStream stream(io, std::ios::in | std::ios::out | std::ios::binary);
            std::vector<uint64_t> allBlocks = detail::getNAvailableBlocks(stream,
                                                                          io->freeBlocks,
                                                                          io->blocks);
            BlockDeque deque(allBlocks.begin(), allBlocks.end());
            return deque;
        }

        void checkAndInitStream(SharedCoreIO const & io, SharedImageStream &stream)
        {
            if(!stream) {
                std::ios::openmode mode = std::ios::in;
                mode |= std::ios::out;
                mode |= std::ios::binary;
                stream = std::make_shared<TeaSafeImageStream>(io, mode);
            }
        }


        /// a very hacky means to attain the number of blocks written in the image
        uint64_t getInitialBlocksWritten(SharedCoreIO const & io, SharedImageStream &stream)
        {
            checkAndInitStream(io, stream);

            (void)stream->seekp(0, std::ios::end);
            std::streamsize toReturn = stream->tellp();
            stream->seekp(0);
            uint64_t const volumeBitMapBytes = io->blocks / uint64_t(8);
            toReturn -= (detail::beginning() + 8 /* block count */ + volumeBitMapBytes + 8 /* count */);
            if(toReturn == 0) { // no block written yet
                return 0;
            }

            return (toReturn / (detail::FILE_BLOCK_META + detail::FILE_BLOCK_SIZE));
        }
    }


    FileBlockBuilder::FileBlockBuilder()
      : m_blocksWritten(0)
    {

    }

    FileBlockBuilder::FileBlockBuilder(SharedCoreIO const &io)
        : m_blockDeque(populateBlockDeque(io))
        , m_blocksWritten(0)
    {

    }

    FileBlock
    FileBlockBuilder::buildWritableFileBlock(SharedCoreIO const &io,
                                             OpenDisposition const &openDisposition,
                                             SharedImageStream &stream,
                                             bool const enforceRootBlock)
    {
        // note building a new block to write to should always be in append mode
        uint64_t id;

        // if the starting file block is enforced, set to root block specified in m_io
        if (enforceRootBlock) {
            id = io->rootBlock;
        } else {
            id = m_blockDeque.front(); //*(detail::getNextAvailableBlock(stream, io->blocks));
            m_blockDeque.pop_front();
        }

        // check if block data is actually written into iomage structure (might not have been
        // if image is sparse).
        if(m_blocksWritten == 0) {
            m_blocksWritten = getInitialBlocksWritten(io, stream);
        }
        if(id >= m_blocksWritten) {
            checkAndInitStream(io, stream);
            detail::writeBlock(io, *stream, id);
            stream->flush();
            stream->close();
            ++m_blocksWritten;
        }

        return FileBlock(io, id, id, openDisposition, stream);
    }

    FileBlock
    FileBlockBuilder::buildFileBlock(SharedCoreIO const &io,
                                     uint64_t const index,
                                     OpenDisposition const &openDisposition,
                                     SharedImageStream &stream)
    {
        if(m_blocksWritten == 0) {
            m_blocksWritten = getInitialBlocksWritten(io, stream);
        }
        return FileBlock(io, index, openDisposition, stream);
    }

    void
    FileBlockBuilder::putBlockBack(uint64_t const block)
    {
        m_blockDeque.push_front(block);
    }


}
