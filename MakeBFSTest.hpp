#include "DetailBFS.hpp"
#include "DetailFileBlock.hpp"
#include "MakeBFS.hpp"

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
    }

    ~MakeBFSTest()
    {
        boost::filesystem::remove_all(m_uniquePath);
    }

  private:
    void correctBlockCountIsReported()
    {
        std::string testImage(boost::filesystem::unique_path().string());
        boost::filesystem::path testPath = m_uniquePath / testImage;
        uint64_t blocks(2048); // 1MB = 512 * 2048
        bfs::MakeBFS bfs(testPath.string(), blocks);

        // test that enough bytes are written
        std::fstream is(testPath.string().c_str(), std::ios::in | std::ios::binary);
        assert(blocks == bfs::detail::getBlockCount(is));
        is.close();
    }

    void correctNumberOfFilesIsReported()
    {
        std::string testImage(boost::filesystem::unique_path().string());
        boost::filesystem::path testPath = m_uniquePath / testImage;
        uint64_t blocks(2048); // 1MB
        bfs::MakeBFS bfs(testPath.string(), blocks);

        uint64_t fileCount(0);
        uint8_t fileCountBytes[8];

        std::fstream is(testPath.string().c_str(), std::ios::in | std::ios::binary);

        // convert the byte array back to uint64 representation
        uint64_t reported = bfs::detail::getFileCount(is);
        is.close();
        assert(reported == fileCount);
    }

    void firstBlockIsReportedAsBeingFree()
    {
        std::string testImage(boost::filesystem::unique_path().string());
        boost::filesystem::path testPath = m_uniquePath / testImage;
        uint64_t blocks(2048); // 1MB
        bfs::MakeBFS bfs(testPath.string(), blocks);

        std::fstream is(testPath.string().c_str(), std::ios::in | std::ios::binary);
        bfs::detail::OptionalBlock p = bfs::detail::getNextAvailableBlock(is);
        assert(*p == 1);
        is.close();
    }

    void blocksCanBeSetAndCleared()
    {
        std::string testImage(boost::filesystem::unique_path().string());
        boost::filesystem::path testPath = m_uniquePath / testImage;
        uint64_t blocks(2048); // 1MB
        bfs::MakeBFS bfs(testPath.string(), blocks);

        std::fstream is(testPath.string().c_str(), std::ios::in | std::ios::out | std::ios::binary);
        bfs::detail::setBlockToInUse(1, blocks, is);
        bfs::detail::OptionalBlock p = bfs::detail::getNextAvailableBlock(is);
        assert(*p == 2);

        // check that rest of map can also be set correctly
        for (int i = 2; i < blocks - 1; ++i) {
            bfs::detail::setBlockToInUse(i, blocks, is);
            p = bfs::detail::getNextAvailableBlock(is);
            assert(*p == i + 1);
        }

        // check that bit 25 (arbitrary) can be unset again
        bfs::detail::setBlockToInUse(25, blocks, is, false);
        p = bfs::detail::getNextAvailableBlock(is);
        assert(*p == 25);

        // should still be 25 when blocks after 25 are also made available
        bfs::detail::setBlockToInUse(27, blocks, is, false);
        p = bfs::detail::getNextAvailableBlock(is);
        assert(*p == 25);

        // should now be 27 since block 25 is made unavailable
        bfs::detail::setBlockToInUse(25, blocks, is);
        p = bfs::detail::getNextAvailableBlock(is);
        assert(*p == 27);

        is.close();
    }

    boost::filesystem::path m_uniquePath;

};
