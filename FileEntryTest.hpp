#include "DetailBFS.hpp"
#include "DetailFileBlock.hpp"
#include "DetailMeta.hpp"
#include "FileEntry.hpp"
#include "MakeBFS.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/stream.hpp>

#include <cassert>
#include <sstream>
#include <fstream>

//int const HELLO_IT = 70000;
//int const BIG_SIZE = HELLO_IT * 13;

class FileEntryTest
{


  public:
	FileEntryTest() : m_uniquePath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
    {
        boost::filesystem::create_directories(m_uniquePath);
        test();
    }


    std::string createLargeStringToWrite() const
    {
        std::string theString("");
        for(int i = 0; i < HELLO_IT; ++i) {
            theString.append("Hello, World!");
        }
        return theString;
    }

    ~FileEntryTest()
    {
        boost::filesystem::remove_all(m_uniquePath);
    }

  private:

    boost::filesystem::path m_uniquePath;

    void test()
    {
        std::string testImage(boost::filesystem::unique_path().string());
        boost::filesystem::path testPath = m_uniquePath / testImage;
        uint64_t blocks(2048); // 1MB

        {
			  bfs::MakeBFS bfs(testPath.string(), blocks);
        }

        // test write
        {
			bfs::FileEntry entry(testPath.c_str(), blocks, "test.txt");
            std::string testData(createLargeStringToWrite());
            std::vector<uint8_t> vec(testData.begin(), testData.end());
            entry.write((char*)&vec.front(), BIG_SIZE);
            entry.flush();
        }

        // test read

        {
            bfs::FileEntry entry(testPath.c_str(), blocks, 0);
            std::string expected(createLargeStringToWrite());
            std::vector<char> vec;
            vec.resize(BIG_SIZE);
            entry.read(&vec.front(), BIG_SIZE);
            std::string recovered(vec.begin(), vec.begin() + BIG_SIZE);
            assert(recovered == expected);
        }
    }
};
