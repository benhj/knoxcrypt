#ifndef BFS_MAKE_BFS_HPP__
#define BFS_MAKE_BFS_HPP__

#include "DetailBFS.hpp"
#include "DetailFileBlock.hpp"

#include <string>
#include <fstream>
#include <vector>
#include <iostream>

namespace bfs
{

    class MakeBFS
    {
      public:

        MakeBFS(std::string const &imageName, uint64_t const blocks)
        {
            buildImage(imageName, blocks);
        }

      private:
        MakeBFS(); // not required

        void buildBlockBytes(uint64_t const fsSize, uint8_t sizeBytes[8])
        {
            detail::convertInt64ToInt8Array(fsSize, sizeBytes);
        }

        void buildFileCountBytes(uint64_t const fileCount, uint8_t sizeBytes[8])
        {
            detail::convertInt64ToInt8Array(fileCount, sizeBytes);
        }

        void writeOutFileSpaceBytes(uint64_t const fileBlockCount, std::fstream &out)
        {
            for(uint64_t i(0); i < fileBlockCount ; ++i) {
                std::vector<uint8_t> ints;
                ints.assign(detail::FILE_BLOCK_SIZE, 0);
                (void)out.write((char*)&ints.front(), detail::FILE_BLOCK_SIZE);
            }
        }

        void zeroOutBits(std::vector<uint8_t> &bitMapData)
        {
            uint8_t byte;
            for (int i = 0; i < 8; ++i) {
                detail::setBitInByte(byte, i, false);
            }
            bitMapData.push_back(byte);
        }

        /**
         *
         * @param blocks
         */
        void createVolumeBitMap(uint64_t const blocks, std::fstream &out)
        {
            //
            // each block will be represented by a bit. If allocated this
            // bit will be set to 1. If not allocated it will be set to 0
            // So we need blocks bits
            // Store the bits in uint8_t. Each of these is a byte so we need
            // blocks divided by 8 to get the number of bytes required.
            // All initialized to zero
            //
            uint64_t bytesRequired = blocks / uint64_t(8);
            std::vector<uint8_t> bitMapData;
            for (uint64_t b = 0; b < bytesRequired; ++b) {
                zeroOutBits(bitMapData);
            }
            (void)out.write((char*)&bitMapData.front(), bytesRequired);
        }

        void writeRootDirectory(uint64_t const blocks, std::fstream &out)
        {
            // the entry count of the root directory (start out at 0 entries)
            uint64_t startCount = 0;
            uint8_t buf[8];
            detail::convertInt64ToInt8Array(startCount, buf);

            // write out entry count
            (void)out.write((char*)buf, 8);
        }

        /**
         * @brief build the file system image
         *
         * The first 8 bytes will represent the number of blocks in the FS
         * The next blocks bits will represent the volume bit map
         * The next 8 bytes will represent the total number of files
         * The next data will be metadata computed as a fraction of the fs
         * size and number of blocks
         * The remaining bytes will be reserved for actual file data 512 byte blocks
         *
         * @param imageName the name of the image
         * @param blocks the number of blocks in the file system
         */
        void buildImage(std::string const &imageName, uint64_t const blocks)
        {

            //
            // store the number of blocks in the first 8 bytes of the superblock
            //
            uint8_t sizeBytes[8];
            buildBlockBytes(blocks, sizeBytes);

            // write out size, and volume bitmap bytes
            {
                std::fstream out(imageName.c_str(), std::ios::out | std::ios::binary);
                out.write((char*)sizeBytes, 8);
                createVolumeBitMap(blocks, out);

                // file count will always be 0 upon initialization
                uint64_t fileCount(0);
                uint8_t countBytes[8];
                buildFileCountBytes(fileCount, countBytes);

                // write out file count
                out.write((char*)countBytes, 8);

                // write out the file space bytes
                writeOutFileSpaceBytes(blocks, out);

                // set the very first file block (0) as being the first file block
                // of the root directory
                writeRootDirectory(blocks, out);
                out.flush();
                out.close();
            }

            // re-open in read/write
            std::fstream out(imageName.c_str(), std::ios::in | std::ios::out | std::ios::binary);

            // set first file block as being in use
            detail::updateVolumeBitmapWithOne(out, 0, blocks);

            out.flush();
            out.close();
        }
    };
}

#endif // BFS_MAKE_BFS_HPP__
