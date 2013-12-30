/*
The MIT License (MIT)

Copyright (c) 2013 Ben H.D. Jones

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef BFS_BFS_DETAIL_FOLDER_HPP__
#define BFS_BFS_DETAIL_FOLDER_HPP__

#include "bfs/BFSImageStream.hpp"
#include "bfs/CoreBFSIO.hpp"
#include "bfs/DetailFileBlock.hpp"

#include <iostream>
#include <stdint.h>
#include <vector>

namespace bfs { namespace detail
{

    void incrementFolderEntryCount(CoreBFSIO const &io,
                                   uint64_t const startBlock)
    {
        bfs::BFSImageStream out(io, std::ios::in | std::ios::out | std::ios::binary);
        uint64_t const offset = getOffsetOfFileBlock(startBlock, io.blocks);
        (void)out.seekg(offset + FILE_BLOCK_META);
        uint8_t buf[8];
        (void)out.read((char*)buf, 8);
        uint64_t count = convertInt8ArrayToInt64(buf);
        ++count;
        (void)out.seekp(offset + FILE_BLOCK_META);
        convertUInt64ToInt8Array(count, buf);
        (void)out.write((char*)buf, 8);
        out.close();
    }

    void decrementFolderEntryCount(CoreBFSIO const &io,
                                   uint64_t const startBlock)
    {
        bfs::BFSImageStream out(io, std::ios::in | std::ios::out | std::ios::binary);
        uint64_t const offset = getOffsetOfFileBlock(startBlock, io.blocks);
        (void)out.seekg(offset + FILE_BLOCK_META);
        uint8_t buf[8];
        (void)out.read((char*)buf, 8);
        uint64_t count = convertInt8ArrayToInt64(buf);
        --count;
        (void)out.seekp(offset + FILE_BLOCK_META);
        convertUInt64ToInt8Array(count, buf);
        (void)out.write((char*)buf, 8);
        out.close();
    }

}
}

#endif // BFS_BFS_DETAIL_FOLDER_HPP__
