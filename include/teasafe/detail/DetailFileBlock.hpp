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

#ifndef TeaSafe_TeaSafe_DETAIL_FILE_BLOCK_HPP__
#define TeaSafe_TeaSafe_DETAIL_FILE_BLOCK_HPP__

#include "teasafe/TeaSafeImageStream.hpp"
#include "teasafe/detail/DetailTeaSafe.hpp"

#include <iostream>
#include <stdint.h>
#include <vector>

namespace teasafe { namespace detail
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
        return 8                           // number of fs blocks
            + volumeBitMapBytes            // volume bit map
            + 8                            // total number of files
            + (FILE_BLOCK_SIZE * block);   // file block
    }

    /**
     * @brief gets the next file block index from the given file block
     * @param in the teasafe image stream
     * @param n the file block to get the next index from
     * @param totalBlocks the total number of blocks in the teasafe
     * @return the next file block index
     */
    inline uint64_t getIndexOfNextFileBlockFromFileBlockN(teasafe::TeaSafeImageStream &in,
                                                          uint64_t const n,
                                                          uint64_t const totalBlocks)
    {
        uint64_t offset = getOffsetOfFileBlock(n, totalBlocks) + 4;
        (void)in.seekg(offset);
        uint8_t dat[8];
        (void)in.read((char*)dat, 8);
        return convertInt8ArrayToInt64(dat);
    }

    /**
     * @brief gets the next file block index from the given file block
     * @param in the teasafe image stream
     * @param n the file block to get the next index from
     * @param totalBlocks the total number of blocks in the teasafe
     * @return the next file block index
     */
    inline uint32_t getNumberOfDataBytesWrittenToFileBlockN(teasafe::TeaSafeImageStream &in,
                                                            uint64_t const n,
                                                            uint64_t const totalBlocks)
    {
        uint64_t offset = getOffsetOfFileBlock(n, totalBlocks);
        (void)in.seekg(offset);
        uint8_t dat[4];
        (void)in.read((char*)dat, 4);
        return convertInt4ArrayToInt32(dat);
    }

    inline void writeNumberOfDataBytesWrittenToFileBlockN(teasafe::TeaSafeImageStream &in,
                                                          uint64_t const n,
                                                          uint64_t const totalBlocks,
                                                          uint32_t const bytesWritten)
    {
        uint64_t offset = getOffsetOfFileBlock(n, totalBlocks);
        (void)in.seekg(offset);
        uint8_t dat[4];
        convertInt32ToInt4Array(bytesWritten, dat);
        (void)in.write((char*)dat, 4);
    }

    inline void writeIndexOfNextFileBlockFromFileBlockN(teasafe::TeaSafeImageStream &in,
                                                        uint64_t const n,
                                                        uint64_t const totalBlocks,
                                                        uint32_t const nextIndex)
    {
        uint64_t offset = getOffsetOfFileBlock(n, totalBlocks) + 4;
        (void)in.seekg(offset);
        uint8_t dat[8];
        convertInt32ToInt4Array(nextIndex, dat);
        (void)in.write((char*)dat, 8);
    }

}
}

#endif // TeaSafe_TeaSafe_DETAIL_FILE_BLOCK_HPP__
