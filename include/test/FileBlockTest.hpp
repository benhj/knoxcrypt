/*
  Copyright (c) <2013-2015>, <BenHJ>
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

#include "knoxcrypt/ContainerImageStream.hpp"
#include "knoxcrypt/FileBlock.hpp"
#include "knoxcrypt/FileBlockException.hpp"
#include "knoxcrypt/OpenDisposition.hpp"
#include "knoxcrypt/detail/Detailknoxcrypt.hpp"
#include "knoxcrypt/detail/DetailFileBlock.hpp"
#include "test/SimpleTest.hpp"
#include "test/TestHelpers.hpp"
#include "utility/Makeknoxcrypt.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/stream.hpp>

#include <cassert>
#include <sstream>

using namespace simpletest;

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
        boost::filesystem::path testPath = buildImage(m_uniquePath);

        knoxcrypt::SharedCoreIO io(createTestIO(testPath));

        knoxcrypt::FileBlock block(io, uint64_t(0), uint64_t(0),
                                 knoxcrypt::OpenDisposition::buildAppendDisposition());
        std::string testData("Hello, world!Hello, world!");
        std::vector<uint8_t> vec(testData.begin(), testData.end());
        block.write((char*)&vec.front(), testData.length());

        // test that actual written correct
        assert(block.getDataBytesWritten() == 26);
        knoxcrypt::ContainerImageStream stream(io, std::ios::in | std::ios::out | std::ios::binary);
        uint64_t size = knoxcrypt::detail::getNumberOfDataBytesWrittenToFileBlockN(stream, 0, blocks);
        ASSERT_EQUAL(size, 26, "FileBlockTest::blockWriteAndReadTest(): correctly returned block size");

        // test that reported next index correct
        assert(block.getNextIndex() == 0);
        uint64_t next = knoxcrypt::detail::getIndexOfNextFileBlockFromFileBlockN(stream, 0, blocks);
        stream.close();
        ASSERT_EQUAL(next, 0, "FileBlockTest::blockWriteAndReadTest(): correct block index");

        // test that data can be read correctly
        std::vector<uint8_t> dat;
        dat.resize(size);
        block.seek(0);
        std::streamsize bytesRead = block.read((char*)&dat.front(), size);
        ASSERT_EQUAL(static_cast<size_t>(bytesRead), size, "FileBlockTest::blockWriteAndReadTest(): data read bytes read check");
        std::string str(dat.begin(), dat.end());
        ASSERT_EQUAL(str, testData, "FileBlockTest::blockWriteAndReadTest(): data read content check");
    }

    void testWritingToNonWritableThrows()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);

        {
            knoxcrypt::SharedCoreIO io(createTestIO(testPath));
            knoxcrypt::FileBlock block(io, uint64_t(0), uint64_t(0),
                                     knoxcrypt::OpenDisposition::buildReadOnlyDisposition());
            std::string testData("Hello, world!Hello, world!");
            std::vector<uint8_t> vec(testData.begin(), testData.end());
            bool pass = false;
            // assert correct exception was thrown
            try {
                block.write((char*)&vec.front(), testData.length());
            } catch (knoxcrypt::FileBlockException const &e) {
                ASSERT_EQUAL(e, knoxcrypt::FileBlockException(knoxcrypt::FileBlockError::NotWritable), "FileBlockTest::testWritingToNonWritableThrows() A");
                pass = true;
            }
            // assert that any exception was thrown
            ASSERT_EQUAL(pass, true, "FileBlockTest::testWritingToNonWritableThrows() B");
        }
    }

    void testReadingFromNonReadableThrows()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);

        {
            knoxcrypt::SharedCoreIO io(createTestIO(testPath));
            knoxcrypt::FileBlock block(io, uint64_t(0), uint64_t(0),
                                     knoxcrypt::OpenDisposition::buildWriteOnlyDisposition());
            std::string testData("Hello, world!Hello, world!");
            std::vector<uint8_t> vec(testData.begin(), testData.end());
            block.write((char*)&vec.front(), testData.length());
        }

        {
            knoxcrypt::SharedCoreIO io(createTestIO(testPath));
            knoxcrypt::FileBlock block(io, uint64_t(0),
                                     knoxcrypt::OpenDisposition::buildWriteOnlyDisposition());
            std::vector<uint8_t> vec(block.getInitialDataBytesWritten());
            // assert correct exception was thrown
            bool pass = false;
            try {
                block.read((char*)&vec.front(), block.getInitialDataBytesWritten());
            } catch (knoxcrypt::FileBlockException const &e) {
                ASSERT_EQUAL(e, knoxcrypt::FileBlockException(knoxcrypt::FileBlockError::NotReadable), "FileBlockTest::testReadingFromNonReadableThrows() A");
                pass = true;
            }
            // assert that any exception was thrown
            ASSERT_EQUAL(pass, true, "FileBlockTest::testReadingFromNonReadableThrows() B");
        }
    }

};
