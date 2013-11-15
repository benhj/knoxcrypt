#include "Detail.hpp"
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
        //correctNumberOfFilesIsReported();
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

    boost::filesystem::path m_uniquePath;

};
