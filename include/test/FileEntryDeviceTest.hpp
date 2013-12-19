#include "bfs/BFSImageStream.hpp"
#include "bfs/DetailBFS.hpp"
#include "bfs/FileEntry.hpp"
#include "bfs/FileEntryDevice.hpp"
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
            bfs::FileEntry entry(testPath.string(), blocks, "test.txt");
            bfs::FileEntryDevice fileStream(entry);
            boost::iostreams::stream<bfs::FileEntryDevice> stream(fileStream);
            std::string testData(createLargeStringToWrite());
            stream << testData.c_str();
        }
        {
            bfs::FileEntry entry(testPath.string(), blocks, "entry", uint64_t(1),
                                 bfs::OpenDisposition::buildReadOnlyDisposition());
            ASSERT_EQUAL(BIG_SIZE, entry.fileSize(), "FileStreamTest::testWriteReportsCorrectFileSize()");
        }

    }

  private:

    boost::filesystem::path m_uniquePath;

};
