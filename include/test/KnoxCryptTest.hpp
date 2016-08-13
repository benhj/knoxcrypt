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

#include "knoxcrypt/CompoundFolder.hpp"
#include "knoxcrypt/CoreknoxcryptIO.hpp"
#include "knoxcrypt/knoxCrypt.hpp"
#include "test/SimpleTest.hpp"
#include "test/TestHelpers.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>

#include <sstream>

using namespace simpletest;

class KnoxCryptTest
{
  public:
    KnoxCryptTest() : m_uniquePath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
    {
        boost::filesystem::create_directories(m_uniquePath);
        testFileExists();
        testFolderExists();
        testAddFile();
        testAddCompoundFolder();
        testAddFileThrowsIfParentNotFound();
        testAddFolderThrowsIfParentNotFound();
        testAddFileThrowsIfAlreadyExists();
        testAddFolderThrowsIfAlreadyExists();
        testRemoveFile();
        testRemoveFileThrowsIfBadParent();
        testRemoveFileThrowsIfNotFound();
        // //testRemoveFileThrowsIfFolder();
        testRemoveEmptyFolder();
        testRemoveFolderWithMustBeEmptyThrowsIfNonEmpty();
        testRemoveNonEmptyFolder();
        //testRemoveNonExistingFolderThrows();
        testWriteToStream();
        testListAllEntriesEmpty();
        testMoveFileSameFolder();
        testMoveFileToSubFolder();
        testMoveFileFromSubFolderToParentFolder();
        testThatDeletingEverythingDeallocatesEverything();
        //testDebugging();
    }

    ~KnoxCryptTest()
    {
        boost::filesystem::remove_all(m_uniquePath);
    }

    knoxcrypt::CompoundFolder createTestFolder(boost::filesystem::path const &p)
    {
        knoxcrypt::SharedCoreIO io = createTestIO(p);
        knoxcrypt::CompoundFolder folder(io, 0, std::string("root"));
        folder.addFile("test.txt");
        folder.addFile("some.log");
        folder.addFolder("folderA");
        folder.addFile("picture.jpg");
        folder.addFile("vai.mp3");
        folder.addFolder("folderB");

        knoxcrypt::CompoundFolder folderA = *folder.getFolder("folderA");
        folderA.addFile("fileA");
        folderA.addFile("fileB");
        folderA.addFolder("subFolderA");

        knoxcrypt::CompoundFolder subFolderA = *folderA.getFolder("subFolderA");
        subFolderA.addFolder("subFolderB");
        subFolderA.addFile("fileX");
        subFolderA.addFolder("subFolderC");
        subFolderA.addFile("fileY");

        knoxcrypt::CompoundFolder subFolderC = *subFolderA.getFolder("subFolderC");
        subFolderC.addFolder("finalFolder");
        subFolderC.addFile("finalFile.txt");

        return folder;
    }

  private:
    boost::filesystem::path m_uniquePath;

    void testFileExists()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);

        {
            (void)createTestFolder(testPath);
        }

        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);

        std::string testPositive("/folderA/subFolderA/subFolderC/finalFile.txt");
        std::string testNegative("/folderA/hello.log");

        ASSERT_EQUAL(true, kc.fileExists(testPositive), "KnoxCryptTest::testFileExists() positive case");
        ASSERT_EQUAL(false, kc.fileExists(testNegative), "KnoxCryptTest::testFileExists() negative case");
    }

    void testFolderExists()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        {
            (void)createTestFolder(testPath);
        }
        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);

        std::string testPositive("/folderA/subFolderA/");
        std::string testNegative("/folderA/subFolderA/subFolderX");

        ASSERT_EQUAL(true, kc.folderExists(testPositive), "KnoxCryptTest::testFolderExists() positive case");
        ASSERT_EQUAL(false, kc.folderExists(testNegative), "KnoxCryptTest::testFolderExists() negative case");
    }

    void testAddFile()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        knoxcrypt::CompoundFolder root = createTestFolder(testPath);

        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);

        kc.addFile("/folderA/subFolderA/subFolderC/testAdded.txt");

        knoxcrypt::CompoundFolder parent = *root.getFolder("folderA")->getFolder("subFolderA")->getFolder("subFolderC");

        knoxcrypt::SharedEntryInfo entryInfo = parent.getEntryInfo("testAdded.txt");
        bool good = entryInfo ? true : false;
        ASSERT_EQUAL(true, good, "KnoxCryptTest::testAddFile() getting info");
        ASSERT_EQUAL(knoxcrypt::EntryType::FileType, entryInfo->type(), "KnoxCryptTest::testAddFile() info type");
        ASSERT_EQUAL("testAdded.txt", entryInfo->filename(), "KnoxCryptTest::testAddFile() info name");
    }

    void testAddCompoundFolder()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        knoxcrypt::CompoundFolder root = createTestFolder(testPath);

        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);

        kc.addFolder("/folderA/subFolderA/subFolderC/testAdded");
        knoxcrypt::CompoundFolder parent = *root.getFolder("folderA")->getFolder("subFolderA")->getFolder("subFolderC");
        knoxcrypt::SharedEntryInfo entryInfo = parent.getEntryInfo("testAdded");
        bool good = entryInfo ? true : false;
        ASSERT_EQUAL(true, good, "KnoxCryptTest::testAddCompoundFolder() getting info");
        ASSERT_EQUAL(knoxcrypt::EntryType::FolderType, entryInfo->type(), "KnoxCryptTest::testAddCompoundFolder() info type");
        ASSERT_EQUAL("testAdded", entryInfo->filename(), "KnoxCryptTest::testAddCompoundFolder() info name");
    }

    void testAddFileThrowsIfParentNotFound()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        knoxcrypt::CompoundFolder root = createTestFolder(testPath);

        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);

        bool caught = false;
        try {
            kc.addFile("/folderA/subFolderA/subFolderX/testAdded");
        } catch (knoxcrypt::KnoxCryptException const &e) {
            caught = true;
            ASSERT_EQUAL(knoxcrypt::KnoxCryptException(knoxcrypt::KnoxCryptError::NotFound), e,
                         "KnoxCryptTest::testAddFileThrowIfParentNotFound() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "KnoxCryptTest::testAddFileThrowIfParentNotFound() caught");
    }

    void testAddFolderThrowsIfParentNotFound()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        knoxcrypt::CompoundFolder root = createTestFolder(testPath);

        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);

        bool caught = false;
        try {
            kc.addFolder("/folderA/subFolderQ/testAdded");
        } catch (knoxcrypt::KnoxCryptException const &e) {
            caught = true;
            ASSERT_EQUAL(knoxcrypt::KnoxCryptException(knoxcrypt::KnoxCryptError::NotFound), e,
                         "KnoxCryptTest::testAddFolderThrowsIfParentNotFound() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "KnoxCryptTest::testAddFolderThrowsIfParentNotFound() caught");
    }

    void testAddFileThrowsIfAlreadyExists()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        knoxcrypt::CompoundFolder root = createTestFolder(testPath);

        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);

        bool caught = false;
        try {
            kc.addFile("/folderA/subFolderA/subFolderC/finalFile.txt");
        } catch (knoxcrypt::KnoxCryptException const &e) {
            caught = true;
            ASSERT_EQUAL(knoxcrypt::KnoxCryptException(knoxcrypt::KnoxCryptError::AlreadyExists), e,
                         "KnoxCryptTest::testAddFileThrowsIfAlreadyExists() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "KnoxCryptTest::testAddFileThrowsIfAlreadyExists() caught");
    }

    void testAddFolderThrowsIfAlreadyExists()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        knoxcrypt::CompoundFolder root = createTestFolder(testPath);

        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);

        bool caught = false;
        try {
            kc.addFolder("/folderA/subFolderA/subFolderC/finalFile.txt");
        } catch (knoxcrypt::KnoxCryptException const &e) {
            caught = true;
            ASSERT_EQUAL(knoxcrypt::KnoxCryptException(knoxcrypt::KnoxCryptError::AlreadyExists), e,
                         "KnoxCryptTest::testAddFolderThrowsIfAlreadyExists() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "KnoxCryptTest::testAddFolderThrowsIfAlreadyExists() caught");
    }

    void testRemoveFile()
    {

        boost::filesystem::path testPath = buildImage(m_uniquePath);
        knoxcrypt::CompoundFolder root = createTestFolder(testPath);
        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);
        kc.removeFile("/folderA/subFolderA/subFolderC/finalFile.txt");
        knoxcrypt::SharedEntryInfo info = root.getFolder("folderA")
            ->getFolder("subFolderA")
            ->getFolder("subFolderC")
            ->getEntryInfo("finalFile.txt");
        bool exists = info ? true : false;
        ASSERT_EQUAL(false, exists, "KnoxCryptTest::testRemoveFile()");
    }

    void testRemoveFileThrowsIfBadParent()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        knoxcrypt::CompoundFolder root = createTestFolder(testPath);

        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);
        bool caught = false;
        try {
            kc.removeFile("/folderA/subFolderA/subFolderX/finalFile.txt");
        } catch (knoxcrypt::KnoxCryptException const &e) {
            caught = true;
            ASSERT_EQUAL(knoxcrypt::KnoxCryptException(knoxcrypt::KnoxCryptError::NotFound), e,
                         "KnoxCryptTest::testRemoveFileThrowsIfBadParent() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "KnoxCryptTest::testRemoveFileThrowsIfBadParent() caught");
    }

    void testRemoveFileThrowsIfNotFound()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        knoxcrypt::CompoundFolder root = createTestFolder(testPath);

        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);
        bool caught = false;
        try {
            kc.removeFile("/folderA/subFolderA/subFolderC/finalFileB.txt");
        } catch (knoxcrypt::KnoxCryptException const &e) {
            caught = true;
            ASSERT_EQUAL(knoxcrypt::KnoxCryptException(knoxcrypt::KnoxCryptError::NotFound), e,
                         "KnoxCryptTest::testRemoveFileThrowsIfNotFound() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "KnoxCryptTest::testRemoveFileThrowsIfNotFound() caught");
    }

    void testRemoveFileThrowsIfFolder()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        knoxcrypt::CompoundFolder root = createTestFolder(testPath);

        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);
        bool caught = false;
        try {
            kc.removeFile("/folderA/subFolderA/subFolderC/finalFolder");
        } catch (knoxcrypt::KnoxCryptException const &e) {
            caught = true;
            ASSERT_EQUAL(knoxcrypt::KnoxCryptException(knoxcrypt::KnoxCryptError::NotFound), e,
                         "KnoxCryptTest::testRemoveFileThrowsIfFolder() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "KnoxCryptTest::testRemoveFileThrowsIfFolder() caught");
    }

    void testRemoveEmptyFolder()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        knoxcrypt::CompoundFolder root = createTestFolder(testPath);
        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);
        kc.removeFolder("/folderA/subFolderA/subFolderC/finalFolder",
                                knoxcrypt::FolderRemovalType::MustBeEmpty);


        knoxcrypt::SharedEntryInfo info = root.getFolder("folderA")
            ->getFolder("subFolderA")
            ->getFolder("subFolderC")
            ->getEntryInfo("finalFolder");
        bool exists = info ? true : false;
        ASSERT_EQUAL(false, exists, "KnoxCryptTest::testRemoveEmptyFolder()");
    }

    void testRemoveFolderWithMustBeEmptyThrowsIfNonEmpty()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        knoxcrypt::CompoundFolder root = createTestFolder(testPath);
        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);
        bool caught = false;
        try {
            kc.removeFolder("/folderA/subFolderA/",
                                    knoxcrypt::FolderRemovalType::MustBeEmpty);
        } catch (knoxcrypt::KnoxCryptException const &e) {
            caught = true;
            ASSERT_EQUAL(knoxcrypt::KnoxCryptException(knoxcrypt::KnoxCryptError::FolderNotEmpty), e,
                         "KnoxCryptTest::testRemoveFolderWithMustBeEmptyThrowsIfNonEmpty() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "KnoxCryptTest::testRemoveFolderWithMustBeEmptyThrowsIfNonEmpty() caught");
    }

    void testRemoveNonEmptyFolder()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        knoxcrypt::CompoundFolder root = createTestFolder(testPath);
        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);
        kc.removeFolder("/folderA/subFolderA/",
                                knoxcrypt::FolderRemovalType::Recursive);
        knoxcrypt::SharedEntryInfo info = root.getFolder("folderA")->getEntryInfo("subFolderA");
        bool exists = info ? true : false;
        ASSERT_EQUAL(false, exists, "KnoxCryptTest::testRemoveNonEmptyFolder()");
    }

    void testRemoveNonExistingFolderThrows()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        knoxcrypt::CompoundFolder root = createTestFolder(testPath);
        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);
        bool caught = false;
        try {
            kc.removeFolder("/folderA/subFolderQ/",
                                    knoxcrypt::FolderRemovalType::MustBeEmpty);
        } catch (knoxcrypt::KnoxCryptException const &e) {
            caught = true;
            ASSERT_EQUAL(knoxcrypt::KnoxCryptException(knoxcrypt::KnoxCryptError::NotFound), e,
                         "KnoxCryptTest::testRemoveNonExistingFolderThrows() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "KnoxCryptTest::testRemoveNonExistingFolderThrows() caught");
    }

    void testWriteToStream()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        knoxcrypt::CompoundFolder root = createTestFolder(testPath);
        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);

        // open file and append to end of it
        std::string const &testString(createLargeStringToWrite());

        knoxcrypt::FileDevice device = kc.openFile("/folderA/subFolderA/fileX",
                                                                knoxcrypt::OpenDisposition::buildAppendDisposition());

        std::streampos wrote = device.write(testString.c_str(), testString.length());
        ASSERT_EQUAL(static_cast<size_t>(wrote), testString.length(), "KnoxCryptTest::testWriteToStream() bytesWrote");
        (void)device.seek(0, std::ios_base::beg);

        // check content
        std::vector<uint8_t> buffer;
        buffer.resize(testString.length());
        std::streampos bytesRead = device.read((char*)&buffer.front(), testString.length());
        ASSERT_EQUAL(testString.length(), static_cast<size_t>(bytesRead), "KnoxCryptTest::testWriteToStream() bytesRead");
        std::string recovered(buffer.begin(), buffer.end());
        ASSERT_EQUAL(testString, recovered, "KnoxCryptTest::testWriteToStream() content");
    }

    void testListAllEntriesEmpty()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);
        knoxcrypt::CompoundFolder fe = kc.getFolder("/");
        auto infos = fe.listAllEntries();
        ASSERT_EQUAL(infos.empty(), true, "KnoxCryptTest::testListAllEntriesEmpty()");
    }

    void testMoveFileSameFolder()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        (void)createTestFolder(testPath);
        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);
        kc.renameEntry("/folderA/fileA", "/folderA/renamed.txt");
        ASSERT_EQUAL(false, kc.fileExists("/folderA/fileA"), "KnoxCryptTest::testMoveFileSameFolder() original removed");
        ASSERT_EQUAL(true, kc.fileExists("/folderA/renamed.txt"), "KnoxCryptTest::testMoveFileSameFolder() new version");
    }

    void testMoveFileToSubFolder()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        (void)createTestFolder(testPath);
        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);
        kc.renameEntry("/folderA/fileA", "/folderA/subFolderA/renamed.txt");
        ASSERT_EQUAL(false, kc.fileExists("/folderA/fileA"), "KnoxCryptTest::testMoveFileToSubFolderFolder() original removed");
        ASSERT_EQUAL(true, kc.fileExists("/folderA/subFolderA/renamed.txt"), "KnoxCryptTest::testMoveFileToSubFolderFolder() new version");
    }

    void testMoveFileFromSubFolderToParentFolder()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        (void)createTestFolder(testPath);
        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);
        kc.renameEntry("/folderA/subFolderA/fileX", "/folderA/renamed.txt");
        ASSERT_EQUAL(false, kc.fileExists("/folderA/subFolderA/fileX"), "KnoxCryptTest::testMoveFileToSubFolderFolder() original removed");
        ASSERT_EQUAL(true, kc.fileExists("/folderA/renamed.txt"), "KnoxCryptTest::testMoveFileToSubFolderFolder() new version");
    }

    // checks that exactly the same blocks are allocated for content that is removed
    // and then re-added
    void testThatDeletingEverythingDeallocatesEverything()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        {
            (void)createTestFolder(testPath);
        }
        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);

        // open file and append to end of it
        std::string const &testString(createLargeStringToWrite());
        {
            knoxcrypt::FileDevice device = kc.openFile("/folderA/subFolderA/fileX",
                                                                    knoxcrypt::OpenDisposition::buildAppendDisposition());
            (void)device.write(testString.c_str(), testString.length());
        }

        // see what blocks are in use
        std::vector<long> blocksInUse;
        {
            knoxcrypt::ContainerImageStream in(io, std::ios::in | std::ios::out | std::ios::binary);
            for (long i = 0; i < blocks; ++i) {

                if (knoxcrypt::detail::isBlockInUse(i, blocks, in)) {
                    blocksInUse.push_back(i);
                }
            }
        }

        // now remove all content
        kc.removeFile("/test.txt");
        kc.removeFile("/some.log");
        kc.removeFolder("/folderA", knoxcrypt::FolderRemovalType::Recursive);
        kc.removeFile("/picture.jpg");
        kc.removeFile("/vai.mp3");
        kc.removeFolder("/folderB", knoxcrypt::FolderRemovalType::Recursive);

        // check that all blocks except 0 which is the root folder entry point
        // have been deallocated
        {
            bool blockCheckPassed = true;
            knoxcrypt::ContainerImageStream in(io, std::ios::in | std::ios::out | std::ios::binary);
            for (int i = 1; i<blocks; ++i) {

                if (knoxcrypt::detail::isBlockInUse(i, blocks, in)) {

                    blockCheckPassed = false;
                    break;
                }
            }
            ASSERT_EQUAL(true, blockCheckPassed,
                         "KnoxCryptTest::testThatDeletingEverythingDeallocatesEverything() blocks dealloc'd");
        }

        // now re-add content and check that allocated blocks are same as previous allocation
        {
            (void)createTestFolder(testPath);
        }
        {
            knoxcrypt::FileDevice device = kc.openFile("/folderA/subFolderA/fileX",
                                                                    knoxcrypt::OpenDisposition::buildAppendDisposition());
            (void)device.write(testString.c_str(), testString.length());
        }

        // see what blocks are in use this time around; should be same as first time around
        std::vector<long> blocksInUseB;
        {
            knoxcrypt::ContainerImageStream in(io, std::ios::in | std::ios::out | std::ios::binary);
            for (long i = 0; i < blocks; ++i) {

                if (knoxcrypt::detail::isBlockInUse(i, blocks, in)) {
                    blocksInUseB.push_back(i);
                }
            }
        }

        ASSERT_EQUAL(blocksInUse.size(), blocksInUseB.size(),
                     "KnoxCryptTest::testThatDeletingEverythingDeallocatesEverything() same block counts");

        bool sameBlocks = true;
        for (size_t i = 0; i < blocksInUse.size(); ++i) {
            if (blocksInUse[i] != blocksInUseB[i]) {
                sameBlocks = false;
                break;
            }
        }

        ASSERT_EQUAL(true, sameBlocks,
                     "KnoxCryptTest::testThatDeletingEverythingDeallocatesEverything() same blocks");

    }

    // in the context of debugging on branch debuggingSeek
    void testDebugging()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        {
            (void)createTestFolder(testPath);
        }
        knoxcrypt::SharedCoreIO io(createTestIO(testPath));
        knoxcrypt::KnoxCrypt kc(io);
        kc.removeFolder("/folderA", knoxcrypt::FolderRemovalType::Recursive);
        knoxcrypt::CompoundFolder folder(io, 0, std::string("root"));
        folder.addFolder("/folderA");
        auto folderA = *folder.getFolder("/folderA");
        folderA.addFolder("subFolderA");
    }
};
