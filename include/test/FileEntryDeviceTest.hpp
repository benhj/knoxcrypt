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

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
*/

#include "bfs/BFSImageStream.hpp"
#include "bfs/DetailBFS.hpp"
#include "bfs/FileEntry.hpp"
#include "bfs/FileEntryDevice.hpp"
#include "bfs/FileStreamPtr.hpp"
#include "bfs/OpenDisposition.hpp"
#include "test/TestHelpers.hpp"
#include "utility/MakeBFS.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/stream.hpp>

#include <cassert>
#include <sstream>

class FileEntryDeviceTest
{
  public:
    FileEntryDeviceTest() : m_uniquePath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
    {
        boost::filesystem::create_directories(m_uniquePath);
        testWriteReportsCorrectFileSize();
        testWriteFollowedByRead();
    }

    ~FileEntryDeviceTest()
    {
        boost::filesystem::remove_all(m_uniquePath);
    }

    void testWriteReportsCorrectFileSize()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);

        // test write get file size from same entry
        {
            bfs::CoreBFSIO io = createTestIO(testPath);
            bfs::FileEntry entry(io, "test.txt");
            bfs::FileStreamPtr stream(new bfs::FileStream(bfs::FileEntryDevice(entry)));
            std::string testData(createLargeStringToWrite());
            (*stream) << testData.c_str();
        }
        {
            bfs::CoreBFSIO io = createTestIO(testPath);
            bfs::FileEntry entry(io, "entry", uint64_t(1),
                                 bfs::OpenDisposition::buildReadOnlyDisposition());
            ASSERT_EQUAL(BIG_SIZE, entry.fileSize(), "FileStreamTest::testWriteReportsCorrectFileSize()");
        }
    }

    void testWriteFollowedByRead()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);

        // test write get file size from same entry
        std::string testData(createLargeStringToWrite());
        {
            bfs::CoreBFSIO io = createTestIO(testPath);
            bfs::FileEntry entry(io, "test.txt");
            bfs::FileEntryDevice device(entry);
            std::streampos bytesWrote = device.write(testData.c_str(), testData.length());
            ASSERT_EQUAL(BIG_SIZE, bytesWrote, "FileStreamTest::testWriteFollowedByRead() bytes wrote");
        }
        {
            bfs::CoreBFSIO io = createTestIO(testPath);
            bfs::FileEntry entry(io, "entry", uint64_t(1),
                                 bfs::OpenDisposition::buildReadOnlyDisposition());
            std::vector<uint8_t> buffer;
            buffer.resize(entry.fileSize());
            bfs::FileEntryDevice device(entry);
            std::streamsize readBytes = device.read((char*)&buffer.front(), entry.fileSize());
            ASSERT_EQUAL(BIG_SIZE, readBytes, "FileStreamTest::testWriteFollowedByRead() bytes read");
            std::string recovered(buffer.begin(), buffer.end());
            ASSERT_EQUAL(recovered, testData, "FileStreamTest::testWriteFollowedByRead() content check");
        }
    }

  private:

    boost::filesystem::path m_uniquePath;

};
