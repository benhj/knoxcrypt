

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
            testAddFileEntry();
            testAddFolderEntry();
            testAddFileThrowsIfParentNotFound();
            testAddFolderThrowsIfParentNotFound();
            testAddFileThrowsIfAlreadyExists();
            testAddFolderThrowsIfAlreadyExists();
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

        void testAddFileEntry()
        {
            long const blocks = 2048;
            boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
            bfs::FolderEntry root = createTestFolder(testPath, blocks);

            bfs::BFS theBFS(testPath.string(), blocks);
            theBFS.addFile("folderA/subFolderA/subFolderC/testAdded.txt");
            bfs::FolderEntry parent = root.getFolderEntry("folderA").getFolderEntry("subFolderA").getFolderEntry("subFolderC");
            bfs::OptionalEntryInfo entryInfo = parent.getEntryInfo("testAdded.txt");
            bool good = entryInfo ? true : false;
            ASSERT_EQUAL(true, good, "BFSTest::testAddFileEntry() getting info");
            ASSERT_EQUAL(bfs::EntryType::FileType, entryInfo->type(), "BFSTest::testAddFileEntry() info type");
            ASSERT_EQUAL("testAdded.txt", entryInfo->filename(), "BFSTest::testAddFileEntry() info name");
        }

        void testAddFolderEntry()
        {
            long const blocks = 2048;
            boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
            bfs::FolderEntry root = createTestFolder(testPath, blocks);

            bfs::BFS theBFS(testPath.string(), blocks);
            theBFS.addFolder("folderA/subFolderA/subFolderC/testAdded");
            bfs::FolderEntry parent = root.getFolderEntry("folderA").getFolderEntry("subFolderA").getFolderEntry("subFolderC");
            bfs::OptionalEntryInfo entryInfo = parent.getEntryInfo("testAdded");
            bool good = entryInfo ? true : false;
            ASSERT_EQUAL(true, good, "BFSTest::testAddFolderEntry() getting info");
            ASSERT_EQUAL(bfs::EntryType::FolderType, entryInfo->type(), "BFSTest::testAddFolderEntry() info type");
            ASSERT_EQUAL("testAdded", entryInfo->filename(), "BFSTest::testAddFolderEntry() info name");
        }

        void testAddFileThrowsIfParentNotFound()
        {
            long const blocks = 2048;
            boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
            bfs::FolderEntry root = createTestFolder(testPath, blocks);

            bfs::BFS theBFS(testPath.string(), blocks);
            bool caught = false;
            try {
                theBFS.addFile("folderA/subFolderA/subFolderX/testAdded");
            } catch (bfs::BFSException const &e) {
                caught = true;
                ASSERT_EQUAL(bfs::BFSException(bfs::BFSError::NotFound), e,
                        "BFSTest::testAddFileThrowIfParentNotFound() asserting error type");
            }
            ASSERT_EQUAL(true, caught, "BFSTest::testAddFileThrowIfParentNotFound() caught");
        }

        void testAddFolderThrowsIfParentNotFound()
        {
            long const blocks = 2048;
            boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
            bfs::FolderEntry root = createTestFolder(testPath, blocks);

            bfs::BFS theBFS(testPath.string(), blocks);
            bool caught = false;
            try {
                theBFS.addFolder("folderA/subFolderQ/testAdded");
            } catch (bfs::BFSException const &e) {
                caught = true;
                ASSERT_EQUAL(bfs::BFSException(bfs::BFSError::NotFound), e,
                        "BFSTest::testAddFolderThrowsIfParentNotFound() asserting error type");
            }
            ASSERT_EQUAL(true, caught, "BFSTest::testAddFolderThrowsIfParentNotFound() caught");
        }

        void testAddFileThrowsIfAlreadyExists()
        {
            long const blocks = 2048;
            boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
            bfs::FolderEntry root = createTestFolder(testPath, blocks);

            bfs::BFS theBFS(testPath.string(), blocks);
            bool caught = false;
            try {
                theBFS.addFile("folderA/subFolderA/subFolderC/finalFile.txt");
            } catch (bfs::BFSException const &e) {
                caught = true;
                ASSERT_EQUAL(bfs::BFSException(bfs::BFSError::AlreadyExists), e,
                        "BFSTest::testAddFileThrowsIfAlreadyExists() asserting error type");
            }
            ASSERT_EQUAL(true, caught, "BFSTest::testAddFileThrowsIfAlreadyExists() caught");
        }

        void testAddFolderThrowsIfAlreadyExists()
        {
            long const blocks = 2048;
            boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
            bfs::FolderEntry root = createTestFolder(testPath, blocks);

            bfs::BFS theBFS(testPath.string(), blocks);
            bool caught = false;
            try {
                theBFS.addFolder("folderA/subFolderA/subFolderC/finalFile.txt");
            } catch (bfs::BFSException const &e) {
                caught = true;
                ASSERT_EQUAL(bfs::BFSException(bfs::BFSError::AlreadyExists), e,
                        "BFSTest::testAddFolderThrowsIfAlreadyExists() asserting error type");
            }
            ASSERT_EQUAL(true, caught, "BFSTest::testAddFolderThrowsIfAlreadyExists() caught");
        }
};
