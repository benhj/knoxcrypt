#ifndef BFS_MAKE_BFS_HPP__
#define BFS_MAKE_BFS_HPP__

#include "Detail.hpp"

#include <string>
#include <fstream>
#include <vector>
#include <iostream>

namespace bfs
{

    uint64_t const MAX_FILENAME_LENGTH = 50;

    class MakeBFS
    {
    public:
        explicit MakeBFS(std::string const &imageName, uint64_t const bytes)
        {
            buildImage(imageName, bytes);
        }

    private:
        MakeBFS(); // not required

        void buildSizeBytes(uint64_t const fsSize, uint8_t sizeBytes[8])
        {
            detail::convertInt64ToInt8Array(fsSize, sizeBytes);
        }

        void buildFileCountBytes(uint64_t const fileCount, uint8_t sizeBytes[8])
        {
            detail::convertInt64ToInt8Array(fileCount, sizeBytes);
        }

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

        /**
         * @brief build the file system image
         *
         * The first 8 bytes will represent the size of the fs
         * The second 8 bytes will represent the number of files
         * The next numberOfFiles * (MAX_FILENAME_LENGTH + 24) will represent metadata
         * Note that the number of space for this metadata will be 1% of the
         * requested image size
         * The remaining bytes will be reserved for actual file data
         *
         * @param imageName the name of the image
         * @param bytes the size of the fs to be built
         */
        void buildImage(std::string const &imageName, uint64_t const bytes)
        {
            //
            // we will have 0.1% bytes file count; each file metadata item
            // will be 32 bytes in length
            //
            // First 8 bytes: file size
            // Second 8 bytes: position
            // Third 8 bytes: index of parent folder
            // Fourth 8 bytes: other metadata (tbd)
            //
            uint64_t statAlloc(static_cast<uint64_t>(bytes * 0.001) * 32);

            //
            // set the size of the file system computed as bytes - statAlloc - 24
            // note 24 (8 for fs size + 8 for file count + 8 total files size)
            //
            uint64_t fsSize(bytes - statAlloc - detail::METABLOCKS_BEGIN);
            uint8_t sizeBytes[8];
            buildSizeBytes(fsSize, sizeBytes);

            // file count will always be 0 upon initialization
            uint64_t fileCount(0);
            uint8_t countBytes[8];
            buildFileCountBytes(fileCount, countBytes);

            // write out size, file count bytes (initialized to zero)
            // and total file size bytes (also initialized to zero)
            std::fstream out(imageName.c_str(), std::ios::out | std::ios::binary);
            out.write((char*)sizeBytes, 8);
            out.write((char*)countBytes, 8);
            out.write((char*)countBytes, 8);

            // write out metaBytes of metadata
            writeOutMetaBytes(statAlloc, out);

            // write out the file space bytes
            writeOutFileSpaceBytes(fsSize, out);

            out.flush();
            out.close();
        }
};
}

#endif // BFS_MAKE_BFS_HPP__
