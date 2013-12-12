#ifndef BFS_BFS_DETAIL_FOLDER_HPP__
#define BFS_BFS_DETAIL_FOLDER_HPP__

#include "DetailFileBlock.hpp"

#include <fstream>
#include <iostream>
#include <stdint.h>
#include <vector>

namespace bfs { namespace detail
{

    void incrementFolderEntryCount(std::string const imagePath,
                                   uint64_t const startBlock,
                                   uint64_t const totalBlocks)
    {
        std::fstream out(imagePath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
        uint64_t const offset = getOffsetOfFileBlock(startBlock, totalBlocks);
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

    void decrementFolderEntryCount(std::string const imagePath,
                                   uint64_t const startBlock,
                                   uint64_t const totalBlocks)
    {
        std::fstream out(imagePath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
        uint64_t const offset = getOffsetOfFileBlock(startBlock, totalBlocks);
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
