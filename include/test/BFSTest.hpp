/*
  The MIT License (MIT)

  Copyright (c) 2013 Ben H.D. Jones

  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
  the Software, and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "bfs/BFS.hpp"
#include "bfs/CoreBFSIO.hpp"
#include "test/TestHelpers.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>

#include <sstream>


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
        testRemoveFileEntry();
        testRemoveFileEntryThrowsIfBadParent();
        testRemoveFileThrowsIfNotFound();
        testRemoveFileThrowsIfFolder();
        testRemoveEmptyFolder();
        testRemoveFolderWithMustBeEmptyThrowsIfNonEmpty();
        testRemoveNonEmptyFolder();
        testRemoveNonExistingFolderThrows();
        testWriteToStream();
        testListAllEntriesEmpty();
        testMoveFileSameFolder();
        testMoveFileToSubFolder();
        testMoveFileFromSubFolderToParentFolder();
    }

    ~BFSTest()
    {
        boost::filesystem::remove_all(m_uniquePath);
    }

    bfs::FolderEntry createTestFolder(boost::filesystem::path const &p, long const blocks)
    {
        bfs::CoreBFSIO io = createTestIO(p);
        bfs::FolderEntry folder(io, 0, "root");
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
        bfs::CoreBFSIO io = createTestIO(testPath);
        bfs::BFS theBFS(io);

        std::string testPositive("/folderA/subFolderA/subFolderC/finalFile.txt");
        std::string testNegative("/folderA/hello.log");

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
        bfs::CoreBFSIO io = createTestIO(testPath);
        bfs::BFS theBFS(io);

        std::string testPositive("/folderA/subFolderA/");
        std::string testNegative("/folderA/subFolderA/subFolderX");

        ASSERT_EQUAL(true, theBFS.folderExists(testPositive), "BFSTest::testFolderExists() positive case");
        ASSERT_EQUAL(false, theBFS.folderExists(testNegative), "BFSTest::testFolderExists() negative case");
    }

    void testAddFileEntry()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry root = createTestFolder(testPath, blocks);

        bfs::CoreBFSIO io = createTestIO(testPath);
        bfs::BFS theBFS(io);

        theBFS.addFile("/folderA/subFolderA/subFolderC/testAdded.txt");
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

        bfs::CoreBFSIO io = createTestIO(testPath);
        bfs::BFS theBFS(io);

        theBFS.addFolder("/folderA/subFolderA/subFolderC/testAdded");
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

        bfs::CoreBFSIO io = createTestIO(testPath);
        bfs::BFS theBFS(io);

        bool caught = false;
        try {
            theBFS.addFile("/folderA/subFolderA/subFolderX/testAdded");
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

        bfs::CoreBFSIO io = createTestIO(testPath);
        bfs::BFS theBFS(io);

        bool caught = false;
        try {
            theBFS.addFolder("/folderA/subFolderQ/testAdded");
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

        bfs::CoreBFSIO io = createTestIO(testPath);
        bfs::BFS theBFS(io);

        bool caught = false;
        try {
            theBFS.addFile("/folderA/subFolderA/subFolderC/finalFile.txt");
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

        bfs::CoreBFSIO io = createTestIO(testPath);
        bfs::BFS theBFS(io);

        bool caught = false;
        try {
            theBFS.addFolder("/folderA/subFolderA/subFolderC/finalFile.txt");
        } catch (bfs::BFSException const &e) {
            caught = true;
            ASSERT_EQUAL(bfs::BFSException(bfs::BFSError::AlreadyExists), e,
                         "BFSTest::testAddFolderThrowsIfAlreadyExists() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "BFSTest::testAddFolderThrowsIfAlreadyExists() caught");
    }

    void testRemoveFileEntry()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry root = createTestFolder(testPath, blocks);
        bfs::CoreBFSIO io = createTestIO(testPath);
        bfs::BFS theBFS(io);
        theBFS.removeFile("/folderA/subFolderA/subFolderC/finalFile.txt");
        bfs::OptionalEntryInfo info = root.getFolderEntry("folderA")
            .getFolderEntry("subFolderA")
            .getFolderEntry("subFolderC")
            .getEntryInfo("finalFile.txt");
        bool exists = info ? true : false;
        ASSERT_EQUAL(false, exists, "BFSTest::testRemoveFileEntry()");
    }

    void testRemoveFileEntryThrowsIfBadParent()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry root = createTestFolder(testPath, blocks);

        bfs::CoreBFSIO io = createTestIO(testPath);
        bfs::BFS theBFS(io);
        bool caught = false;
        try {
            theBFS.removeFile("/folderA/subFolderA/subFolderX/finalFile.txt");
        } catch (bfs::BFSException const &e) {
            caught = true;
            ASSERT_EQUAL(bfs::BFSException(bfs::BFSError::NotFound), e,
                         "BFSTest::testRemoveFileEntryThrowsIfBadParent() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "BFSTest::testRemoveFileEntryThrowsIfBadParent() caught");
    }

    void testRemoveFileThrowsIfNotFound()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry root = createTestFolder(testPath, blocks);

        bfs::CoreBFSIO io = createTestIO(testPath);
        bfs::BFS theBFS(io);
        bool caught = false;
        try {
            theBFS.removeFile("/folderA/subFolderA/subFolderC/finalFileB.txt");
        } catch (bfs::BFSException const &e) {
            caught = true;
            ASSERT_EQUAL(bfs::BFSException(bfs::BFSError::NotFound), e,
                         "BFSTest::testRemoveFileThrowsIfNotFound() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "BFSTest::testRemoveFileThrowsIfNotFound() caught");
    }

    void testRemoveFileThrowsIfFolder()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry root = createTestFolder(testPath, blocks);

        bfs::CoreBFSIO io = createTestIO(testPath);
        bfs::BFS theBFS(io);
        bool caught = false;
        try {
            theBFS.removeFile("/folderA/subFolderA/subFolderC/finalFolder");
        } catch (bfs::BFSException const &e) {
            caught = true;
            ASSERT_EQUAL(bfs::BFSException(bfs::BFSError::NotFound), e,
                         "BFSTest::testRemoveFileThrowsIfFolder() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "BFSTest::testRemoveFileThrowsIfFolder() caught");
    }

    void testRemoveEmptyFolder()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry root = createTestFolder(testPath, blocks);
        bfs::CoreBFSIO io = createTestIO(testPath);
        bfs::BFS theBFS(io);
        theBFS.removeFolder("/folderA/subFolderA/subFolderC/finalFolder",
                            bfs::FolderRemovalType::MustBeEmpty);


        bfs::OptionalEntryInfo info = root.getFolderEntry("folderA")
            .getFolderEntry("subFolderA")
            .getFolderEntry("subFolderC")
            .getEntryInfo("finalFolder");
        bool exists = info ? true : false;
        ASSERT_EQUAL(false, exists, "BFSTest::testRemoveEmptyFolder()");
    }

    void testRemoveFolderWithMustBeEmptyThrowsIfNonEmpty()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry root = createTestFolder(testPath, blocks);
        bfs::CoreBFSIO io = createTestIO(testPath);
        bfs::BFS theBFS(io);
        bool caught = false;
        try {
            theBFS.removeFolder("/folderA/subFolderA/",
                                bfs::FolderRemovalType::MustBeEmpty);
        } catch (bfs::BFSException const &e) {
            caught = true;
            ASSERT_EQUAL(bfs::BFSException(bfs::BFSError::FolderNotEmpty), e,
                         "BFSTest::testRemoveFolderWithMustBeEmptyThrowsIfNonEmpty() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "BFSTest::testRemoveFolderWithMustBeEmptyThrowsIfNonEmpty() caught");
    }

    void testRemoveNonEmptyFolder()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry root = createTestFolder(testPath, blocks);
        bfs::CoreBFSIO io = createTestIO(testPath);
        bfs::BFS theBFS(io);
        theBFS.removeFolder("/folderA/subFolderA/",
                            bfs::FolderRemovalType::Recursive);
        bfs::OptionalEntryInfo info = root.getFolderEntry("folderA")
            .getEntryInfo("subFolderA");
        bool exists = info ? true : false;
        ASSERT_EQUAL(false, exists, "BFSTest::testRemoveNonEmptyFolder()");
    }

    void testRemoveNonExistingFolderThrows()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry root = createTestFolder(testPath, blocks);
        bfs::CoreBFSIO io = createTestIO(testPath);
        bfs::BFS theBFS(io);
        bool caught = false;
        try {
            theBFS.removeFolder("/folderA/subFolderQ/",
                                bfs::FolderRemovalType::MustBeEmpty);
        } catch (bfs::BFSException const &e) {
            caught = true;
            ASSERT_EQUAL(bfs::BFSException(bfs::BFSError::NotFound), e,
                         "BFSTest::testRemoveNonExistingFolderThrows() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "BFSTest::testRemoveNonExistingFolderThrows() caught");
    }

    void testWriteToStream()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry root = createTestFolder(testPath, blocks);
        bfs::CoreBFSIO io = createTestIO(testPath);
        bfs::BFS theBFS(io);

        // open file and append to end of it
        std::string const &testString(createLargeStringToWrite());

        bfs::FileEntryDevice device = theBFS.openFile("/folderA/subFolderA/fileX",
                                                      bfs::OpenDisposition::buildAppendDisposition());

        std::streampos wrote = device.write(testString.c_str(), testString.length());
        ASSERT_EQUAL(wrote, testString.length(), "BFSTest::testWriteToStream() bytesWrote");
        (void)device.seek(0, std::ios_base::beg);

        // check content
        std::vector<uint8_t> buffer;
        buffer.resize(testString.length());
        std::streampos bytesRead = device.read((char*)&buffer.front(), testString.length());
        ASSERT_EQUAL(testString.length(), bytesRead, "BFSTest::testWriteToStream() bytesRead");
        std::string recovered(buffer.begin(), buffer.end());
        ASSERT_EQUAL(testString, recovered, "BFSTest::testWriteToStream() content");
    }

    void testListAllEntriesEmpty()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::CoreBFSIO io = createTestIO(testPath);
        bfs::BFS theBFS(io);
        bfs::FolderEntry fe = theBFS.getCurrent("/");
        std::vector<bfs::EntryInfo> infos = fe.listAllEntries();
        ASSERT_EQUAL(infos.empty(), true, "BFSTest::testListAllEntriesEmpty()");
    }

    void testMoveFileSameFolder()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        (void)createTestFolder(testPath, blocks);
        bfs::CoreBFSIO io = createTestIO(testPath);
        bfs::BFS theBFS(io);
        theBFS.renameEntry("/folderA/fileA", "/folderA/renamed.txt");
        ASSERT_EQUAL(false, theBFS.fileExists("/folderA/fileA"), "BFSTest::testMoveFileSameFolder() original removed");
        ASSERT_EQUAL(true, theBFS.fileExists("/folderA/renamed.txt"), "BFSTest::testMoveFileSameFolder() new version");
    }

    void testMoveFileToSubFolder()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        (void)createTestFolder(testPath, blocks);
        bfs::CoreBFSIO io = createTestIO(testPath);
        bfs::BFS theBFS(io);
        theBFS.renameEntry("/folderA/fileA", "/folderA/subFolderA/renamed.txt");
        ASSERT_EQUAL(false, theBFS.fileExists("/folderA/fileA"), "BFSTest::testMoveFileToSubFolderFolder() original removed");
        ASSERT_EQUAL(true, theBFS.fileExists("/folderA/subFolderA/renamed.txt"), "BFSTest::testMoveFileToSubFolderFolder() new version");
    }

    void testMoveFileFromSubFolderToParentFolder()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        (void)createTestFolder(testPath, blocks);
        bfs::CoreBFSIO io = createTestIO(testPath);
        bfs::BFS theBFS(io);
        theBFS.renameEntry("/folderA/subFolderA/fileX", "/folderA/renamed.txt");
        ASSERT_EQUAL(false, theBFS.fileExists("/folderA/subFolderA/fileX"), "BFSTest::testMoveFileToSubFolderFolder() original removed");
        ASSERT_EQUAL(true, theBFS.fileExists("/folderA/renamed.txt"), "BFSTest::testMoveFileToSubFolderFolder() new version");
    }
};
