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
        correctNumberOfFilesIsReported();
        firstBlockIsReportedAsBeingFree();
        blocksCanBetSetToInUse();
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
		std::pair<uint64_t, uint64_t> p = bfs::detail::getOffSetNextAvailableBlock(is);
		assert(p.first == 0);
		assert(p.second > 0);
		uint64_t bytes(blocks / 8);
		assert(p.second == 8 + 8 + bytes + bfs::detail::getMetaDataSize(blocks));
		is.close();
    }

    void blocksCanBetSetToInUse()
    {
    	std::string testImage(boost::filesystem::unique_path().string());
		boost::filesystem::path testPath = m_uniquePath / testImage;
		uint64_t blocks(2048); // 1MB
		bfs::MakeBFS bfs(testPath.string(), blocks);

		std::fstream is(testPath.string().c_str(), std::ios::in | std::ios::out | std::ios::binary);
		bfs::detail::setBlockToInUse(0, blocks, is);
		std::pair<uint64_t, uint64_t> p = bfs::detail::getOffSetNextAvailableBlock(is);
		assert(p.first == 1);

		for(int i = 1; i < 220; ++i) {
			bfs::detail::setBlockToInUse(i, blocks, is);
			p = bfs::detail::getOffSetNextAvailableBlock(is);
			assert(p.first == i + 1);
		}



    }

    boost::filesystem::path m_uniquePath;

};
