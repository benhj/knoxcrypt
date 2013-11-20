#ifndef BFS_BFS_DETAIL_HPP__
#define BFS_BFS_DETAIL_HPP__

#include <boost/optional.hpp>

#include <fstream>
#include <iostream>
#include <stdint.h>
#include <vector>

namespace bfs { namespace detail
{

    uint64_t const METABLOCKS_BEGIN = 24;
    uint64_t const METABLOCK_SIZE = 25;
    uint64_t const MAX_FILENAME_LENGTH = 255;
    uint64_t const FILE_BLOCK_SIZE = 512;

    inline void convertInt64ToInt8Array(uint64_t const bigNum, uint8_t array[8])
    {
        array[0] = static_cast<uint8_t>((bigNum >> 56) & 0xFF);
        array[1] = static_cast<uint8_t>((bigNum >> 48) & 0xFF);
        array[2] = static_cast<uint8_t>((bigNum >> 40) & 0xFF);
        array[3] = static_cast<uint8_t>((bigNum >> 32) & 0xFF);
        array[4] = static_cast<uint8_t>((bigNum >> 24) & 0xFF);
        array[5] = static_cast<uint8_t>((bigNum >> 16) & 0xFF);
        array[6] = static_cast<uint8_t>((bigNum >> 8) & 0xFF);
        array[7] = static_cast<uint8_t>((bigNum) & 0xFF);
    }

    inline void convertInt32ToInt4Array(uint32_t const bigNum, uint8_t array[8])
    {
        array[0] = static_cast<uint8_t>((bigNum >> 24) & 0xFF);
        array[1] = static_cast<uint8_t>((bigNum >> 16) & 0xFF);
        array[2] = static_cast<uint8_t>((bigNum >> 8) & 0xFF);
        array[3] = static_cast<uint8_t>((bigNum) & 0xFF);
    }

    inline uint64_t convertInt8ArrayToInt64(uint8_t array[8])
    {
        return ((uint64_t)array[0] << 56) | ((uint64_t)array[1] << 48)  |
            ((uint64_t)array[2] << 40) | ((uint64_t)array[3] << 32) |
            ((uint64_t)array[4] << 24) | ((uint64_t)array[5] << 16)  |
            ((uint64_t)array[6] << 8) | ((uint64_t)array[7]);
    }

    /**
     * @brief gets the size of the BFS image
     * @param in the image stream
     * @return image size
     */
    inline uint64_t getImageSize(std::fstream &in)
    {
        in.seekg(0, in.end);
        uint64_t const bytes(in.tellg());
        return bytes;
    }

    /**
     * @brief get amount of space reserved for file data
     * @param in the image stream
     * @return amount of file space
     */
    inline uint64_t getBlockCount(std::fstream &in)
    {
        in.seekg(0, in.beg);
        uint8_t dat[8];
        (void)in.read((char*)dat, 8);
        return convertInt8ArrayToInt64(dat);
    }

    /**
     * @brief gets the number of files stored in the BFS
     * @param in the BFS image stream
     * @return the number of files
     */
    inline uint64_t getFileCount(std::fstream &in)
    {
        uint8_t blockCountBytes[8];
        in.seekg(0, in.beg);
        (void)in.read((char*)blockCountBytes, 8);
        uint64_t blockCount = convertInt8ArrayToInt64(blockCountBytes);
        in.seekg(blockCount + 8);
        uint8_t dat[8];
        (void)in.read((char*)dat, 8);
        return convertInt8ArrayToInt64(dat);
    }

    /**
     * @brief increments the file count
     * @param imagePath the path of the container
     */
    inline void incrementFileCount(std::fstream &in)
    {
        uint64_t orig = getFileCount(in);                    // convert to a 64 bit int
        ++orig;                                              // increment the count
        uint8_t countBytes[8];
        convertInt64ToInt8Array(orig, countBytes);           // convert back to bytes
        in.seekp((std::streampos)8);                         // seek to position 8
        in.write((char*)countBytes, 8);                      // write the bytes
    }

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
     * @param fileIndex the file index that this metablock represents
     * @return the offset of the meta data block
     */
    inline uint64_t getOffsetOfMetaDataBlock(uint64_t const fileIndex,
                                             uint64_t const totalBlocks)
    {
        uint64_t const volumeBitMapBytes = totalBlocks / 8;
        return 8                                                       // number of fs blocks
            + volumeBitMapBytes                 // volume bit map
            + 8                                                     // total number of files
            + (METABLOCK_SIZE * fileIndex); // meta data block
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
        uint64_t const offset = getOffsetOfMetaDataBlock(block, totalBlocks);
        (void)in.seekp(offset);
        uint8_t dat[8];
        convertInt64ToInt8Array(fileSize, dat);
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
        uint64_t const offset = getOffsetOfMetaDataBlock(fileIndex, totalBlocks);
        (void)in.seekp(offset + 8);
        uint8_t dat[8];
        convertInt64ToInt8Array(firstBlockOffset, dat);
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
        uint64_t const offset = getOffsetOfMetaDataBlock(fileIndex, totalBlocks);
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
        uint64_t const offset = getOffsetOfMetaDataBlock(fileIndex, totalBlocks);
        (void)in.seekg(offset + 8);
        uint8_t dat[8];
        (void)in.read((char*)dat, 8);
        return convertInt8ArrayToInt64(dat);
    }


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
            + getMetaDataSize(totalBlocks) // space occupied by metadata
            + (FILE_BLOCK_SIZE * block);   // file block
    }

    /**
     * @brief gets the file name of a particular entry
     * @param in the image stream
     * @param n the file index
     * @return the file name
     */
    inline std::string getFileNameForFileN(std::fstream &in, uint64_t const n)
    {
        /*
          uint64_t fileOffset = getOffsetOfFileN(in, n);
          (void)in.seekg((std::streampos)fileOffset);
          uint8_t dat[MAX_FILENAME_LENGTH];
          (void)in.read((char*)dat, MAX_FILENAME_LENGTH);
          std::string name("");
          for(int i = 0; i < MAX_FILENAME_LENGTH; ++i) {
          if((char)dat[i] != '\0') {
          name.push_back((char)dat[i]);
          } else {
          break;
          }
          }
          return name;
        */
    }

    /**
     * @brief gets the number of blocks in this bfs
     * @param in the bfs image stream
     * @return the number of blocks
     */
    inline uint64_t getNumberOfBlocks(std::fstream &in)
    {
        (void)in.seekg(0, in.beg);
        uint8_t dat[8];
        (void)in.read((char*)dat, 8);
        return convertInt8ArrayToInt64(dat);
    }

    /**
     * @brief determines whether a bit is set in a byte
     * @param byte the byte to test
     * @param bit the bit index to check
     * @return true bit is set false otherwise
     */
    inline bool isBitSetInByte(uint8_t const byte, int const bit)
    {
        return (byte & (1 << bit));
    }

    /**
     * @brief gets the index of the first unset bit from the byte
     * @param byte the byte to search
     * @return the index
     */
    inline int getNextAvailableBitInAByte(uint8_t const byte)
    {
        for (int i = 0; i < 8; ++i) {
            if (!isBitSetInByte(byte, i)) {
                return i;
            }
        }
        return -1; // none available
    }

    /**
     * @brief set or clear a bit
     * @param byte the byte to set or clear the bit of
     * @param bit the bit to set or clear
     * @param set whether to set, true be default; false will clear the bit
     */
    inline void setBitInByte(uint8_t &byte, int const bit, bool const set = true)
    {
        if (set) {
            byte |= 1 << bit;
        } else {
            byte &= ~(1 << bit);
        }
    }

    /**
     * @brief sets a block to in use in bit map representation
     * @param block the block to set in the bit map to 'in use'
     * @param blocks the number of blocks in this fs
     * @param in the image stream
     */
    inline void setBlockToInUse(uint64_t const block,
                                uint64_t const blocks,
                                std::fstream &in,
                                bool const set = true)
    {
        uint64_t bytes = blocks / uint64_t(8);
        // read the bytes in to a buffer
        std::vector<uint8_t> buf;
        buf.assign(bytes, 0);
        (void)in.seekg(8);
        (void)in.read((char*)&buf.front(), bytes);

        uint64_t byteThatStoresBit(0);
        if (block < 8) {

            (void)in.seekg(8);
            uint8_t dat = buf[byteThatStoresBit];
            setBitInByte(dat, block, set);
            (void)in.seekp(8);
            (void)in.write((char*)&dat, 1);

        } else {
            (void)in.seekg(8);
            uint64_t const leftOver = block % 8;
            uint64_t withoutLeftOver = block - leftOver;
            byteThatStoresBit = (withoutLeftOver / 8) - 1;
            ++byteThatStoresBit;
            uint8_t dat = buf[byteThatStoresBit];
            setBitInByte(dat, leftOver, set);
            (void)in.seekp(8 + byteThatStoresBit);
            (void)in.write((char*)&dat, 1);
        }
    }

    /**
     * @brief gets the the next available block
     * @param in the image stream
     * @return the next available block
     */
    typedef boost::optional<uint64_t> OptionalBlock;
    inline OptionalBlock getNextAvailableBlock(std::fstream &in)
    {
        // get number of blocks that make up fs
        uint64_t blocks = getNumberOfBlocks(in);

        // how many bytes does this value fit in to?
        uint64_t bytes = blocks / uint64_t(8);

        // read the bytes in to a buffer
        std::vector<uint8_t> buf;
        buf.assign(bytes, 0);
        (void)in.read((char*)&buf.front(), bytes);

        // find out the next available bit
        uint64_t bitCounter(0);

        for (uint64_t i = 0; i < bytes; ++i) {
            int availableBit = getNextAvailableBitInAByte(buf[i]);
            if (availableBit > -1) {
                bitCounter += availableBit;
                break;
            } else {
                bitCounter += 8;
            }
        }

        // no available blocks found
        if (bitCounter == blocks) {
            return OptionalBlock();
        }

        // next available block == bitCounter
        // derive offset from bit counter. Note blocks
        // to be stored starting at 0 index.
        // Metadata starts at 8 (block count) + 8 (file count)
        // + bytes (volume bit map size) + meta data size
        //uint64_t const offset = 8 + 8 + bytes + getMetaDataSize(blocks) + (FILE_BLOCK_SIZE * bitCounter);

        return OptionalBlock(bitCounter);
    }

    /**
     * @brief determines whether a meta block is available by reading the very first bit
     * @param in the image stream
     * @param metablock meta block to determine its availability
     * @return true if metablock is available, false otherwise
     */
    inline bool metaBlockIsAvailable(std::fstream &in, uint64_t const metaBlock)
    {
        // get number of blocks that make up fs
        uint64_t blocks = getNumberOfBlocks(in);

        uint64_t offset = 8 + blocks + 8 + (metaBlock * METABLOCK_SIZE);
        (void)in.seekg(offset);

        uint8_t byte;
        (void)in.read((char*)&byte, 1);

        return (!isBitSetInByte(byte, 0));
    }

    /**
     * @brief get N available file blocks if they're available
     * @param in the bfs image stream
     * @param blocksRequired the number of file blocks required
     * @param totalBlocks the total number of blocks in the bfs
     * @return a vector of available file block indices
     */
    inline std::vector<uint64_t> getNAvailableBlocks(std::fstream &in,
                                                     uint64_t const blocksRequired,
                                                     uint64_t const totalBlocks)
    {
        // how many bytes does this value fit in to?
        uint64_t bytes = totalBlocks / uint64_t(8);

        // read the bytes in to a buffer
        std::vector<uint8_t> buf;
        buf.assign(bytes, 0);
        (void)in.read((char*)&buf.front(), bytes);

        // find n available blocks
        std::vector<uint64_t> bitBuffer;
        for (uint64_t i = 0; i < bytes; ++i) {
            int availableBit = getNextAvailableBitInAByte(buf[i]);
            if (availableBit > -1) {
                bitBuffer.push_back((uint64_t)availableBit);
                if(bitBuffer.size() == blocksRequired) {
                    return bitBuffer;
                }
            }
        }
        return std::vector<uint64_t>(); // empty not enough free blocks found
    }
}
}

#endif // BFS_BFS_DETAIL_HPP__
