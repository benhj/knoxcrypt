#include "DetailBFS.hpp"
#include "DetailFileBlock.hpp"
#include "FolderEntry.hpp"
#include "MakeBFS.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/stream.hpp>

#include <cassert>
#include <sstream>
#include <fstream>


class FolderEntryTest
{


  public:
	FolderEntryTest() : m_uniquePath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
    {
        boost::filesystem::create_directories(m_uniquePath);
        testAddEntry();
    }

    ~FolderEntryTest()
    {
        boost::filesystem::remove_all(m_uniquePath);
    }

  private:

    boost::filesystem::path m_uniquePath;

    void testAddEntry()
    {
        std::string testImage(boost::filesystem::unique_path().string());
        boost::filesystem::path testPath = m_uniquePath / testImage;
        uint64_t blocks(2048); // 1MB

        {
			  bfs::MakeBFS bfs(testPath.string(), blocks);
        }

        {
			bfs::FolderEntry folder(testPath.string(), blocks, 0, "root");
			folder.addFileEntry("test.txt");
			folder.addFileEntry("fucker.log");
			folder.addFileEntry("crap.jpg");
			folder.addFileEntry("shitter.mp3");
        }
        {
        	bfs::FolderEntry folder(testPath.string(), blocks, 0, "root");
			assert(folder.getEntryName(0) == "test.txt");
			assert(folder.getEntryName(1) == "fucker.log");
			assert(folder.getEntryName(2) == "crap.jpg");
			assert(folder.getEntryName(3) == "shitter.mp3");
        }

        std::cout<<"Test adding folder entries (files) passed.."<<std::endl;

    }
};
