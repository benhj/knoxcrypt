

#include "bfs/BFS.hpp"
#include "test/TestHelpers.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>


class BFSTest
{
    public:
        BFSTest() : m_uniquePath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
        {
            boost::filesystem::create_directories(m_uniquePath);
            testFileExists();
            testFolderExists();
        }

        ~BFSTest()
        {
            boost::filesystem::remove_all(m_uniquePath);
        }

        bfs::FolderEntry createTestFolder(boost::filesystem::path const &p, long const blocks)
        {
            bfs::FolderEntry folder(p.string(), blocks, 0, "root");
            folder.addFileEntry("test.txt");
            folder.addFileEntry("some.log");
            folder.addFolderEntry("folderA");
            folder.addFileEntry("picture.jpg");
            folder.addFileEntry("vai.mp3");
            folder.addFolderEntry("folderB");

            bfs::FolderEntry folderA = folder.getFolderEntry("folderA");
            folderA.addFileEntry("fileA");
            folderA.addFileEntry("fileB");
            folderA.addFolderEntry("subFolderA");

            bfs::FolderEntry subFolderA = folderA.getFolderEntry("subFolderA");
            subFolderA.addFolderEntry("subFolderB");
            subFolderA.addFileEntry("fileX");
            subFolderA.addFolderEntry("subFolderC");
            subFolderA.addFileEntry("fileY");

            bfs::FolderEntry subFolderC = subFolderA.getFolderEntry("subFolderC");
            subFolderC.addFolderEntry("finalFolder");
            subFolderC.addFileEntry("finalFile.txt");

            return folder;
        }

    private:
        boost::filesystem::path m_uniquePath;

        void testFileExists()
        {
            long const blocks = 2048;
            boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
            {
                (void)createTestFolder(testPath, blocks);
            }
            bfs::BFS theBFS(testPath.string(), blocks);

            std::string testPositive("folderA/subFolderA/subFolderC/finalFile.txt");
            std::string testNegative("folderA/hello.log");

            ASSERT_EQUAL(true, theBFS.fileExists(testPositive), "BFSTest::testFileExists() positive case");
            ASSERT_EQUAL(false, theBFS.fileExists(testNegative), "BFSTest::testFileExists() negative case");
        }

        void testFolderExists()
        {
            long const blocks = 2048;
            boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
            {
                (void)createTestFolder(testPath, blocks);
            }
            bfs::BFS theBFS(testPath.string(), blocks);

            std::string testPositive("folderA/subFolderA/");
            std::string testNegative("folderA/subFolderA/subFolderX");

            ASSERT_EQUAL(true, theBFS.folderExists(testPositive), "BFSTest::testFolderExists() positive case");
            ASSERT_EQUAL(false, theBFS.folderExists(testNegative), "BFSTest::testFolderExists() negative case");
        }
};
