#include "Detail.hpp"
#include "BFSEntryWriter.hpp"
#include "MakeBFS.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/stream.hpp>

#include <cassert>
#include <sstream>
#include <fstream>

int const HELLO_IT = 200;
int const BIG_SIZE = HELLO_IT * 13;

class BFSEntryWriterTest
{


  public:
    BFSEntryWriterTest() : m_uniquePath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
    {
        boost::filesystem::create_directories(m_uniquePath);
        oneDataEntry();
    }

    ~BFSEntryWriterTest()
    {
        boost::filesystem::remove_all(m_uniquePath);
    }

  private:

    std::string createLargeStringToWrite() const
    {
        std::string theString("");
        for(int i = 0; i < HELLO_IT; ++i) {
            theString.append("Hello, World!");
        }
        return theString;
    }

    void oneDataEntry()
    {

          std::string testImage(boost::filesystem::unique_path().string());
          boost::filesystem::path testPath = m_uniquePath / testImage;
          uint64_t blocks(2048); // 1MB



          {
			  bfs::MakeBFS bfs(testPath.string(), blocks);
          }

          // check before that meta block is available
          {
              std::fstream input(testPath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
              assert(bfs::detail::metaBlockIsAvailable(input, 0, blocks));
              assert(bfs::detail::getNextAvailableMetaBlock(input, blocks) == 0);
              input.close();
          }

          // create a test entry sink

          {
        	  bfs::BFSEntryWriter entrySink(testPath.c_str(), "test.txt", uint64_t(13), uint64_t(0));
        	  std::string testData("Hello, world!");
        	  std::stringstream ss;
        	  ss << testData.c_str();
        	  boost::iostreams::stream<bfs::BFSEntryWriter> bfsEntryStream(entrySink);
        	  boost::iostreams::copy(ss,  bfsEntryStream);
          }

          // create a second entry sink
          {
              bfs::BFSEntryWriter entrySink(testPath.c_str(), "testB.log", uint64_t(26), uint64_t(0));
              std::string testData("Hello, world!Hello, world!");
              std::stringstream ss;
              ss << testData.c_str();
              boost::iostreams::stream<bfs::BFSEntryWriter> bfsEntryStream(entrySink);
              boost::iostreams::copy(ss,  bfsEntryStream);
          }

          // create a third entry sink with much bigger file
          {
              bfs::BFSEntryWriter entrySink(testPath.c_str(), "testLongFileName.log", uint64_t(BIG_SIZE), uint64_t(0));
              std::string testData(createLargeStringToWrite());
              std::stringstream ss;
              ss << testData.c_str();
              ss.flush();
              std::vector<uint8_t> vec(testData.begin(), testData.end());
              entrySink.write((char*)&vec.front(), testData.length());
          }

          // check after that meta block is unavailable
          std::fstream input(testPath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
          assert(!bfs::detail::metaBlockIsAvailable(input, 0, blocks));
          assert(!bfs::detail::metaBlockIsAvailable(input, 1, blocks));
          assert(bfs::detail::getNextAvailableMetaBlock(input, blocks) == 3);
          std::cout<<(bfs::detail::getFileNameForFileN(input, 0, blocks))<<std::endl;
          std::cout<<(bfs::detail::getFileNameForFileN(input, 1, blocks))<<std::endl;
          std::cout<<(bfs::detail::getFileNameForFileN(input, 2, blocks))<<std::endl;
          assert(bfs::detail::getFileNameForFileN(input, 0, blocks) == "test.txt");
          assert(bfs::detail::getFileNameForFileN(input, 1, blocks) == "testB.log");
          assert(bfs::detail::getFileNameForFileN(input, 2, blocks) == "testLongFileName.log");
          assert(bfs::detail::readFileSizeFromMetaBlock(0, blocks, input) == 13);
          assert(bfs::detail::readFileSizeFromMetaBlock(1, blocks, input) == 26);
          assert(bfs::detail::readFileSizeFromMetaBlock(2, blocks, input) == (BIG_SIZE));
          assert(bfs::detail::getFileCount(input, blocks) == 3);
          input.close();

          /*

          // create a second entry sink
          {
          bfs::BFSEntrySink entrySink(testPath.c_str(), "testB.log", uint64_t(118), uint64_t(0));
          //std::string testData("Hello, world!");
          //std::stringstream ss;
          //ss << testData.c_str();
          //boost::iostreams::stream<bfs::BFSEntrySink> bfsEntryStream(entrySink);
          // copy from the test stream to the entry stream
          //boost::iostreams::copy(ss,  bfsEntryStream);
          }

          // tests
          std::fstream input(testPath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
          assert(bfs::detail::getImageSize(input) == bytes);
          assert(bfs::detail::getFileCount(input) == uint64_t(2));
          assert(bfs::detail::getAccumulatedFileSize(input) == uint64_t(13 + 118));
          assert(bfs::detail::getSizeOfFileN(input, 1) == uint64_t(13));
          assert(bfs::detail::getSizeOfFileN(input, 2) == uint64_t(118));
          //assert(bfs::detail::getOffsetOfFileN(input, 1) == bfs::detail::METABLOCKS_BEGIN + bfs::detail::METABLOCK_SIZE + bfs::detail::getMetaDataSize(input));
          assert(bfs::detail::getFileNameForFileN(input, 1) == "test.txt");
          assert(bfs::detail::getFileNameForFileN(input, 2) == "testB.log");
          input.close();
        */
    }

    boost::filesystem::path m_uniquePath;

};
