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

#include "bfs/BFSImageStream.hpp"
#include "bfs/CoreBFSIO.hpp"
#include "bfs/DetailBFS.hpp"
#include "bfs/DetailFileBlock.hpp"
#include "bfs/MakeBFS.hpp"
#include "test/TestHelpers.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <cassert>

class MakeBFSTest
{
  public:
    MakeBFSTest() : m_uniquePath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
    {
        boost::filesystem::create_directories(m_uniquePath);
        correctBlockCountIsReported();
        correctNumberOfFilesIsReported();
        firstBlockIsReportedAsBeingFree();
        blocksCanBeSetAndCleared();
        testThatRootFolderContainsZeroEntries();
    }

    ~MakeBFSTest()
    {
        boost::filesystem::remove_all(m_uniquePath);
    }

  private:
    void correctBlockCountIsReported()
    {
        int blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);

        bfs::CoreBFSIO io;
        io.path = testPath.string();
        io.blocks = blocks;
        io.password = "abcd1234";

        // test that enough bytes are written
        bfs::BFSImageStream is(io, std::ios::in | std::ios::binary);
        ASSERT_EQUAL(blocks, bfs::detail::getBlockCount(is), "correctBlockCountIsReported");
        is.close();
    }

    void correctNumberOfFilesIsReported()
    {
        int blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);

        uint64_t fileCount(0);
        uint8_t fileCountBytes[8];

        bfs::CoreBFSIO io;
        io.path = testPath.string();
        io.blocks = blocks;
        io.password = "abcd1234";

        bfs::BFSImageStream is(io, std::ios::in | std::ios::binary);

        // convert the byte array back to uint64 representation
        uint64_t reported = bfs::detail::getFileCount(is);
        is.close();
        ASSERT_EQUAL(reported, fileCount, "correctNumberOfFilesIsReported");
    }

    void firstBlockIsReportedAsBeingFree()
    {
        int blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);

        bfs::CoreBFSIO io;
        io.path = testPath.string();
        io.blocks = blocks;
        io.password = "abcd1234";

        bfs::BFSImageStream is(io, std::ios::in | std::ios::binary);
        bfs::detail::OptionalBlock p = bfs::detail::getNextAvailableBlock(is);
        assert(*p == 1);
        ASSERT_EQUAL(*p, 1, "firstBlockIsReportedAsBeingFree");
        is.close();
    }

    void blocksCanBeSetAndCleared()
    {
        int blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);

        bfs::CoreBFSIO io;
        io.path = testPath.string();
        io.blocks = blocks;
        io.password = "abcd1234";

        bfs::BFSImageStream is(io, std::ios::in | std::ios::out | std::ios::binary);
        bfs::detail::setBlockToInUse(1, blocks, is);
        bfs::detail::OptionalBlock p = bfs::detail::getNextAvailableBlock(is);
        assert(*p == 2);

        // check that rest of map can also be set correctly
        bool broken = false;
        for (int i = 2; i < blocks - 1; ++i) {
            bfs::detail::setBlockToInUse(i, blocks, is);
            p = bfs::detail::getNextAvailableBlock(is);
            if(*p != i + 1) {
                broken = true;
                break;
            }
        }
        ASSERT_EQUAL(broken, false, "blocksCanBeSetAndCleared A");

        // check that bit 25 (arbitrary) can be unset again
        bfs::detail::setBlockToInUse(25, blocks, is, false);
        p = bfs::detail::getNextAvailableBlock(is);
        ASSERT_EQUAL(*p, 25, "blocksCanBeSetAndCleared B");

        // should still be 25 when blocks after 25 are also made available
        bfs::detail::setBlockToInUse(27, blocks, is, false);
        p = bfs::detail::getNextAvailableBlock(is);
        ASSERT_EQUAL(*p, 25, "blocksCanBeSetAndCleared C");

        // should now be 27 since block 25 is made unavailable
        bfs::detail::setBlockToInUse(25, blocks, is);
        p = bfs::detail::getNextAvailableBlock(is);
        ASSERT_EQUAL(*p, 27, "blocksCanBeSetAndCleared D");

        is.close();
    }

    void testThatRootFolderContainsZeroEntries()
    {
        int blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);

        uint64_t offset = bfs::detail::getOffsetOfFileBlock(0, blocks);
        // open a stream and read the first byte which signifies number of entries
        bfs::CoreBFSIO io;
        io.path = testPath.string();
        io.blocks = blocks;
        io.password = "abcd1234";
        bfs::BFSImageStream is(io, std::ios::in | std::ios::out | std::ios::binary);
        is.seekg(offset + bfs::detail::FILE_BLOCK_META);
        uint8_t bytes[8];
        (void)is.read((char*)bytes, 8);
        uint64_t const count = bfs::detail::convertInt8ArrayToInt64(bytes);
        ASSERT_EQUAL(count, 0, "testThatRootFolderContainsZeroEntries");
    }

    boost::filesystem::path m_uniquePath;

};
