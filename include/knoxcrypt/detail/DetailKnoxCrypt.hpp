/*
  Copyright (c) <2013-2016>, <BenHJ>
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

#include "knoxcrypt/ContainerImageStream.hpp"

#include <boost/optional.hpp>

#include <iostream>
#include <stdint.h>
#include <vector>
#include <strings.h>

namespace knoxcrypt { namespace detail
{

    uint64_t const METABLOCKS_BEGIN = 24;
    uint64_t const METABLOCK_SIZE = 25;
    uint64_t const MAX_FILENAME_LENGTH = 255;
    uint64_t const FILE_BLOCK_META = 12;
    uint64_t const IV_BYTES = 8;
    uint64_t const HEADER_BYTES = 8;
    long     const CIPHER_BUFFER_SIZE = 270000000;
    uint64_t const PASS_HASH_BYTES = 32;

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
        return (IV_BYTES * 4) + HEADER_BYTES + PASS_HASH_BYTES;
    }

    /**
     * @brief gets the size of the knoxcrypt image
     * @param in the image stream
     * @return image size
     */
    inline void getPassHash(knoxcrypt::ContainerImageStream &in, uint8_t hash[32])
    {
        in.seekg(beginning() - PASS_HASH_BYTES);
        (void)in.read((char*)hash, 32);
    }

    /**
     * @brief gets the password hash from the container
     * @param in the image stream
     * @return a string of the hash
     */
    inline uint64_t getImageSize(knoxcrypt::ContainerImageStream &in)
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
    inline uint64_t getBlockCount(knoxcrypt::ContainerImageStream &in)
    {
        in.seekg(beginning());
        uint8_t dat[8];
        (void)in.read((char*)dat, 8);
        return convertInt8ArrayToInt64(dat);
    }

    /**
     * @brief gets the number of blocks in this knoxcrypt
     * @param in the knoxcrypt image stream
     * @return the number of blocks
     */
    inline uint64_t getNumberOfBlocks(knoxcrypt::ContainerImageStream &in)
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
        if (byte == 0xFF) { return -1; }
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
                                uint64_t const,// blocks,
                                knoxcrypt::ContainerImageStream &in,
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
                             knoxcrypt::ContainerImageStream &in)
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
            if (dat == 0xFF) {
                return true;
            }
            return isBitSetInByte(dat, block);
        } else {
            uint64_t const leftOver = block % 8;
            uint64_t withoutLeftOver = block - leftOver;
            byteThatStoresBit = (withoutLeftOver / 8) - 1;
            ++byteThatStoresBit;
            uint8_t &dat = buf[byteThatStoresBit];
            if (dat == 0xFF) {
                return true;
            }
            return isBitSetInByte(dat, leftOver);
        }
    }

    /**
     * @brief gets the number of blocks currently allocated
     * @param in the knoxcrypt image stream
     * @return the number of allocated blocks
     */
    inline uint64_t getNumberOfAllocatedBlocks(knoxcrypt::ContainerImageStream &in)
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

        // note this is quicker than calling isBlockInUse repeatedly
        uint64_t allocatedBlocks(0);
        for (uint64_t byte = 0; byte < bytes; ++byte) {
            uint8_t dat = buf[byte];
            if(dat == 0xFF) {
                allocatedBlocks += 8;
                continue;
            } 
            for(int i = 0; i < 8; ++i) {
                if(isBitSetInByte(dat, i)) {
                    ++allocatedBlocks;
                }
            }
        }

        return allocatedBlocks;
    }

    /**
     * @brief gets the the next available block
     * @param in the image stream
     * @return the next available block
     */
    using OptionalBlock = boost::optional<uint64_t>;
    inline OptionalBlock getNextAvailableBlock(knoxcrypt::ContainerImageStream &in, uint64_t const blocks_ = 0)
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
        std::vector<uint8_t> buf(bytes);
        (void)in.read((char*)&buf.front(), bytes);

        // find out the next available bit
        uint64_t bitCounter(0);

        for (uint64_t i = 0; i < bytes; ++i) {
            int availableBit = getNextAvailableBitInAByte(buf[i]);
            if(availableBit > -1) {
                bitCounter += availableBit;
                break;
            }
            bitCounter += 8;
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
     * @param in the knoxcrypt image stream
     * @param blocksRequired the number of file blocks required
     * @param totalBlocks the total number of blocks in the knoxcrypt
     * @return a vector of available file block indices. Note this might
     * be less that blocksRequired if there are not enough blocks available
     */
    inline std::vector<uint64_t> getNAvailableBlocks(knoxcrypt::ContainerImageStream &in,
                                                     uint64_t const blocksRequired,
                                                     uint64_t const totalBlocks)
    {
        // how many bytes does this value fit in to?
        uint64_t bytes = totalBlocks / uint64_t(8);

        // read the bytes in to a buffer
        std::vector<uint8_t> buf(bytes);
        (void)in.seekg(beginning() + 8);
        (void)in.read((char*)&buf.front(), bytes);


        // find n available blocks
        uint64_t eightCounter(0);
        std::vector<uint64_t> bitBuffer(blocksRequired);
        uint64_t filled(0);
        for (uint64_t i = 0; i < bytes; ++i) {
            // only continue if at least one bit available
            if(buf[i] != 0xFF) {
                for (int b = 0; b < 8; ++b) {
                    if(!isBitSetInByte(buf[i], b)) {
                        bitBuffer[filled] = b + eightCounter;
                        ++filled;
                        if (filled == blocksRequired) {
                            return bitBuffer;
                        }
                    }
                }
            } 
            
            eightCounter += 8;
        }
        return bitBuffer; // return all blocks that could be found
    }

    /**
     * @brief updates the volume bit map with newly allocated file blocks
     * @param in the knoxcrypt image stream
     * @param blocksUsed a vector of newly allocated file block indices
     * @param totalBlocks total number of fs blocks
     */
    inline void updateVolumeBitmap(ContainerImageStream &in,
                                   std::vector<uint64_t> const &blocksUsed,
                                   uint64_t const totalBlocks)
    {
        for (auto const & it : blocksUsed) {
            setBlockToInUse(it, totalBlocks, in);
        }
    }

    /**
     * @brief updates the volume bit map with newly allocated file blocks
     * @param in the knoxcrypt image stream
     * @param blockUsed the used block
     * @param totalBlocks total number of fs blocks
     */
    inline void updateVolumeBitmapWithOne(ContainerImageStream &in,
                                          uint64_t const &blockUsed,
                                          uint64_t const totalBlocks,
                                          bool const set = true)
    {
        setBlockToInUse(blockUsed, totalBlocks, in, set);
    }

    /**
     * @brief  checks the g position of the stream and updates if necessary
     * @param  stream the stream to update
     * @param  offset the position to seek to
     * @return true seeking was successful, false otherwise
     */
    inline bool checkAndSeekG(ContainerImageStream &stream, std::streamoff offset)
    {
        if(stream.tellg() != offset) {
            bool bad = stream.seekg(offset).bad();
            return (!bad);
        }
        return true;
    }

    /**
     * @brief checks the p position of the stream and updates if necessary
     * @param stream the stream to update
     * @param offset the position to seek to
     * @return true seeking was successful, false otherwise
     */
    inline bool checkAndSeekP(ContainerImageStream &stream, std::streamoff offset)
    {
        if(stream.tellp() != offset) {
            bool bad = stream.seekp(offset).bad();
            return (!bad);
        }
        return true;
    }

    /**
     * @brief reads the initialization vector and number of encryption rounds
     * from a knoxcrypt image and sets the io's iv and rounds fields accordingly
     * @param io the core io to be populated with the iv and rounds
     */
    inline void readImageIVAndRounds(SharedCoreIO &io)
    {
        std::ifstream in(io->path.c_str(), std::ios::in | std::ios::binary);
        std::vector<uint8_t> ivBuffer;
        ivBuffer.resize(8);
        std::vector<uint8_t> ivBuffer2;
        ivBuffer2.resize(8);
        std::vector<uint8_t> ivBuffer3;
        ivBuffer3.resize(8);
        std::vector<uint8_t> ivBuffer4;
        ivBuffer4.resize(8);
        (void)in.read((char*)&ivBuffer.front(), knoxcrypt::detail::IV_BYTES);
        (void)in.read((char*)&ivBuffer2.front(), knoxcrypt::detail::IV_BYTES);
        (void)in.read((char*)&ivBuffer3.front(), knoxcrypt::detail::IV_BYTES);
        (void)in.read((char*)&ivBuffer4.front(), knoxcrypt::detail::IV_BYTES);
        char i;
        (void)in.read((char*)&i, 1);
        char j;
        (void)in.read((char*)&j, 1);
        unsigned int cipher = (unsigned int)j;
        // note, i should always > 0 <= 255
        io->rounds = (unsigned int)i;

        if(cipher == 1) {
            io->encProps.cipher = cryptostreampp::Algorithm::AES;
        } else if(cipher == 2) {
            io->encProps.cipher = cryptostreampp::Algorithm::Twofish;
        } else if(cipher == 3) {
            io->encProps.cipher = cryptostreampp::Algorithm::Serpent;
        } else if(cipher == 4) {
            io->encProps.cipher = cryptostreampp::Algorithm::RC6;
        } else if(cipher == 5) {
            io->encProps.cipher = cryptostreampp::Algorithm::MARS;
        } else if(cipher == 6) {
            io->encProps.cipher = cryptostreampp::Algorithm::CAST256;
        } else if(cipher == 7) {
            io->encProps.cipher = cryptostreampp::Algorithm::Camellia;
        } else if(cipher == 8) {
            io->encProps.cipher = cryptostreampp::Algorithm::RC5;
        } else if(cipher == 9) {
            io->encProps.cipher = cryptostreampp::Algorithm::SHACAL2;
        } else if(cipher == 10) {
            io->encProps.cipher = cryptostreampp::Algorithm::Blowfish;
        } else if(cipher == 11) {
            io->encProps.cipher = cryptostreampp::Algorithm::SKIPJACK;
        } else if(cipher == 12) {
            io->encProps.cipher = cryptostreampp::Algorithm::IDEA;
        } else if(cipher == 13) {
            io->encProps.cipher = cryptostreampp::Algorithm::SEED;
        } else if(cipher == 14) {
            io->encProps.cipher = cryptostreampp::Algorithm::TEA;
        } else if(cipher == 15) {
            io->encProps.cipher = cryptostreampp::Algorithm::XTEA;
        } else if(cipher == 16) {
            io->encProps.cipher = cryptostreampp::Algorithm::DES_EDE2;
        } else if(cipher == 17) {
            io->encProps.cipher = cryptostreampp::Algorithm::DES_EDE3;
        } else if(cipher == 0) {
            io->encProps.cipher = cryptostreampp::Algorithm::NONE;
        }

        in.close();
        io->encProps.iv = knoxcrypt::detail::convertInt8ArrayToInt64(&ivBuffer.front());
        io->encProps.iv2 = knoxcrypt::detail::convertInt8ArrayToInt64(&ivBuffer2.front());
        io->encProps.iv3 = knoxcrypt::detail::convertInt8ArrayToInt64(&ivBuffer3.front());
        io->encProps.iv4 = knoxcrypt::detail::convertInt8ArrayToInt64(&ivBuffer4.front());
    }

}
}
