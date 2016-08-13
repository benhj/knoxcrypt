/*
  Copyright (c) <2013-2015>, <BenHJ>
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

#pragma once

#include "knoxcrypt/CoreknoxcryptIO.hpp"
#include "knoxcrypt/ContainerImageStream.hpp"
#include "knoxcrypt/detail/Detailknoxcrypt.hpp"

#include <iostream>
#include <stdint.h>
#include <vector>

namespace knoxcrypt { namespace detail
{

    /**
     * @brief gets the offset of a given file block
     * @param block the file block that we want to get the offset of
     * @return the offset of the file block
     */
    inline uint64_t getOffsetOfFileBlock(uint64_t const block,
                                         uint64_t const totalBlocks)
    {
        uint64_t const volumeBitMapBytes = totalBlocks / uint64_t(8);
        return beginning()                 // where main start after IV
            + 8                            // number of fs blocks
            + volumeBitMapBytes            // volume bit map
            + 8                            // total number of files
            + (FILE_BLOCK_SIZE * block);   // file block
    }

    /**
     * @brief gets the next file block index from the given file block
     * @param in the knoxcrypt image stream
     * @param n the file block to get the next index from
     * @param totalBlocks the total number of blocks in the knoxcrypt
     * @return the next file block index
     */
    inline uint64_t getIndexOfNextFileBlockFromFileBlockN(knoxcrypt::ContainerImageStream &in,
                                                          uint64_t const n,
                                                          uint64_t const totalBlocks)
    {
        auto offset = getOffsetOfFileBlock(n, totalBlocks) + 4;
        (void)in.seekg(offset);
        uint8_t dat[8];
        (void)in.read((char*)dat, 8);
        return convertInt8ArrayToInt64(dat);
    }

    /**
     * @brief gets the next file block index from the given file block
     * @param in the knoxcrypt image stream
     * @param n the file block to get the next index from
     * @param totalBlocks the total number of blocks in the knoxcrypt
     * @return the next file block index
     */
    inline uint32_t getNumberOfDataBytesWrittenToFileBlockN(knoxcrypt::ContainerImageStream &in,
                                                            uint64_t const n,
                                                            uint64_t const totalBlocks)
    {
        uint64_t offset = getOffsetOfFileBlock(n, totalBlocks);
        (void)in.seekg(offset);
        uint8_t dat[4];
        (void)in.read((char*)dat, 4);
        return convertInt4ArrayToInt32(dat);
    }

    /**
     * @brief write a given file block to disk
     * @param io the core io data structure
     * @param out the image stream to write to
     * @param block the block to write out
     */
    inline void writeBlock(SharedCoreIO const &io, ContainerImageStream &out, uint64_t const block)
    {
        std::vector<uint8_t> ints;
        ints.assign(FILE_BLOCK_SIZE - FILE_BLOCK_META, 0);

        // write out block metadata
        uint64_t offset = getOffsetOfFileBlock(block, io->blocks);
        (void)out.seekp(offset);

        // write m_bytesWritten; 0 to begin with
        uint8_t sizeDat[4];
        uint32_t size = 0;
        convertInt32ToInt4Array(size, sizeDat);
        (void)out.write((char*)sizeDat, 4);

        // write m_next; begins as same as index
        uint8_t nextDat[8];
        convertUInt64ToInt8Array(block, nextDat);
        (void)out.write((char*)nextDat, 8);

        // write data bytes
        (void)out.write((char*)&ints.front(), FILE_BLOCK_SIZE - FILE_BLOCK_META);

        assert(!out.bad());
    }
}
}

