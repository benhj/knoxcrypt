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

#include "knoxcrypt/ContainerImageStream.hpp"
#include "knoxcrypt/CoreIO.hpp"
#include "knoxcrypt/detail/DetailKnoxCrypt.hpp"
#include "knoxcrypt/detail/DetailFileBlock.hpp"
#include "test/SimpleTest.hpp"
#include "test/TestHelpers.hpp"
#include "utility/MakeKnoxCrypt.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <cassert>

using namespace simpletest;

class MakeKnoxCryptTest
{
  public:
    MakeKnoxCryptTest() : m_uniquePath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
    {
        boost::filesystem::create_directories(m_uniquePath);
        correctBlockCountIsReported();
        firstBlockIsReportedAsBeingFree();
        blocksCanBeSetAndCleared();
        testThatRootFolderContainsZeroEntries();
    }

    ~MakeKnoxCryptTest()
    {
        boost::filesystem::remove_all(m_uniquePath);
    }

  private:
    void correctBlockCountIsReported()
    {
        uint64_t blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath);

        knoxcrypt::SharedCoreIO io(createTestIO(testPath));

        // test that enough bytes are written
        knoxcrypt::ContainerImageStream is(io, std::ios::in | std::ios::binary);
        ASSERT_EQUAL(blocks, knoxcrypt::detail::getBlockCount(is), "MakeKnoxCryptTest::correctBlockCountIsReported");
        is.close();
    }

    void firstBlockIsReportedAsBeingFree()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);

        knoxcrypt::SharedCoreIO io(createTestIO(testPath));

        knoxcrypt::ContainerImageStream is(io, std::ios::in | std::ios::binary);
        knoxcrypt::detail::OptionalBlock p = knoxcrypt::detail::getNextAvailableBlock(is);
        ASSERT_EQUAL(*p, 1, "MakeKnoxCryptTest::firstBlockIsReportedAsBeingFree");
        is.close();
    }

    void blocksCanBeSetAndCleared()
    {
        int blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath);

        knoxcrypt::SharedCoreIO io(createTestIO(testPath));

        knoxcrypt::ContainerImageStream is(io, std::ios::in | std::ios::out | std::ios::binary);
        knoxcrypt::detail::setBlockToInUse(1, blocks, is);
        knoxcrypt::detail::OptionalBlock p = knoxcrypt::detail::getNextAvailableBlock(is);
        ASSERT_EQUAL(2, *p, "MakeKnoxCryptTest::blocksCanBeSetAndCleared next available block");

        // check that rest of map can also be set correctly
        bool broken = false;
        for (int i = 2; i < blocks - 1; ++i) {
            knoxcrypt::detail::setBlockToInUse(i, blocks, is);
            p = knoxcrypt::detail::getNextAvailableBlock(is);
            if (*p != (uint64_t)(i + 1)) {
                broken = true;
                break;
            }
        }
        ASSERT_EQUAL(broken, false, "MakeKnoxCryptTest::blocksCanBeSetAndCleared A");

        // check that bit 25 (arbitrary) can be unset again
        knoxcrypt::detail::setBlockToInUse(25, blocks, is, false);
        p = knoxcrypt::detail::getNextAvailableBlock(is);
        ASSERT_EQUAL(*p, 25, "MakeKnoxCryptTest::blocksCanBeSetAndCleared B");

        // should still be 25 when blocks after 25 are also made available
        knoxcrypt::detail::setBlockToInUse(27, blocks, is, false);
        p = knoxcrypt::detail::getNextAvailableBlock(is);
        ASSERT_EQUAL(*p, 25, "MakeKnoxCryptTest::blocksCanBeSetAndCleared C");

        // should now be 27 since block 25 is made unavailable
        knoxcrypt::detail::setBlockToInUse(25, blocks, is);
        p = knoxcrypt::detail::getNextAvailableBlock(is);
        ASSERT_EQUAL(*p, 27, "MakeKnoxCryptTest::blocksCanBeSetAndCleared D");

        is.close();
    }

    void testThatRootFolderContainsZeroEntries()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        uint64_t blocks(2048);
        // open a stream and read the first byte which signifies number of entries
        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        uint64_t offset = knoxcrypt::detail::getOffsetOfFileBlock(io->blockSize, 0, blocks);
        knoxcrypt::ContainerImageStream is(io, std::ios::in | std::ios::out | std::ios::binary);
        is.seekg(offset + knoxcrypt::detail::FILE_BLOCK_META);
        uint8_t bytes[8];
        (void)is.read((char*)bytes, 8);
        uint64_t const count = knoxcrypt::detail::convertInt8ArrayToInt64(bytes);
        ASSERT_EQUAL(count, 0, "testThatRootFolderContainsZeroEntries");
    }

    boost::filesystem::path m_uniquePath;

};
