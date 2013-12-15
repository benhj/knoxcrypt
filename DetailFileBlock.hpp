#ifndef BFS_BFS_DETAIL_FILE_BLOCK_HPP__
#define BFS_BFS_DETAIL_FILE_BLOCK_HPP__

#include "BFSImageStream.hpp"
#include "DetailBFS.hpp"

#include <iostream>
#include <stdint.h>
#include <vector>

namespace bfs { namespace detail
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
     * @param in the bfs image stream
     * @param n the file block to get the next index from
     * @param totalBlocks the total number of blocks in the bfs
     * @return the next file block index
     */
    inline uint64_t getIndexOfNextFileBlockFromFileBlockN(bfs::BFSImageStream &in,
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
     * @param in the bfs image stream
     * @param n the file block to get the next index from
     * @param totalBlocks the total number of blocks in the bfs
     * @return the next file block index
     */
    inline uint32_t getNumberOfDataBytesWrittenToFileBlockN(bfs::BFSImageStream &in,
                                                            uint64_t const n,
                                                            uint64_t const totalBlocks)
    {
        uint64_t offset = getOffsetOfFileBlock(n, totalBlocks);
        (void)in.seekg(offset);
        uint8_t dat[4];
        (void)in.read((char*)dat, 4);
        return convertInt4ArrayToInt32(dat);
    }

    inline void writeNumberOfDataBytesWrittenToFileBlockN(bfs::BFSImageStream &in,
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

    inline void writeIndexOfNextFileBlockFromFileBlockN(bfs::BFSImageStream &in,
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

#endif // BFS_BFS_DETAIL_FILE_BLOCK_HPP__
