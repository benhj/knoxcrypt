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
        	  // copy from the test stream to the entry stream
        	  boost::iostreams::copy(ss,  bfsEntryStream);
          }

          // check after that meta block is unavailable
          {
              std::fstream input(testPath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
              assert(!bfs::detail::metaBlockIsAvailable(input, 0, blocks));

              assert(bfs::detail::getNextAvailableMetaBlock(input, blocks) == 1);
              input.close();
          }


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
