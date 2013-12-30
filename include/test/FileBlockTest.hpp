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
#include "bfs/DetailBFS.hpp"
#include "bfs/DetailFileBlock.hpp"
#include "bfs/FileBlock.hpp"
#include "bfs/FileBlockException.hpp"
#include "bfs/MakeBFS.hpp"
#include "bfs/OpenDisposition.hpp"
#include "test/TestHelpers.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/stream.hpp>

#include <cassert>
#include <sstream>

class FileBlockTest
{


  public:
    FileBlockTest() : m_uniquePath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
    {
        boost::filesystem::create_directories(m_uniquePath);
        blockWriteAndReadTest();
        testWritingToNonWritableThrows();
        testReadingFromNonReadableThrows();
    }

    ~FileBlockTest()
    {
        boost::filesystem::remove_all(m_uniquePath);
    }

  private:

    boost::filesystem::path m_uniquePath;

    void blockWriteAndReadTest()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);

        bfs::CoreBFSIO io;
        io.path = testPath.string();
        io.blocks = blocks;
        io.password = "abcd1234";

        bfs::FileBlock block(io, uint64_t(0), uint64_t(0),
                             bfs::OpenDisposition::buildAppendDisposition());
        std::string testData("Hello, world!Hello, world!");
        std::vector<uint8_t> vec(testData.begin(), testData.end());
        block.write((char*)&vec.front(), testData.length());

        // test that actual written correct
        assert(block.getDataBytesWritten() == 26);
        bfs::BFSImageStream stream(io, std::ios::in | std::ios::out | std::ios::binary);
        uint64_t size = bfs::detail::getNumberOfDataBytesWrittenToFileBlockN(stream, 0, blocks);
        ASSERT_EQUAL(size, 26, "FileBlockTest::blockWriteAndReadTest(): correctly returned block size");

        // test that reported next index correct
        assert(block.getNextIndex() == 0);
        uint64_t next = bfs::detail::getIndexOfNextFileBlockFromFileBlockN(stream, 0, blocks);
        stream.close();
        ASSERT_EQUAL(next, 0, "FileBlockTest::blockWriteAndReadTest(): correct block index");

        // test that data can be read correctly
        std::vector<uint8_t> dat;
        dat.resize(size);
        block.seek(0);
        std::streamsize bytesRead = block.read((char*)&dat.front(), size);
        ASSERT_EQUAL(bytesRead, size, "FileBlockTest::blockWriteAndReadTest(): data read bytes read check");
        std::string str(dat.begin(), dat.end());
        ASSERT_EQUAL(str, testData, "FileBlockTest::blockWriteAndReadTest(): data read content check");
    }

    void testWritingToNonWritableThrows()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);

        {
            bfs::CoreBFSIO io;
            io.path = testPath.string();
            io.blocks = blocks;
            io.password = "abcd1234";
            bfs::FileBlock block(io, uint64_t(0), uint64_t(0),
                                 bfs::OpenDisposition::buildReadOnlyDisposition());
            std::string testData("Hello, world!Hello, world!");
            std::vector<uint8_t> vec(testData.begin(), testData.end());
            bool pass = false;
            // assert correct exception was thrown
            try {
                block.write((char*)&vec.front(), testData.length());
            } catch (bfs::FileBlockException const &e) {
                ASSERT_EQUAL(e, bfs::FileBlockException(bfs::FileBlockError::NotWritable), "FileBlockTest::testWritingToNonWritableThrows() A");
                pass = true;
            }
            // assert that any exception was thrown
            ASSERT_EQUAL(pass, true, "FileBlockTest::testWritingToNonWritableThrows() B");
        }
    }

    void testReadingFromNonReadableThrows()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);

        {
            bfs::CoreBFSIO io;
            io.path = testPath.string();
            io.blocks = blocks;
            io.password = "abcd1234";
            bfs::FileBlock block(io, uint64_t(0), uint64_t(0),
                                 bfs::OpenDisposition::buildWriteOnlyDisposition());
            std::string testData("Hello, world!Hello, world!");
            std::vector<uint8_t> vec(testData.begin(), testData.end());
            block.write((char*)&vec.front(), testData.length());
        }

        {
            bfs::CoreBFSIO io;
            io.path = testPath.string();
            io.blocks = blocks;
            io.password = "abcd1234";
            bfs::FileBlock block(io, uint64_t(0),
                                 bfs::OpenDisposition::buildWriteOnlyDisposition());
            std::vector<uint8_t> vec(block.getInitialDataBytesWritten());
            // assert correct exception was thrown
            bool pass = false;
            try {
                block.read((char*)&vec.front(), block.getInitialDataBytesWritten());
            } catch (bfs::FileBlockException const &e) {
                ASSERT_EQUAL(e, bfs::FileBlockException(bfs::FileBlockError::NotReadable), "FileBlockTest::testReadingFromNonReadableThrows() A");
                pass = true;
            }
            // assert that any exception was thrown
            ASSERT_EQUAL(pass, true, "FileBlockTest::testReadingFromNonReadableThrows() B");
        }
    }

};
