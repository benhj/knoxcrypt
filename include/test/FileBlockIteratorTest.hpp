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

#include "teasafe/TeaSafeImageStream.hpp"
#include "teasafe/CoreTeaSafeIO.hpp"
#include "teasafe/FileBlockIterator.hpp"
#include "test/TestHelpers.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

class FileBlockIteratorTest
{
  public:
    FileBlockIteratorTest() : m_uniquePath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
    {
        boost::filesystem::create_directories(m_uniquePath);
        test();
    }

    ~FileBlockIteratorTest()
    {
        boost::filesystem::remove_all(m_uniquePath);
    }

  private:

    boost::filesystem::path m_uniquePath;

    void test()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);

        // build some blocks to iterate over
        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafeFile entry(io, "test.txt");
        std::string testData(createLargeStringToWrite());
        std::vector<uint8_t> vec(testData.begin(), testData.end());
        entry.write((char*)&vec.front(), BIG_SIZE);
        entry.flush();

        uint64_t count(0);
        teasafe::FileBlockIterator begin(io,
                                         entry.getStartVolumeBlockIndex(),
                                         teasafe::OpenDisposition::buildReadOnlyDisposition());
        teasafe::FileBlockIterator end;

        ASSERT_EQUAL(false, (begin.equal(end)), "FileBlockIteratorTest::test equality BEFORE false");

        for (; begin != end; ++begin) {
            ++count;
        }

        ASSERT_EQUAL(4, count, "FileBlockIteratorTest::test number of blocks iterated over");
        ASSERT_EQUAL(true, (begin.equal(end)), "FileBlockIteratorTest::test equality AFTER true");

    }


};
