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
        imageIsExpectedSize();
        correctFSSizeIsReported();
        correctNumberOfFilesIsReported();
    }

    ~MakeBFSTest()
    {
        boost::filesystem::remove_all(m_uniquePath);
    }

private:
    void imageIsExpectedSize()
    {
        std::string testImage(boost::filesystem::unique_path().string());
        boost::filesystem::path testPath = m_uniquePath / testImage;
        uint64_t bytes(1048576); // 1MB
        bfs::MakeBFS bfs(testPath.string(), bytes);

        // test that enough bytes are written
        std::fstream is(testPath.string().c_str(), std::ios::in | std::ios::binary);
        assert(bytes == bfs::detail::getImageSize(is));
        is.close();
    }


    void correctFSSizeIsReported()
    {
        std::string testImage(boost::filesystem::unique_path().string());
        boost::filesystem::path testPath = m_uniquePath / testImage;
        uint64_t bytes(1048576); // 1MB
        bfs::MakeBFS bfs(testPath.string(), bytes);
        uint64_t statAlloc(static_cast<uint64_t>(bytes * 0.001) * 32);
        uint64_t fsSize(bytes - statAlloc - bfs::detail::METABLOCKS_BEGIN);
        std::fstream is(testPath.string().c_str(), std::ios::in | std::ios::binary);

        // get the size from this point to end which will be the file block size
        uint64_t const actualSize = bfs::detail::getFSSize(is);

        is.close();

        // test assert that reported size is the expected size
        assert(actualSize == fsSize);
    }

    void correctNumberOfFilesIsReported()
    {
        std::string testImage(boost::filesystem::unique_path().string());
        boost::filesystem::path testPath = m_uniquePath / testImage;
        uint64_t bytes(1048576); // 1MB
        bfs::MakeBFS bfs(testPath.string(), bytes);

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
