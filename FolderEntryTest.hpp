#include "BFSImageStream.hpp"
#include "DetailBFS.hpp"
#include "DetailFileBlock.hpp"
#include "FileEntry.hpp"
#include "FolderEntry.hpp"
#include "MakeBFS.hpp"
#include "TestHelpers.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/stream.hpp>

#include <cassert>
#include <sstream>

class FolderEntryTest
{


  public:
    FolderEntryTest() : m_uniquePath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
    {
        boost::filesystem::create_directories(m_uniquePath);
        testAddEntryNameRetrieval();
        testListAllEntries();
        testAddEntryBlockIndexRetrieval();
        testEntryRetrievalAndAppendSmallData();
        testEntryRetrievalAndAppendLargeData();
        testEntryRetrievalAppendSmallFollowedByAppendLarge();
        testEntryRetrievalAppendLargeFollowedByAppendSmall();
        testEntryRetrievalAppendSmallToFirstFileAndAppendLargeToSecond();
        testEntryRetrievalAppendLargeToFirstFileAndAppendSmallToSecond();
    }

    ~FolderEntryTest()
    {
        boost::filesystem::remove_all(m_uniquePath);
    }

  private:

    boost::filesystem::path m_uniquePath;

    bfs::FolderEntry createTestFolder(boost::filesystem::path const &p, long const blocks)
    {
        bfs::FolderEntry folder(p.string(), blocks, 0, "root");
        folder.addFileEntry("test.txt");
        folder.addFileEntry("fucker.log");
        folder.addFolderEntry("folderA");
        folder.addFileEntry("crap.jpg");
        folder.addFileEntry("shitter.mp3");
        folder.addFolderEntry("folderB");
        return folder;
    }

    void testAddEntryNameRetrieval()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);

        {
            bfs::FolderEntry folder = createTestFolder(testPath, blocks);
            ASSERT_EQUAL(folder.getEntryInfo(0).filename(), "test.txt", "testAddEntryNameRetrieval A");
            ASSERT_EQUAL(folder.getEntryInfo(1).filename(), "fucker.log", "testAddEntryNameRetrieval B");
            ASSERT_EQUAL(folder.getEntryInfo(2).filename(), "folderA", "testAddEntryNameRetrieval B");
            ASSERT_EQUAL(folder.getEntryInfo(3).filename(), "crap.jpg", "testAddEntryNameRetrieval C");
            ASSERT_EQUAL(folder.getEntryInfo(4).filename(), "shitter.mp3", "testAddEntryNameRetrieval D");
            ASSERT_EQUAL(folder.getEntryInfo(5).filename(), "folderB", "testAddEntryNameRetrieval B");
        }
    }

    void testListAllEntries()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry folder = createTestFolder(testPath, blocks);
        std::vector<bfs::EntryInfo> entries = folder.listAllEntries();
        ASSERT_EQUAL(entries.size(), 6, "testListAllEntries: number of entries");
        std::vector<bfs::EntryInfo>::iterator it = entries.begin();
        ASSERT_EQUAL(entries[0].filename(), "test.txt", "testListAllEntries: filename A");
        ASSERT_EQUAL(entries[1].filename(), "fucker.log", "testListAllEntries: filename B");
        ASSERT_EQUAL(entries[2].filename(), "folderA", "testListAllEntries: filename C");
        ASSERT_EQUAL(entries[3].filename(), "crap.jpg", "testListAllEntries: filename D");
        ASSERT_EQUAL(entries[4].filename(), "shitter.mp3", "testListAllEntries: filename E");
        ASSERT_EQUAL(entries[5].filename(), "folderB", "testListAllEntries: filename F");
    }

    void testAddEntryBlockIndexRetrieval()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry folder = createTestFolder(testPath, blocks);

        uint64_t b1 = folder.getEntryInfo(0).firstFileBlock();
        uint64_t b2 = folder.getEntryInfo(1).firstFileBlock();
        uint64_t b3 = folder.getEntryInfo(2).firstFileBlock();
        uint64_t b4 = folder.getEntryInfo(3).firstFileBlock();

        //
        // when entry 0 added to root folder, root folder block starts at 0
        // this creates a new file entry at block 1
        // when entry 1 added, still at block 0, this creates a new file entry
        // at block 2; folder entry ends up also writing to block 3
        // when entry 2 added, root folder at block 3, this creates a new file
        // entry at block 4
        // when entry 3 added, root folder still at block 3, this creates a new
        // file entry at block 5. Root folder ends up also writing to block 6
        //
        ASSERT_EQUAL(b1, 1, "testAddEntryBlockIndexRetrieval A");
        ASSERT_EQUAL(b2, 2, "testAddEntryBlockIndexRetrieval B");
        ASSERT_EQUAL(b3, 4, "testAddEntryBlockIndexRetrieval C");
        ASSERT_EQUAL(b4, 5, "testAddEntryBlockIndexRetrieval D");
    }

    void testEntryRetrievalAndAppendSmallData()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry folder = createTestFolder(testPath, blocks);
        std::string testData("some test data!");
        bfs::FileEntry entry = folder.getFileEntry("fucker.log");
        std::vector<uint8_t> vec(testData.begin(), testData.end());
        entry.write((char*)&vec.front(), testData.length());
        entry.flush();
        entry.seek(0); // seek to start since retrieving from folder automatically seeks to end
        vec.resize(entry.fileSize());
        entry.read((char*)&vec.front(), entry.fileSize());
        std::string result(vec.begin(), vec.end());
        ASSERT_EQUAL(result, testData, "testEntryRetrievalAndAppendSmallData");

    }

    void testEntryRetrievalAndAppendLargeData()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry folder = createTestFolder(testPath, blocks);
        std::string testString(createLargeStringToWrite());
        bfs::FileEntry entry = folder.getFileEntry("fucker.log");
        std::vector<uint8_t> vec(testString.begin(), testString.end());
        entry.write((char*)&vec.front(), testString.length());
        entry.flush();
        entry.seek(0); // seek to start since retrieving from folder automatically seeks to end
        vec.resize(entry.fileSize());
        entry.read((char*)&vec.front(), entry.fileSize());
        std::string result(vec.begin(), vec.end());
        ASSERT_EQUAL(result, testString, "testEntryRetrievalAndAppendLargeData");
    }

    void testEntryRetrievalAppendSmallFollowedByAppendLarge()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry folder = createTestFolder(testPath, blocks);
        std::string testData("some test data!");
        {
            bfs::FolderEntry folder(testPath.string(), blocks, 0, "root");
            bfs::FileEntry entry = folder.getFileEntry("fucker.log");
            std::vector<uint8_t> vec(testData.begin(), testData.end());
            entry.write((char*)&vec.front(), testData.length());
            entry.flush();
        }
        {
            std::string testString(createLargeStringToWrite());
            bfs::FileEntry entry = folder.getFileEntry("fucker.log");
            std::vector<uint8_t> vec(testString.begin(), testString.end());
            entry.write((char*)&vec.front(), testString.length());
            entry.flush();
            entry.seek(0); // seek to start since retrieving from folder automatically seeks to end
            vec.resize(entry.fileSize());
            entry.read((char*)&vec.front(), entry.fileSize());
            std::string result(vec.begin(), vec.end());
            std::string concat(testData.append(testString));
            ASSERT_EQUAL(result, concat, "testEntryRetrievalAppendSmallFollowedByAppendLarge");
        }
    }

    void testEntryRetrievalAppendLargeFollowedByAppendSmall()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry folder = createTestFolder(testPath, blocks);
        std::string testData("some test data!");
        std::string testString(createLargeStringToWrite());
        {
            bfs::FolderEntry folder(testPath.string(), blocks, 0, "root");
            bfs::FileEntry entry = folder.getFileEntry("fucker.log");
            std::vector<uint8_t> vec(testString.begin(), testString.end());
            entry.write((char*)&vec.front(), testString.length());
        }
        {
            bfs::FileEntry entry = folder.getFileEntry("fucker.log");
            std::vector<uint8_t> vec(testData.begin(), testData.end());
            entry.write((char*)&vec.front(), testData.length());
            entry.flush();
            entry.flush();
            entry.seek(0); // seek to start since retrieving from folder automatically seeks to end
            vec.resize(entry.fileSize());
            entry.read((char*)&vec.front(), entry.fileSize());
            std::string result(vec.begin(), vec.end());
            std::string concat(testString.append(testData));
            ASSERT_EQUAL(result, concat, "testEntryRetrievalAppendLargeFollowedByAppendSmall");
        }
    }

    void testEntryRetrievalAppendSmallToFirstFileAndAppendLargeToSecond()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry folder = createTestFolder(testPath, blocks);
        {
            std::string testData("some test data!");
            bfs::FileEntry entry = folder.getFileEntry("fucker.log");
            std::vector<uint8_t> vec(testData.begin(), testData.end());
            entry.write((char*)&vec.front(), testData.length());
            entry.flush();
            entry.seek(0); // seek to start since retrieving from folder automatically seeks to end
            vec.resize(entry.fileSize());
            entry.read((char*)&vec.front(), entry.fileSize());
            std::string result(vec.begin(), vec.end());
        }

        {
            std::string testString(createLargeStringToWrite());
            bfs::FileEntry entry = folder.getFileEntry("crap.jpg");
            std::vector<uint8_t> vec(testString.begin(), testString.end());
            entry.write((char*)&vec.front(), testString.length());
            entry.flush();
            entry.seek(0); // seek to start since retrieving from folder automatically seeks to end
            vec.resize(entry.fileSize());
            entry.read((char*)&vec.front(), entry.fileSize());
            std::string result(vec.begin(), vec.end());
            ASSERT_EQUAL(result, testString, "testEntryRetrievalAppendSmallToFirstFileAndAppendLargeToSecond");
        }
    }

    void testEntryRetrievalAppendLargeToFirstFileAndAppendSmallToSecond()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry folder = createTestFolder(testPath, blocks);

        {
            std::string testString(createLargeStringToWrite());
            bfs::FileEntry entry = folder.getFileEntry("crap.jpg");
            std::vector<uint8_t> vec(testString.begin(), testString.end());
            entry.write((char*)&vec.front(), testString.length());
            entry.flush();
            entry.seek(0); // seek to start since retrieving from folder automatically seeks to end
            vec.resize(entry.fileSize());
            entry.read((char*)&vec.front(), entry.fileSize());
            std::string result(vec.begin(), vec.end());
        }
        {
            std::string testData("some test data!");
            bfs::FileEntry entry = folder.getFileEntry("fucker.log");
            std::vector<uint8_t> vec(testData.begin(), testData.end());
            entry.write((char*)&vec.front(), testData.length());
            entry.flush();
            entry.seek(0); // seek to start since retrieving from folder automatically seeks to end
            vec.resize(entry.fileSize());
            entry.read((char*)&vec.front(), entry.fileSize());
            std::string result(vec.begin(), vec.end());
            ASSERT_EQUAL(result, testData, "testEntryRetrievalAppendLargeToFirstFileAndAppendSmallToSecond");
        }
    }

};
