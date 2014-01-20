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

#ifndef TeaSafe_TeaSafe_DETAIL_FOLDER_HPP__
#define TeaSafe_TeaSafe_DETAIL_FOLDER_HPP__

#include "teasafe/TeaSafeImageStream.hpp"
#include "teasafe/CoreTeaSafeIO.hpp"
#include "teasafe/detail/DetailFileBlock.hpp"

#include <iostream>
#include <stdint.h>
#include <vector>

namespace teasafe { namespace detail
{

    void incrementFolderEntryCount(CoreTeaSafeIO const &io,
                                   uint64_t const startBlock,
                                   uint64_t const inc = 1)
    {
        teasafe::TeaSafeImageStream out(io, std::ios::in | std::ios::out | std::ios::binary);
        uint64_t const offset = getOffsetOfFileBlock(startBlock, io.blocks);
        (void)out.seekg(offset + FILE_BLOCK_META);
        uint8_t buf[8];
        (void)out.read((char*)buf, 8);
        uint64_t count = convertInt8ArrayToInt64(buf);
        count += inc;
        (void)out.seekp(offset + FILE_BLOCK_META);
        convertUInt64ToInt8Array(count, buf);
        (void)out.write((char*)buf, 8);
        out.close();
    }

    void decrementFolderEntryCount(CoreTeaSafeIO const &io,
                                   uint64_t const startBlock,
                                   uint64_t const dec = 1)
    {
        teasafe::TeaSafeImageStream out(io, std::ios::in | std::ios::out | std::ios::binary);
        uint64_t const offset = getOffsetOfFileBlock(startBlock, io.blocks);
        (void)out.seekg(offset + FILE_BLOCK_META);
        uint8_t buf[8];
        (void)out.read((char*)buf, 8);
        uint64_t count = convertInt8ArrayToInt64(buf);
        count -= dec;
        (void)out.seekp(offset + FILE_BLOCK_META);
        convertUInt64ToInt8Array(count, buf);
        (void)out.write((char*)buf, 8);
        out.close();
    }

}
}

#endif // TeaSafe_TeaSafe_DETAIL_FOLDER_HPP__
