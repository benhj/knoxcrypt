/*
  Copyright (c) <2013-2015>, <BenHJ>
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
  3. Neither the name of the copyright holder nor the names of its contributors
  may be used to endorse or promote products derived from this software without
  specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "teasafe/TeaSafeImageStream.hpp"
#include "teasafe/CoreTeaSafeIO.hpp"
#include "teasafe/File.hpp"
#include "teasafe/LeafFolder.hpp"
#include "teasafe/detail/DetailTeaSafe.hpp"
#include "teasafe/detail/DetailFileBlock.hpp"
#include "test/TestHelpers.hpp"
#include "utility/MakeTeaSafe.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/stream.hpp>

#include <cassert>
#include <sstream>

class LeafFolderTest
{

  public:
    LeafFolderTest() : m_uniquePath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
    {
        boost::filesystem::create_directories(m_uniquePath);
        testAddEntryNameRetrieval();
        testListAllEntries();
        testListAllEntriesEmpty();
        testListFileEntries();
        testListFolderEntries();
        //commented out since block-size dependent
        //testAddEntryBlockIndexRetrieval();
        testEntryRetrievalAndAppendSmallData();
        testEntryRetrievalAndAppendLargeData();
        testEntryRetrievalAppendSmallFollowedByAppendLarge();
        testEntryRetrievalAppendLargeFollowedByAppendSmall();
        testEntryRetrievalAppendSmallToFirstFileAndAppendLargeToSecond();
        testEntryRetrievalAppendLargeToFirstFileAndAppendSmallToSecond();
        testLeafFolderRetrievalAddEntries();
        testLeafFolderRetrievalAddEntriesAppendData();
        testRemoveFile();
        testRemoveEmptySubFolder();
        testRemoveNonEmptySubFolder();
    }

    ~LeafFolderTest()
    {
        boost::filesystem::remove_all(m_uniquePath);
    }

  private:

    boost::filesystem::path m_uniquePath;

    teasafe::LeafFolder createTestFolder(boost::filesystem::path const &p)
    {
        teasafe::SharedCoreIO io(createTestIO(p));
        teasafe::LeafFolder folder(io, 0, std::string("root"));
        folder.addFile("test.txt");
        folder.addFile("some.log");
        folder.addLeafFolder("folderA");
        folder.addFile("picture.jpg");
        folder.addFile("vai.mp3");
        folder.addLeafFolder("folderB");
        return folder;
    }

    void testAddEntryNameRetrieval()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);

        {
            teasafe::LeafFolder folder = createTestFolder(testPath);
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
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::LeafFolder folder = createTestFolder(testPath);
        auto entries = folder.listAllEntries();
        ASSERT_EQUAL(entries.size(), 6, "testListAllEntries: number of entries");
        ASSERT_UNEQUAL(entries.find("test.txt"), (entries.end()), "testListAllEntries: filename A");
        ASSERT_UNEQUAL(entries.find("some.log"), (entries.end()), "testListAllEntries: filename B");
        ASSERT_UNEQUAL(entries.find("folderA"), (entries.end()), "testListAllEntries: filename C");
        ASSERT_UNEQUAL(entries.find("picture.jpg"), (entries.end()), "testListAllEntries: filename D");
        ASSERT_UNEQUAL(entries.find("vai.mp3"), (entries.end()), "testListAllEntries: filename E");
        ASSERT_UNEQUAL(entries.find("folderB"), (entries.end()), "testListAllEntries: filename F");
    }

    void testListAllEntriesEmpty()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::LeafFolder folder(io, 0, std::string("root"));
        auto entries = folder.listAllEntries();
        ASSERT_EQUAL(entries.size(), 0, "testListAllEntriesEmpty: number of entries");
    }

    void testListFileEntries()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::LeafFolder folder = createTestFolder(testPath);
        auto entries = folder.listFileEntries();
        ASSERT_EQUAL(entries.size(), 4, "testListFileEntries: number of entries");
        ASSERT_EQUAL(entries[0]->filename(), "test.txt", "testListFileEntries: filename A");
        ASSERT_EQUAL(entries[1]->filename(), "some.log", "testListFileEntries: filename B");
        ASSERT_EQUAL(entries[2]->filename(), "picture.jpg", "testListFileEntries: filename D");
        ASSERT_EQUAL(entries[3]->filename(), "vai.mp3", "testListFileEntries: filename E");
    }

    void testListFolderEntries()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::LeafFolder folder = createTestFolder(testPath);
        auto entries = folder.listFolderEntries();
        ASSERT_EQUAL(entries.size(), 2, "testListFolderEntries: number of entries");
        ASSERT_EQUAL(entries[0]->filename(), "folderA", "testListFolderEntries: filename C");
        ASSERT_EQUAL(entries[1]->filename(), "folderB", "testListFolderEntries: filename F");
    }

    /* Commented out, since block size dependent
       void testAddEntryBlockIndexRetrieval()
       {
       long const blocks = 2048;
       boost::filesystem::path testPath = buildImage(m_uniquePath, blocks);
       teasafe::LeafFolder folder = createTestFolder(testPath, blocks);

       uint64_t b1 = folder.getEntryInfo(0).firstFileBlock();
       uint64_t b2 = folder.getEntryInfo(1).firstFileBlock();
       uint64_t b3 = folder.getEntryInfo(2).firstFileBlock();
       uint64_t b4 = folder.getEntryInfo(3).firstFileBlock();

       // !!!NOTE: These tests are contingent on the file block size
       // being 512 bytes and will not work on other (e.g. 4096) byte block sizes
       ASSERT_EQUAL(b1, 1, "testAddEntryBlockIndexRetrieval A");
       ASSERT_EQUAL(b2, 2, "testAddEntryBlockIndexRetrieval B");
       ASSERT_EQUAL(b3, 4, "testAddEntryBlockIndexRetrieval C");
       ASSERT_EQUAL(b4, 5, "testAddEntryBlockIndexRetrieval D");
       }*/

    void testEntryRetrievalAndAppendSmallData()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::LeafFolder folder = createTestFolder(testPath);
        std::string testData("some test data!");
        teasafe::File entry = *folder.getFile("some.log", teasafe::OpenDisposition::buildAppendDisposition());
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
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::LeafFolder folder = createTestFolder(testPath);
        std::string testString(createLargeStringToWrite());
        teasafe::File entry = *folder.getFile("some.log", teasafe::OpenDisposition::buildAppendDisposition());
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
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::LeafFolder folder = createTestFolder(testPath);
        std::string testData("some test data!");
        {
            teasafe::SharedCoreIO io(createTestIO(testPath));
            teasafe::LeafFolder folder(io, 0, std::string("root"));
            teasafe::File entry = *folder.getFile("some.log", teasafe::OpenDisposition::buildAppendDisposition());
            std::vector<uint8_t> vec(testData.begin(), testData.end());
            entry.write((char*)&vec.front(), testData.length());
            entry.flush();
        }
        {
            std::string testString(createLargeStringToWrite());
            teasafe::File entry = *folder.getFile("some.log", teasafe::OpenDisposition::buildAppendDisposition());
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
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::LeafFolder folder = createTestFolder(testPath);
        std::string testData("some test data!");
        std::string testString(createLargeStringToWrite());
        {
            teasafe::SharedCoreIO io(createTestIO(testPath));
            teasafe::LeafFolder folder(io, 0, std::string("root"));
            teasafe::File entry = *folder.getFile("some.log", teasafe::OpenDisposition::buildAppendDisposition());
            std::vector<uint8_t> vec(testString.begin(), testString.end());
            entry.write((char*)&vec.front(), testString.length());
            entry.flush();
        }
        {
            teasafe::File entry = *folder.getFile("some.log", teasafe::OpenDisposition::buildAppendDisposition());
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
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::LeafFolder folder = createTestFolder(testPath);
        {
            std::string testData("some test data!");
            teasafe::File entry = *folder.getFile("some.log", teasafe::OpenDisposition::buildAppendDisposition());
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
            teasafe::File entry = *folder.getFile("picture.jpg", teasafe::OpenDisposition::buildAppendDisposition());
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
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::LeafFolder folder = createTestFolder(testPath);

        {
            std::string testString(createLargeStringToWrite());
            teasafe::File entry = *folder.getFile("picture.jpg", teasafe::OpenDisposition::buildAppendDisposition());
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
            teasafe::File entry = *folder.getFile("some.log", teasafe::OpenDisposition::buildAppendDisposition());
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

    void testLeafFolderRetrievalAddEntries()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::LeafFolder folder = createTestFolder(testPath);
        teasafe::LeafFolder subFolder = *folder.getLeafFolder("folderA");
        subFolder.addFile("subFileA");
        subFolder.addFile("subFileB");
        subFolder.addFile("subFileC");
        subFolder.addFile("subFileD");

        // test root entries still intact
        {
            auto entries = folder.listAllEntries();
            ASSERT_EQUAL(entries.size(), 6, "testLeafFolderRetrievalAddEntries: root number of entries");
            ASSERT_UNEQUAL(entries.find("test.txt"), (entries.end()), "testLeafFolderRetrievalAddEntries: root filename A");
            ASSERT_UNEQUAL(entries.find("some.log"), (entries.end()), "testLeafFolderRetrievalAddEntries: root filename B");
            ASSERT_UNEQUAL(entries.find("folderA"), (entries.end()), "testLeafFolderRetrievalAddEntries: root filename C");
            ASSERT_UNEQUAL(entries.find("picture.jpg"), (entries.end()), "testLeafFolderRetrievalAddEntries: root filename D");
            ASSERT_UNEQUAL(entries.find("vai.mp3"), (entries.end()), "testLeafFolderRetrievalAddEntries: root filename E");
            ASSERT_UNEQUAL(entries.find("folderB"), (entries.end()), "testLeafFolderRetrievalAddEntries: root filename F");
        }
        // test sub folder entries exist
        {
            auto entries = subFolder.listAllEntries();
            ASSERT_EQUAL(entries.size(), 4, "testLeafFolderRetrievalAddEntries: subfolder number of entries");
            ASSERT_UNEQUAL(entries.find("subFileA"), (entries.end()), "testLeafFolderRetrievalAddEntries: subFolder filename A");
            ASSERT_UNEQUAL(entries.find("subFileB"), (entries.end()), "testLeafFolderRetrievalAddEntries: subFolder filename B");
            ASSERT_UNEQUAL(entries.find("subFileC"), (entries.end()), "testLeafFolderRetrievalAddEntries: subFolder filename C");
            ASSERT_UNEQUAL(entries.find("subFileD"), (entries.end()), "testLeafFolderRetrievalAddEntries: subFolder filename D");
        }
    }

    void testLeafFolderRetrievalAddEntriesAppendData()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::LeafFolder folder = createTestFolder(testPath);
        teasafe::LeafFolder subFolder = *folder.getLeafFolder("folderA");
        subFolder.addFile("subFileA");
        subFolder.addFile("subFileB");
        subFolder.addFile("subFileC");
        subFolder.addFile("subFileD");

        std::string testData("some test data!");
        teasafe::File entry = *subFolder.getFile("subFileB", teasafe::OpenDisposition::buildAppendDisposition());
        std::vector<uint8_t> vec(testData.begin(), testData.end());
        entry.write((char*)&vec.front(), testData.length());
        entry.flush();
        entry.seek(0); // seek to start since retrieving from folder automatically seeks to end
        vec.resize(entry.fileSize());
        entry.read((char*)&vec.front(), entry.fileSize());
        std::string result(vec.begin(), vec.end());
        ASSERT_EQUAL(result, testData, "testLeafFolderRetrievalAddEntriesAppendData");
    }

    void testRemoveFile()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::LeafFolder folder = createTestFolder(testPath);
        folder.removeFile("test.txt");
        auto entries = folder.listAllEntries();
        ASSERT_EQUAL(entries.size(), 5, "testRemoveFile: number of entries after removal");
    }

    void testRemoveEmptySubFolder()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::LeafFolder folder = createTestFolder(testPath);
        folder.removeLeafFolder("folderA");
        auto entries = folder.listAllEntries();
        ASSERT_EQUAL(entries.size(), 5, "testRemoveEmptySubFolder: number of entries after removal");
    }

    void testRemoveNonEmptySubFolder()
    {
        {
            boost::filesystem::path testPath = buildImage(m_uniquePath);
            teasafe::LeafFolder folder = createTestFolder(testPath);
            teasafe::LeafFolder subFolder = *folder.getLeafFolder("folderA");
            subFolder.addFile("subFileA");
            subFolder.addFile("subFileB");
            subFolder.addFile("subFileC");
            subFolder.addFile("subFileD");

            std::string testData("some test data!");
            teasafe::File entry = *subFolder.getFile("subFileB", teasafe::OpenDisposition::buildAppendDisposition());
            std::vector<uint8_t> vec(testData.begin(), testData.end());
            entry.write((char*)&vec.front(), testData.length());
            entry.flush();
        }
        {
            boost::filesystem::path testPath = buildImage(m_uniquePath);
            teasafe::LeafFolder folder = createTestFolder(testPath);
            folder.removeLeafFolder("folderA");
            auto entries = folder.listAllEntries();
            ASSERT_EQUAL(entries.size(), 5, "testRemoveNonEmptySubFolder: number of entries after removal");
        }
    }

};
