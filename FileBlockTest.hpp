#include "BFSImageStream.hpp"
#include "DetailBFS.hpp"
#include "DetailFileBlock.hpp"
#include "FileBlock.hpp"
#include "MakeBFS.hpp"
#include "TestHelpers.hpp"

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

        bfs::FileBlock block(testPath.string(), blocks, uint64_t(0), uint64_t(0));
        std::string testData("Hello, world!Hello, world!");
        std::vector<uint8_t> vec(testData.begin(), testData.end());
        block.write((char*)&vec.front(), testData.length());

        // test that actual written correct
        assert(block.getDataBytesWritten() == 26);
        bfs::BFSImageStream stream(testPath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
        uint64_t size = bfs::detail::getNumberOfDataBytesWrittenToFileBlockN(stream, 0, blocks);
        ASSERT_EQUAL(size, 26, "blockWriteAndReadTest: correctly returned block size");

        // test that reported next index correct
        assert(block.getNextIndex() == 0);
        uint64_t next = bfs::detail::getIndexOfNextFileBlockFromFileBlockN(stream, 0, blocks);
        stream.close();
        ASSERT_EQUAL(next, 0, "blockWriteAndReadTest: correct block index");

        // test that data can be read correctly
        std::vector<uint8_t> dat;
        dat.resize(size);
        assert(block.read((char*)&dat.front(), size) == size);
        std::string str(dat.begin(), dat.end());
        ASSERT_EQUAL(str, testData, "blockWriteAndReadTest: data read");
    }

};
