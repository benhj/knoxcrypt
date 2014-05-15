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

#ifndef TeaSafe_TeaSafe_DETAIL_TeaSafe_HPP__
#define TeaSafe_TeaSafe_DETAIL_TeaSafe_HPP__

#include "teasafe/TeaSafeImageStream.hpp"

#include <boost/optional.hpp>

#include <iostream>
#include <stdint.h>
#include <vector>

namespace teasafe { namespace detail
{

    uint64_t const METABLOCKS_BEGIN = 24;
    uint64_t const METABLOCK_SIZE = 25;
    uint64_t const MAX_FILENAME_LENGTH = 255;
    uint64_t const FILE_BLOCK_SIZE = 4096;
    uint64_t const FILE_BLOCK_META = 12;
    uint64_t const IV_BYTES = 8;
    uint64_t const HEADER_BYTES = 8;

    inline void convertUInt64ToInt8Array(uint64_t const bigNum, uint8_t array[8])
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

    inline uint32_t convertInt4ArrayToInt32(uint8_t array[4])
    {
        return ((uint32_t)array[0] << 24) | ((uint32_t)array[1] << 16)  |
            ((uint32_t)array[2] << 8) | ((uint32_t)array[3]);
    }

    /**
     * @brief get where the main encrypted bytes starts, i.e. after the initial
     * iv data
     * @return
     */
    inline uint64_t beginning()
    {
        return IV_BYTES + HEADER_BYTES;
    }

    /**
     * @brief gets the size of the TeaSafe image
     * @param in the image stream
     * @return image size
     */
    inline uint64_t getImageSize(teasafe::TeaSafeImageStream &in)
    {
        in.seekg(0, std::ios::end);
        uint64_t const bytes(in.tellg());
        return bytes;
    }

    /**
     * @brief get amount of space reserved for file data
     * @param in the image stream
     * @return amount of file space
     */
    inline uint64_t getBlockCount(teasafe::TeaSafeImageStream &in)
    {
        in.seekg(beginning());
        uint8_t dat[8];
        (void)in.read((char*)dat, 8);
        return convertInt8ArrayToInt64(dat);
    }

    /**
     * @brief gets the number of blocks in this teasafe
     * @param in the teasafe image stream
     * @return the number of blocks
     */
    inline uint64_t getNumberOfBlocks(teasafe::TeaSafeImageStream &in)
    {
        (void)in.seekg(beginning());
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
    inline bool isBitSetInByte(uint8_t &byte, int const bit)
    {
        return (byte & (1 << bit));
    }

    /**
     * @brief gets the index of the first unset bit from the byte
     * @param byte the byte to search
     * @return the index
     */
    inline int getNextAvailableBitInAByte(uint8_t &byte)
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
                                teasafe::TeaSafeImageStream &in,
                                bool const set = true)
    {
        uint64_t byteThatStoresBit(0);
        if (block < 8) {

            (void)in.seekg(beginning() + 8);
            uint8_t dat;
            (void)in.read((char*)&dat, 1);
            setBitInByte(dat, block, set);
            (void)in.seekp(beginning() + 8);
            (void)in.write((char*)&dat, 1);

        } else {

            uint64_t const leftOver = block % 8;
            uint64_t withoutLeftOver = block - leftOver;
            byteThatStoresBit = (withoutLeftOver / 8) - 1;
            ++byteThatStoresBit;
            (void)in.seekg(beginning() + 8 + byteThatStoresBit);
            uint8_t dat;
            (void)in.read((char*)&dat, 1);
            setBitInByte(dat, leftOver, set);
            (void)in.seekp(beginning() + 8 + byteThatStoresBit);
            (void)in.write((char*)&dat, 1);
        }
        in.flush();
    }

    /**
     * @brief determines whether a file block is in use
     * @param block the block to determine if in use
     * @param blocks the total number of file blocks
     * @param in the stream to read from
     * @return true if allocated, false otherwise
     */
    inline bool isBlockInUse(uint64_t const block,
                             uint64_t const blocks,
                             teasafe::TeaSafeImageStream &in)
    {
        uint64_t bytes = blocks / uint64_t(8);
        // read the bytes in to a buffer
        std::vector<uint8_t> buf;
        buf.assign(bytes, 0);
        (void)in.seekg(beginning() + 8);
        (void)in.read((char*)&buf.front(), bytes);

        uint64_t byteThatStoresBit(0);
        if (block < 8) {
            uint8_t dat = buf[byteThatStoresBit];
            return isBitSetInByte(dat, block);
        } else {
            uint64_t const leftOver = block % 8;
            uint64_t withoutLeftOver = block - leftOver;
            byteThatStoresBit = (withoutLeftOver / 8) - 1;
            ++byteThatStoresBit;
            uint8_t &dat = buf[byteThatStoresBit];
            return isBitSetInByte(dat, leftOver);
        }
    }

    /**
     * @brief gets the number of blocks currently allocated
     * @param in the teasafe image stream
     * @return the number of allocated blocks
     */
    inline uint64_t getNumberOfAllocatedBlocks(teasafe::TeaSafeImageStream &in)
    {
        (void)in.seekg(beginning());
        uint8_t dat[8];
        (void)in.read((char*)dat, 8);
        uint64_t blocks = convertInt8ArrayToInt64(dat);
        uint64_t bytes = blocks / uint64_t(8);
        // read the bytes in to a buffer
        std::vector<uint8_t> buf;
        buf.assign(bytes, 0);
        (void)in.read((char*)&buf.front(), bytes);
        uint64_t used(0);
        // note this is quicker than calling isBlockInUse repeatedly
        for (uint64_t block = 0; block < blocks; ++block) {
            uint64_t byteThatStoresBit(0);
            if (block < 8) {
                uint8_t dat = buf[byteThatStoresBit];
                if (isBitSetInByte(dat, block)) {
                    ++used;
                }
            } else {
                uint64_t const leftOver = block % 8;
                uint64_t withoutLeftOver = block - leftOver;
                byteThatStoresBit = (withoutLeftOver / 8) - 1;
                ++byteThatStoresBit;
                uint8_t &dat = buf[byteThatStoresBit];
                if (isBitSetInByte(dat, leftOver)) {
                    ++used;
                }
            }
        }

        return used;
    }

    /**
     * @brief gets the the next available block
     * @param in the image stream
     * @return the next available block
     */
    typedef boost::optional<uint64_t> OptionalBlock;
    inline OptionalBlock getNextAvailableBlock(teasafe::TeaSafeImageStream &in, uint64_t const blocks_ = 0)
    {
        // get number of blocks that make up fs
        uint64_t blocks = blocks_;
        if (blocks == 0) {
            blocks = getNumberOfBlocks(in);
        } else {
            (void)in.seekg(beginning() + 8);
        }

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
     * @brief get N available file blocks if they're available
     * @param in the teasafe image stream
     * @param blocksRequired the number of file blocks required
     * @param totalBlocks the total number of blocks in the teasafe
     * @return a vector of available file block indices. Note this might
     * be less that blocksRequired if there are not enough blocks available
     */
    inline std::vector<uint64_t> getNAvailableBlocks(teasafe::TeaSafeImageStream &in,
                                                     uint64_t const blocksRequired,
                                                     uint64_t const totalBlocks)
    {
        // how many bytes does this value fit in to?
        uint64_t bytes = totalBlocks / uint64_t(8);

        // read the bytes in to a buffer
        std::vector<uint8_t> buf;
        buf.assign(bytes, 0);
        (void)in.seekg(beginning() + 8);
        (void)in.read((char*)&buf.front(), bytes);


        // find n available blocks
        uint64_t eightCounter(0);
        std::vector<uint64_t> bitBuffer;
        for (uint64_t i = 0; i < bytes; ++i) {

            for (int b = 0; b < 8; ++b) {
                int availableBit = (!isBitSetInByte(buf[i], b)) ? (b + eightCounter) : -1;
                if (availableBit > -1) {
                    bitBuffer.push_back((uint64_t)availableBit);
                    if (bitBuffer.size() == blocksRequired) {
                        return bitBuffer;
                    }
                }
            }
            eightCounter += 8;
        }
        return bitBuffer; // return all blocks that could be found
    }

    /**
     * @brief updates the volume bit map with newly allocated file blocks
     * @param in the teasafe image stream
     * @param blocksUsed a vector of newly allocated file block indices
     * @param totalBlocks total number of fs blocks
     */
    inline void updateVolumeBitmap(TeaSafeImageStream &in,
                                   std::vector<uint64_t> const &blocksUsed,
                                   uint64_t const totalBlocks)
    {
        // how many bytes does this value fit in to?
        uint64_t bytes = totalBlocks / uint64_t(8);
        std::vector<uint64_t>::const_iterator it = blocksUsed.begin();
        for (; it != blocksUsed.end(); ++it) {
            setBlockToInUse(*it, totalBlocks, in);
        }
    }

    /**
     * @brief updates the volume bit map with newly allocated file blocks
     * @param in the teasafe image stream
     * @param blockUsed the used block
     * @param totalBlocks total number of fs blocks
     */
    inline void updateVolumeBitmapWithOne(TeaSafeImageStream &in,
                                          uint64_t const &blockUsed,
                                          uint64_t const totalBlocks,
                                          bool const set = true)
    {
        // how many bytes does this value fit in to?
        uint64_t bytes = totalBlocks / uint64_t(8);
        setBlockToInUse(blockUsed, totalBlocks, in, set);
    }

    /**
     * @brief checks the g position of the stream and updates if necessary
     * @param stream the stream to update
     * @param offset the position to seek to
     */
    inline void checkAndSeekG(TeaSafeImageStream &stream, uint64_t offset)
    {
        if(stream.tellg() != offset) {
            (void)stream.seekg(offset);
        }
    }

    /**
     * @brief checks the p position of the stream and updates if necessary
     * @param stream the stream to update
     * @param offset the position to seek to
     */
    inline void checkAndSeekP(TeaSafeImageStream &stream, uint64_t offset)
    {
        if(stream.tellp() != offset) {
            (void)stream.seekp(offset);
        }
    }

}
}

#endif // TeaSafe_TeaSafe_DETAIL_TeaSafe_HPP__
