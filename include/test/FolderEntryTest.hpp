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

#include "bfs/BFSImageStream.hpp"
#include "bfs/CoreBFSIO.hpp"
#include "bfs/DetailBFS.hpp"
#include "bfs/DetailFileBlock.hpp"
#include "bfs/FileEntry.hpp"
#include "bfs/FolderEntry.hpp"
#include "bfs/MakeBFS.hpp"
#include "test/TestHelpers.hpp"

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
        testListAllEntriesEmpty();
        testListFileEntries();
        testListFolderEntries();
        testAddEntryBlockIndexRetrieval();
        testEntryRetrievalAndAppendSmallData();
        testEntryRetrievalAndAppendLargeData();
        testEntryRetrievalAppendSmallFollowedByAppendLarge();
        testEntryRetrievalAppendLargeFollowedByAppendSmall();
        testEntryRetrievalAppendSmallToFirstFileAndAppendLargeToSecond();
        testEntryRetrievalAppendLargeToFirstFileAndAppendSmallToSecond();
        testFolderEntryRetrievalAddEntries();
        testFolderEntryRetrievalAddEntriesAppendData();
        testRemoveFileEntry();
        testRemoveEmptySubFolder();
        testRemoveNonEmptySubFolder();
    }

    ~FolderEntryTest()
    {
        boost::filesystem::remove_all(m_uniquePath);
    }

  private:

    boost::filesystem::path m_uniquePath;

    bfs::FolderEntry createTestFolder(boost::filesystem::path const &p, long const blocks)
    {
        bfs::CoreBFSIO io;
        io.path = p.string();
        io.blocks = blocks;
        io.password = "abcd1234";
        bfs::FolderEntry folder(io, 0, "root");
        folder.addFileEntry("test.txt");
        folder.addFileEntry("some.log");
        folder.addFolderEntry("folderA");
        folder.addFileEntry("picture.jpg");
        folder.addFileEntry("vai.mp3");
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
            ASSERT_EQUAL(folder.getEntryInfo(1).filename(), "some.log", "testAddEntryNameRetrieval B");
            ASSERT_EQUAL(folder.getEntryInfo(2).filename(), "folderA", "testAddEntryNameRetrieval B");
            ASSERT_EQUAL(folder.getEntryInfo(3).filename(), "picture.jpg", "testAddEntryNameRetrieval C");
            ASSERT_EQUAL(folder.getEntryInfo(4).filename(), "vai.mp3", "testAddEntryNameRetrieval D");
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
        ASSERT_EQUAL(entries[1].filename(), "some.log", "testListAllEntries: filename B");
        ASSERT_EQUAL(entries[2].filename(), "folderA", "testListAllEntries: filename C");
        ASSERT_EQUAL(entries[3].filename(), "picture.jpg", "testListAllEntries: filename D");
        ASSERT_EQUAL(entries[4].filename(), "vai.mp3", "testListAllEntries: filename E");
        ASSERT_EQUAL(entries[5].filename(), "folderB", "testListAllEntries: filename F");
    }

    void testListAllEntriesEmpty()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::CoreBFSIO io;
        io.path = testPath.string();
        io.blocks = blocks;
        io.password = "abcd1234";
        bfs::FolderEntry folder(io, 0, "root");
        std::vector<bfs::EntryInfo> entries = folder.listAllEntries();
        ASSERT_EQUAL(entries.size(), 0, "testListAllEntriesEmpty: number of entries");
    }

    void testListFileEntries()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry folder = createTestFolder(testPath, blocks);
        std::vector<bfs::EntryInfo> entries = folder.listFileEntries();
        ASSERT_EQUAL(entries.size(), 4, "testListFileEntries: number of entries");
        std::vector<bfs::EntryInfo>::iterator it = entries.begin();
        ASSERT_EQUAL(entries[0].filename(), "test.txt", "testListFileEntries: filename A");
        ASSERT_EQUAL(entries[1].filename(), "some.log", "testListFileEntries: filename B");
        ASSERT_EQUAL(entries[2].filename(), "picture.jpg", "testListFileEntries: filename D");
        ASSERT_EQUAL(entries[3].filename(), "vai.mp3", "testListFileEntries: filename E");
    }

    void testListFolderEntries()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry folder = createTestFolder(testPath, blocks);
        std::vector<bfs::EntryInfo> entries = folder.listFolderEntries();
        ASSERT_EQUAL(entries.size(), 2, "testListFolderEntries: number of entries");
        std::vector<bfs::EntryInfo>::iterator it = entries.begin();
        ASSERT_EQUAL(entries[0].filename(), "folderA", "testListFolderEntries: filename C");
        ASSERT_EQUAL(entries[1].filename(), "folderB", "testListFolderEntries: filename F");
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
        bfs::FileEntry entry = folder.getFileEntry("some.log", bfs::OpenDisposition::buildAppendDisposition());
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
        bfs::FileEntry entry = folder.getFileEntry("some.log", bfs::OpenDisposition::buildAppendDisposition());
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
            bfs::CoreBFSIO io;
            io.path = testPath.string();
            io.blocks = blocks;
            io.password = "abcd1234";
            bfs::FolderEntry folder(io, 0, "root");
            bfs::FileEntry entry = folder.getFileEntry("some.log", bfs::OpenDisposition::buildAppendDisposition());
            std::vector<uint8_t> vec(testData.begin(), testData.end());
            entry.write((char*)&vec.front(), testData.length());
            entry.flush();
        }
        {
            std::string testString(createLargeStringToWrite());
            bfs::FileEntry entry = folder.getFileEntry("some.log", bfs::OpenDisposition::buildAppendDisposition());
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
            bfs::CoreBFSIO io;
            io.path = testPath.string();
            io.blocks = blocks;
            io.password = "abcd1234";
            bfs::FolderEntry folder(io, 0, "root");
            bfs::FileEntry entry = folder.getFileEntry("some.log", bfs::OpenDisposition::buildAppendDisposition());
            std::vector<uint8_t> vec(testString.begin(), testString.end());
            entry.write((char*)&vec.front(), testString.length());
        }
        {
            bfs::FileEntry entry = folder.getFileEntry("some.log", bfs::OpenDisposition::buildAppendDisposition());
            std::vector<uint8_t> vec(testData.begin(), testData.end());
            entry.write((char*)&vec.front(), testData.length());
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
            bfs::FileEntry entry = folder.getFileEntry("some.log", bfs::OpenDisposition::buildAppendDisposition());
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
            bfs::FileEntry entry = folder.getFileEntry("picture.jpg", bfs::OpenDisposition::buildAppendDisposition());
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
            bfs::FileEntry entry = folder.getFileEntry("picture.jpg", bfs::OpenDisposition::buildAppendDisposition());
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
            bfs::FileEntry entry = folder.getFileEntry("some.log", bfs::OpenDisposition::buildAppendDisposition());
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

    void testFolderEntryRetrievalAddEntries()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry folder = createTestFolder(testPath, blocks);
        bfs::FolderEntry subFolder = folder.getFolderEntry("folderA");
        subFolder.addFileEntry("subFileA");
        subFolder.addFileEntry("subFileB");
        subFolder.addFileEntry("subFileC");
        subFolder.addFileEntry("subFileD");

        // test root entries still intact
        {
            std::vector<bfs::EntryInfo> entries = folder.listAllEntries();
            ASSERT_EQUAL(entries.size(), 6, "testFolderEntryRetrievalAddEntries: root number of entries");
            std::vector<bfs::EntryInfo>::iterator it = entries.begin();
            ASSERT_EQUAL(entries[0].filename(), "test.txt", "testFolderEntryRetrievalAddEntries: root filename A");
            ASSERT_EQUAL(entries[1].filename(), "some.log", "testFolderEntryRetrievalAddEntries: root filename B");
            ASSERT_EQUAL(entries[2].filename(), "folderA", "testFolderEntryRetrievalAddEntries: root filename C");
            ASSERT_EQUAL(entries[3].filename(), "picture.jpg", "testFolderEntryRetrievalAddEntries: root filename D");
            ASSERT_EQUAL(entries[4].filename(), "vai.mp3", "testFolderEntryRetrievalAddEntries: root filename E");
            ASSERT_EQUAL(entries[5].filename(), "folderB", "testFolderEntryRetrievalAddEntries: root filename F");
        }
        // test sub folder entries exist
        {
            std::vector<bfs::EntryInfo> entries = subFolder.listAllEntries();
            ASSERT_EQUAL(entries.size(), 4, "testFolderEntryRetrievalAddEntries: subfolder number of entries");
            std::vector<bfs::EntryInfo>::iterator it = entries.begin();
            ASSERT_EQUAL(entries[0].filename(), "subFileA", "testFolderEntryRetrievalAddEntries: subFolder filename A");
            ASSERT_EQUAL(entries[1].filename(), "subFileB", "testFolderEntryRetrievalAddEntries: subFolder filename B");
            ASSERT_EQUAL(entries[2].filename(), "subFileC", "testFolderEntryRetrievalAddEntries: subFolder filename C");
            ASSERT_EQUAL(entries[3].filename(), "subFileD", "testFolderEntryRetrievalAddEntries: subFolder filename D");
        }
    }

    void testFolderEntryRetrievalAddEntriesAppendData()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry folder = createTestFolder(testPath, blocks);
        bfs::FolderEntry subFolder = folder.getFolderEntry("folderA");
        subFolder.addFileEntry("subFileA");
        subFolder.addFileEntry("subFileB");
        subFolder.addFileEntry("subFileC");
        subFolder.addFileEntry("subFileD");

        std::string testData("some test data!");
        bfs::FileEntry entry = subFolder.getFileEntry("subFileB", bfs::OpenDisposition::buildAppendDisposition());
        std::vector<uint8_t> vec(testData.begin(), testData.end());
        entry.write((char*)&vec.front(), testData.length());
        entry.flush();
        entry.seek(0); // seek to start since retrieving from folder automatically seeks to end
        vec.resize(entry.fileSize());
        entry.read((char*)&vec.front(), entry.fileSize());
        std::string result(vec.begin(), vec.end());
        ASSERT_EQUAL(result, testData, "testFolderEntryRetrievalAddEntriesAppendData");
    }

    void testRemoveFileEntry()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry folder = createTestFolder(testPath, blocks);
        folder.removeFileEntry("test.txt");
        std::vector<bfs::EntryInfo> entries = folder.listAllEntries();
        ASSERT_EQUAL(entries.size(), 5, "testRemoveFileEntry: number of entries after removal");
    }

    void testRemoveEmptySubFolder()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
        bfs::FolderEntry folder = createTestFolder(testPath, blocks);
        folder.removeFolderEntry("folderA");
        std::vector<bfs::EntryInfo> entries = folder.listAllEntries();
        ASSERT_EQUAL(entries.size(), 5, "testRemoveEmptySubFolder: number of entries after removal");
    }

    void testRemoveNonEmptySubFolder()
    {
        {
            long const blocks = 2048;
            boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
            bfs::FolderEntry folder = createTestFolder(testPath, blocks);
            bfs::FolderEntry subFolder = folder.getFolderEntry("folderA");
            subFolder.addFileEntry("subFileA");
            subFolder.addFileEntry("subFileB");
            subFolder.addFileEntry("subFileC");
            subFolder.addFileEntry("subFileD");

            std::string testData("some test data!");
            bfs::FileEntry entry = subFolder.getFileEntry("subFileB", bfs::OpenDisposition::buildAppendDisposition());
            std::vector<uint8_t> vec(testData.begin(), testData.end());
            entry.write((char*)&vec.front(), testData.length());
            entry.flush();
        }
        {
            long const blocks = 2048;
            boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
            bfs::FolderEntry folder = createTestFolder(testPath, blocks);
            folder.removeFolderEntry("folderA");
            std::vector<bfs::EntryInfo> entries = folder.listAllEntries();
            ASSERT_EQUAL(entries.size(), 5, "testRemoveNonEmptySubFolder: number of entries after removal");
        }
    }

};
