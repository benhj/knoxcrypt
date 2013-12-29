#include "bfs/BFSImageStream.hpp"
#include "bfs/DetailBFS.hpp"
#include "bfs/FileEntry.hpp"
#include "bfs/FileEntryDevice.hpp"
#include "bfs/FileStreamPtr.hpp"
#include "bfs/MakeBFS.hpp"
#include "bfs/OpenDisposition.hpp"
#include "test/TestHelpers.hpp"

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
            bfs::CoreBFSIO io;
            io.path = testPath.string();
            io.blocks = blocks;
            io.password = "abcd1234";
            bfs::FileEntry entry(io, "test.txt");
            bfs::FileStreamPtr stream(new bfs::FileStream(bfs::FileEntryDevice(entry)));
            std::string testData(createLargeStringToWrite());
            (*stream) << testData.c_str();
        }
        {
            bfs::CoreBFSIO io;
            io.path = testPath.string();
            io.blocks = blocks;
            io.password = "abcd1234";
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
            bfs::CoreBFSIO io;
            io.path = testPath.string();
            io.blocks = blocks;
            io.password = "abcd1234";
            bfs::FileEntry entry(io, "test.txt");
            bfs::FileEntryDevice device(entry);
            std::streampos bytesWrote = device.write(testData.c_str(), testData.length());
            ASSERT_EQUAL(BIG_SIZE, bytesWrote, "FileStreamTest::testWriteFollowedByRead() bytes wrote");
        }
        {
            bfs::CoreBFSIO io;
            io.path = testPath.string();
            io.blocks = blocks;
            io.password = "abcd1234";
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
