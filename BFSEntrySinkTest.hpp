#include "Detail.hpp"
#include "BFSEntrySink.hpp"
#include "MakeBFS.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/stream.hpp>

#include <cassert>
#include <sstream>
#include <fstream>

class BFSEntrySinkTest
{
public:
    BFSEntrySinkTest() : m_uniquePath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
    {
        boost::filesystem::create_directories(m_uniquePath);
        oneDataEntry();
    }

    ~BFSEntrySinkTest()
    {
        boost::filesystem::remove_all(m_uniquePath);
    }

private:

    void oneDataEntry()
    {
        std::string testImage(boost::filesystem::unique_path().string());
        boost::filesystem::path testPath = m_uniquePath / testImage;
        uint64_t bytes(1048576); // 1MB
        bfs::MakeBFS bfs(testPath.string(), bytes);

        // create a test entry sink
        bfs::BFSEntrySink entrySink(testPath.c_str(), "test.txt", uint64_t(13), uint64_t(0));
        std::string testData("Hello, world!");
        std::stringstream ss;
        ss << testData.c_str();
        boost::iostreams::stream<bfs::BFSEntrySink> bfsEntryStream(entrySink);

        // copy from the test stream to the entry stream
        boost::iostreams::copy(ss,  bfsEntryStream);

        // tests
        std::fstream input(testPath.string().c_str(), std::ios::in | std::ios::binary);
        assert(bfs::detail::getImageSize(input) == bytes);
        std::cout<<bfs::detail::getSizeOfFileN(input, 0)<<std::endl;
        assert(bfs::detail::getSizeOfFileN(input, 1) == uint64_t(13));
        input.close();
    }

    boost::filesystem::path m_uniquePath;

};
