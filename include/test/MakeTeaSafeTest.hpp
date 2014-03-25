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

#include "teasafe/TeaSafeImageStream.hpp"
#include "teasafe/CoreTeaSafeIO.hpp"
#include "teasafe/detail/DetailTeaSafe.hpp"
#include "teasafe/detail/DetailFileBlock.hpp"
#include "test/TestHelpers.hpp"
#include "utility/MakeTeaSafe.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <cassert>

class MakeTeaSafeTest
{
  public:
    MakeTeaSafeTest() : m_uniquePath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
    {
        boost::filesystem::create_directories(m_uniquePath);
        correctBlockCountIsReported();
        firstBlockIsReportedAsBeingFree();
        blocksCanBeSetAndCleared();
        testThatRootFolderContainsZeroEntries();
    }

    ~MakeTeaSafeTest()
    {
        boost::filesystem::remove_all(m_uniquePath);
    }

  private:
    void correctBlockCountIsReported()
    {
        int blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);

        teasafe::SharedCoreIO io(createTestIO(testPath));

        // test that enough bytes are written
        teasafe::TeaSafeImageStream is(io, std::ios::in | std::ios::binary);
        ASSERT_EQUAL(blocks, teasafe::detail::getBlockCount(is), "correctBlockCountIsReported");
        is.close();
    }

    void firstBlockIsReportedAsBeingFree()
    {
        int blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);

        teasafe::SharedCoreIO io(createTestIO(testPath));

        teasafe::TeaSafeImageStream is(io, std::ios::in | std::ios::binary);
        teasafe::detail::OptionalBlock p = teasafe::detail::getNextAvailableBlock(is);
        ASSERT_EQUAL(*p, 1, "firstBlockIsReportedAsBeingFree");
        is.close();
    }

    void blocksCanBeSetAndCleared()
    {
        int blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);

        teasafe::SharedCoreIO io(createTestIO(testPath));

        teasafe::TeaSafeImageStream is(io, std::ios::in | std::ios::out | std::ios::binary);
        teasafe::detail::setBlockToInUse(1, blocks, is);
        teasafe::detail::OptionalBlock p = teasafe::detail::getNextAvailableBlock(is);
        assert(*p == 2);

        // check that rest of map can also be set correctly
        bool broken = false;
        for (int i = 2; i < blocks - 1; ++i) {
            teasafe::detail::setBlockToInUse(i, blocks, is);
            p = teasafe::detail::getNextAvailableBlock(is);
            if (*p != i + 1) {
                broken = true;
                break;
            }
        }
        ASSERT_EQUAL(broken, false, "blocksCanBeSetAndCleared A");

        // check that bit 25 (arbitrary) can be unset again
        teasafe::detail::setBlockToInUse(25, blocks, is, false);
        p = teasafe::detail::getNextAvailableBlock(is);
        ASSERT_EQUAL(*p, 25, "blocksCanBeSetAndCleared B");

        // should still be 25 when blocks after 25 are also made available
        teasafe::detail::setBlockToInUse(27, blocks, is, false);
        p = teasafe::detail::getNextAvailableBlock(is);
        ASSERT_EQUAL(*p, 25, "blocksCanBeSetAndCleared C");

        // should now be 27 since block 25 is made unavailable
        teasafe::detail::setBlockToInUse(25, blocks, is);
        p = teasafe::detail::getNextAvailableBlock(is);
        ASSERT_EQUAL(*p, 27, "blocksCanBeSetAndCleared D");

        is.close();
    }

    void testThatRootFolderContainsZeroEntries()
    {
        int blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);

        uint64_t offset = teasafe::detail::getOffsetOfFileBlock(0, blocks);
        // open a stream and read the first byte which signifies number of entries
        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafeImageStream is(io, std::ios::in | std::ios::out | std::ios::binary);
        is.seekg(offset + teasafe::detail::FILE_BLOCK_META);
        uint8_t bytes[8];
        (void)is.read((char*)bytes, 8);
        uint64_t const count = teasafe::detail::convertInt8ArrayToInt64(bytes);
        ASSERT_EQUAL(count, 0, "testThatRootFolderContainsZeroEntries");
    }

    boost::filesystem::path m_uniquePath;

};
