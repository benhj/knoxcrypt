#ifndef BFS_BFS_DETAIL_META_HPP__
#define BFS_BFS_DETAIL_META_HPP__

#include <fstream>
#include <iostream>
#include <stdint.h>
#include <vector>

namespace bfs { namespace detail
{


    /**
     * @brief gets the number of meta blocks used to store file meta data
     * computed as a fraction of the total number of bytes in the file space
     * @note the number of metablocks imposes a limit on the total number of
     * permitted files
     * @param blocks the total number of fs blocks
     * @return the number of meta blocks
     */
    inline uint64_t getMetaBlockCount(uint64_t const blocks)
    {
    	uint64_t metaBlockCount = ((blocks * FILE_BLOCK_SIZE) * 0.001);
    	uint64_t leftOver = metaBlockCount % 2;
    	if(leftOver != 0) {
    		metaBlockCount -= leftOver;
    	}
    	return metaBlockCount;
    }

    /**
     * @brief gets the total amount of space allocated to metadata
     * @param blocks number of blocks making up image
     * @return space allocated to metadata
     */
    inline uint64_t getMetaDataSize(uint64_t const blocks)
    {
        return getMetaBlockCount(blocks) * METABLOCK_SIZE;
    }

    /**
     * @brief gets the offset of a given meta data block
     * @param metaBlockIndex the index of the metablock
     * @return the offset of the meta block
     */
    inline uint64_t getOffsetOfMetaBlock(uint64_t const metaBlockIndex,
                                        uint64_t const totalBlocks)
    {
        uint64_t const volumeBitMapBytes = totalBlocks / 8;
        return 8 + volumeBitMapBytes + 8 + (METABLOCK_SIZE * metaBlockIndex);
    }

    /**
     * @brief writes the file size to metadata of given file
     * @param fileIndex index of associated file
     * @param totalBlocks total blocks in fs image
     * @param fileSize size of associated file
     * @param in the bfs image stream
     */
    inline void writeFileSizeToMetaBlock(uint64_t const block,
                                         uint64_t const totalBlocks,
                                         uint64_t const fileSize,
                                         std::fstream &in)
    {
        uint64_t const offset = getOffsetOfMetaBlock(block, totalBlocks);
        (void)in.seekp(offset);
        uint8_t dat[8];
        convertUInt64ToInt8Array(fileSize, dat);
        (void)in.write((char*)dat, 8);
    }

    /**
     * @brief writes the first block offset to metadata of given file
     * @param fileIndex index of associated file
     * @param totalBlocks total blocks in fs image
     * @param firstBlockOffset offset of first file block
     * @param in the bfs image stream
     */
    inline void writeFirstBlockOffsetToMetaBlock(uint64_t const fileIndex,
                                                 uint64_t const totalBlocks,
                                                 uint64_t const firstBlockOffset,
                                                 std::fstream &in)
    {
        uint64_t const offset = getOffsetOfMetaBlock(fileIndex, totalBlocks) + 8 + 1;
        (void)in.seekp(offset);
        uint8_t dat[8];
        convertUInt64ToInt8Array(firstBlockOffset, dat);
        (void)in.write((char*)dat, 8);
    }

    /**
     * @brief returns the file size of the file that meta data is associated with
     * @param fileIndex the index of the file to read the metadata of
     * @param totalBlocks the total number of file blocks
     * @param in the bfs image stream
     * @return the file size
     */
    inline uint64_t readFileSizeFromMetaBlock(uint64_t const fileIndex,
                                              uint64_t const totalBlocks,
                                              std::fstream &in)
    {
        uint64_t const offset = getOffsetOfMetaBlock(fileIndex, totalBlocks) + 1;
        (void)in.seekg(offset);
        uint8_t dat[8];
        (void)in.read((char*)dat, 8);
        return convertInt8ArrayToInt64(dat);
    }

    /**
     * @brief returns the index of the first file block as stored
     * in the file's metadata
     * @param fileIndex the index of the file to read the metadata of
     * @param totalBlocks the total number of file blocks
     * @param in the bfs image stream
     * @return the offset value
     */
    inline uint64_t readFirstBlockIndexFromMetaBlock(uint64_t const fileIndex,
                                                      uint64_t const totalBlocks,
                                                      std::fstream &in)
    {
        uint64_t const offset = getOffsetOfMetaBlock(fileIndex, totalBlocks) + 1 + 8;
        (void)in.seekg(offset);
        uint8_t dat[8];
        (void)in.read((char*)dat, 8);
        return convertInt8ArrayToInt64(dat);
    }

    /**
     * @brief determines whether a meta block is available by reading the very first bit
     * @param in the image stream
     * @param metablock meta block to determine its availability
     * @return true if metablock is available, false otherwise
     */
    inline bool metaBlockIsAvailable(std::fstream &in, uint64_t const metaBlock, uint64_t const totalBlocks = 0)
    {

        uint64_t const offset = getOffsetOfMetaBlock(metaBlock, totalBlocks);
        (void)in.seekg(offset);
        uint8_t byte;
        (void)in.read((char*)&byte, 1);
        return (!isBitSetInByte(byte, 0));
    }

    inline uint64_t getNextAvailableMetaBlock(std::fstream &in, uint64_t const totalFileBlocks)
    {
        uint64_t const metaBlockCount = getMetaBlockCount(totalFileBlocks);
        for(uint64_t b(0); b < metaBlockCount; ++b) {
            if(metaBlockIsAvailable(in, b, totalFileBlocks)) {
                return b;
            }
        }

        // maybe throw here?
    }

}
}

#endif // BFS_BFS_DETAIL_META_HPP__
