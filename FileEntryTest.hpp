#include "DetailBFS.hpp"
#include "DetailFileBlock.hpp"
#include "FileEntry.hpp"
#include "MakeBFS.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/stream.hpp>

#include <cassert>
#include <sstream>
#include <fstream>

int const HELLO_IT = 539;
int const BIG_SIZE = HELLO_IT * 13;

class FileEntryTest
{


  public:
	FileEntryTest() : m_uniquePath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
    {
        boost::filesystem::create_directories(m_uniquePath);
        testBasicAppend();
        testSeekAndReadSmallFile();
        testSeekAndReadBigFile();
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

    void testBasicAppend()
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
            bfs::FileEntry entry(testPath.c_str(), blocks, 1);
            std::string expected(createLargeStringToWrite());
            std::vector<char> vec;
            vec.resize(BIG_SIZE);
            entry.read(&vec.front(), BIG_SIZE);
            std::string recovered(vec.begin(), vec.begin() + BIG_SIZE);
            assert(recovered == expected);
        }

        std::cout<<"File entry write followed by read passed"<<std::endl;

        // test append
        std::string appendString("appended!");
        {
            bfs::FileEntry entry(testPath.c_str(), blocks, "test.txt", 1);
            entry.write(appendString.c_str(), appendString.length());
            entry.flush();
        }


        {
            bfs::FileEntry entry(testPath.c_str(), blocks, 1);
            std::string expected(createLargeStringToWrite());
            expected.append(appendString);
            std::vector<char> vec;
            vec.resize(BIG_SIZE + appendString.length());
            entry.read(&vec.front(), BIG_SIZE + appendString.length());
            std::string recovered(vec.begin(), vec.begin() + BIG_SIZE + appendString.length());
            assert(recovered == expected);
        }

        std::cout<<"File entry append passed"<<std::endl;
    }

    void testSeekAndReadSmallFile()
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
			std::string const testString("Hello and goodbye!");
            std::string testData(testString);
            std::vector<uint8_t> vec(testData.begin(), testData.end());
            entry.write((char*)&vec.front(), BIG_SIZE);
            entry.flush();
        }

        // test seek and read
        {
            bfs::FileEntry entry(testPath.c_str(), blocks, 1);
            std::string expected("goodbye!");
            std::vector<char> vec;
            vec.resize(expected.size());
            entry.seek(10);
            entry.read(&vec.front(), expected.size());
            std::string recovered(vec.begin(), vec.end());
            assert(recovered == expected);
        }

        std::cout<<"Test seek and read small file passed"<<std::endl;

    }

    void testSeekAndReadBigFile()
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
            bfs::FileEntry entry(testPath.c_str(), blocks, 1);
            std::string expected(createLargeStringToWrite());
            std::vector<char> vec;
            vec.resize(BIG_SIZE);
            entry.read(&vec.front(), BIG_SIZE);
            std::string recovered(vec.begin(), vec.end());

            assert(recovered == expected);
        }

        // test seek of big file
        std::string appendString("appended!");
        {
            bfs::FileEntry entry(testPath.c_str(), blocks, "test.txt", 1);
            entry.write(appendString.c_str(), appendString.length());
            entry.flush();
        }

        {
            bfs::FileEntry entry(testPath.c_str(), blocks, 1);
            std::vector<char> vec;
            vec.resize(appendString.length());
            entry.seek(BIG_SIZE);
            entry.read(&vec.front(), appendString.length());
            std::string recovered(vec.begin(), vec.end());
            assert(recovered == appendString);
        }

        std::cout<<"Test seek and read big file passed"<<std::endl;
    }

};
