#ifndef BFS_MAKE_BFS_HPP__
#define BFS_MAKE_BFS_HPP__

#include "Detail.hpp"

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

        //
        // note this is just an implementation detail, we don't have
        // to write in 4K blocks. This was just an arbitrary decision
        // based on what the size of out buffer should be
        //
        void writeOut4KBlocks(uint64_t const byteCount, std::fstream &out)
        {
            // write out metaBytes of metadata
            std::vector<int8_t> ints;
            ints.assign(4096, 0);
            uint64_t leftOver = byteCount % uint64_t(4096);

            if(byteCount > 4096) {
                uint64_t iterations = (byteCount - leftOver) / uint64_t(4096);
                for (int i = 0; i < iterations; ++i) {
                    out.write((char*)&ints.front(), 4096);
                }
            }
            out.write((char*) (&ints.front()), leftOver);
        }

        void writeOutMetaBytes(uint64_t const metaBytes, std::fstream &out)
        {
            writeOut4KBlocks(metaBytes, out);
        }

        void writeOutFileSpaceBytes(uint64_t const fileSpaceBytes, std::fstream &out)
        {
            writeOut4KBlocks(fileSpaceBytes, out);
        }

        void zeroOutBits(std::vector<uint8_t> &bitMapData)
        {
            uint8_t byte;
            byte &= 1 << 1;
            byte &= 1 << 2;
            byte &= 1 << 3;
            byte &= 1 << 4;
            byte &= 1 << 5;
            byte &= 1 << 6;
            byte &= 1 << 7;
            byte &= 1 << 8;
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
            for(uint64_t b = 0; b < bytesRequired; ++b) {
                zeroOutBits(bitMapData);
            }
            (void)out.write((char*)&bitMapData.front(), bytesRequired);
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
            std::fstream out(imageName.c_str(), std::ios::out | std::ios::binary);
            out.write((char*)sizeBytes, 8);
            createVolumeBitMap(blocks, out);

            // file count will always be 0 upon initialization
            uint64_t fileCount(0);
            uint8_t countBytes[8];
            buildFileCountBytes(fileCount, countBytes);

            // write out file count
            out.write((char*)countBytes, 8);

            //
            // we will have 0.1% bytes file count; each file metadata item
            // will be 74 bytes in length
            //
            // First 50 bytes: name
            // Next 8 bytes: 1st block position
            // Next 8 bytes: file size
            // Fourth 8 bytes: other metadata (tbd)
            //
            uint64_t statAlloc(static_cast<uint64_t>((blocks * 512) * 0.001) * 74);

            // write out metaBytes of metadata
            writeOutMetaBytes(statAlloc, out);

            // write out the file space bytes
            writeOutFileSpaceBytes((blocks * 512), out);

            out.flush();
            out.close();
        }
};
}

#endif // BFS_MAKE_BFS_HPP__
