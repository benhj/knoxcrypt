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

#include "teasafe/CompoundFolder.hpp"
#include "teasafe/CoreTeaSafeIO.hpp"
#include "teasafe/TeaSafe.hpp"
#include "test/SimpleTest.hpp"
#include "test/TestHelpers.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>

#include <sstream>

using namespace simpletest;

class TeaSafeTest
{
  public:
    TeaSafeTest() : m_uniquePath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
    {
        boost::filesystem::create_directories(m_uniquePath);
        // testFileExists();
        // testFolderExists();
        // testAddFile();
        // testAddCompoundFolder();
        // testAddFileThrowsIfParentNotFound();
        // testAddFolderThrowsIfParentNotFound();
        // testAddFileThrowsIfAlreadyExists();
        // testAddFolderThrowsIfAlreadyExists();
        // testRemoveFile();
        // testRemoveFileThrowsIfBadParent();
        // testRemoveFileThrowsIfNotFound();
        // //testRemoveFileThrowsIfFolder();
        // testRemoveEmptyFolder();
        // testRemoveFolderWithMustBeEmptyThrowsIfNonEmpty();
        // testRemoveNonEmptyFolder();
        // //testRemoveNonExistingFolderThrows();
        // testWriteToStream();
        // testListAllEntriesEmpty();
        // testMoveFileSameFolder();
        // testMoveFileToSubFolder();
        // testMoveFileFromSubFolderToParentFolder();
        // testThatDeletingEverythingDeallocatesEverything();
        testDebugging();
    }

    ~TeaSafeTest()
    {
        boost::filesystem::remove_all(m_uniquePath);
    }

    teasafe::CompoundFolder createTestFolder(boost::filesystem::path const &p)
    {
        teasafe::SharedCoreIO io = createTestIO(p);
        teasafe::CompoundFolder folder(io, 0, std::string("root"));
        // folder.addFile("test.txt");
        // folder.addFile("some.log");
        folder.addFolder("folderA");
        // folder.addFile("picture.jpg");
        // folder.addFile("vai.mp3");
        // folder.addFolder("folderB");

        teasafe::CompoundFolder folderA = *folder.getFolder("folderA");
        // folderA.addFile("fileA");
        // folderA.addFile("fileB");
        folderA.addFolder("subFolderA");

        teasafe::CompoundFolder subFolderA = *folderA.getFolder("subFolderA");
        //subFolderA.addFolder("subFolderB");
        subFolderA.addFile("fileX");
        // subFolderA.addFolder("subFolderC");
        // subFolderA.addFile("fileY");

        // teasafe::CompoundFolder subFolderC = *subFolderA.getFolder("subFolderC");
        // subFolderC.addFolder("finalFolder");
        // subFolderC.addFile("finalFile.txt");

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

        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);

        std::string testPositive("/folderA/subFolderA/subFolderC/finalFile.txt");
        std::string testNegative("/folderA/hello.log");

        ASSERT_EQUAL(true, theTeaSafe.fileExists(testPositive), "TeaSafeTest::testFileExists() positive case");
        ASSERT_EQUAL(false, theTeaSafe.fileExists(testNegative), "TeaSafeTest::testFileExists() negative case");
    }

    void testFolderExists()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        {
            (void)createTestFolder(testPath);
        }
        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);

        std::string testPositive("/folderA/subFolderA/");
        std::string testNegative("/folderA/subFolderA/subFolderX");

        ASSERT_EQUAL(true, theTeaSafe.folderExists(testPositive), "TeaSafeTest::testFolderExists() positive case");
        ASSERT_EQUAL(false, theTeaSafe.folderExists(testNegative), "TeaSafeTest::testFolderExists() negative case");
    }

    void testAddFile()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::CompoundFolder root = createTestFolder(testPath);

        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);

        theTeaSafe.addFile("/folderA/subFolderA/subFolderC/testAdded.txt");

        teasafe::CompoundFolder parent = *root.getFolder("folderA")->getFolder("subFolderA")->getFolder("subFolderC");

        teasafe::SharedEntryInfo entryInfo = parent.getEntryInfo("testAdded.txt");
        bool good = entryInfo ? true : false;
        ASSERT_EQUAL(true, good, "TeaSafeTest::testAddFile() getting info");
        ASSERT_EQUAL(teasafe::EntryType::FileType, entryInfo->type(), "TeaSafeTest::testAddFile() info type");
        ASSERT_EQUAL("testAdded.txt", entryInfo->filename(), "TeaSafeTest::testAddFile() info name");
    }

    void testAddCompoundFolder()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::CompoundFolder root = createTestFolder(testPath);

        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);

        theTeaSafe.addFolder("/folderA/subFolderA/subFolderC/testAdded");
        teasafe::CompoundFolder parent = *root.getFolder("folderA")->getFolder("subFolderA")->getFolder("subFolderC");
        teasafe::SharedEntryInfo entryInfo = parent.getEntryInfo("testAdded");
        bool good = entryInfo ? true : false;
        ASSERT_EQUAL(true, good, "TeaSafeTest::testAddCompoundFolder() getting info");
        ASSERT_EQUAL(teasafe::EntryType::FolderType, entryInfo->type(), "TeaSafeTest::testAddCompoundFolder() info type");
        ASSERT_EQUAL("testAdded", entryInfo->filename(), "TeaSafeTest::testAddCompoundFolder() info name");
    }

    void testAddFileThrowsIfParentNotFound()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::CompoundFolder root = createTestFolder(testPath);

        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);

        bool caught = false;
        try {
            theTeaSafe.addFile("/folderA/subFolderA/subFolderX/testAdded");
        } catch (teasafe::TeaSafeException const &e) {
            caught = true;
            ASSERT_EQUAL(teasafe::TeaSafeException(teasafe::TeaSafeError::NotFound), e,
                         "TeaSafeTest::testAddFileThrowIfParentNotFound() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "TeaSafeTest::testAddFileThrowIfParentNotFound() caught");
    }

    void testAddFolderThrowsIfParentNotFound()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::CompoundFolder root = createTestFolder(testPath);

        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);

        bool caught = false;
        try {
            theTeaSafe.addFolder("/folderA/subFolderQ/testAdded");
        } catch (teasafe::TeaSafeException const &e) {
            caught = true;
            ASSERT_EQUAL(teasafe::TeaSafeException(teasafe::TeaSafeError::NotFound), e,
                         "TeaSafeTest::testAddFolderThrowsIfParentNotFound() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "TeaSafeTest::testAddFolderThrowsIfParentNotFound() caught");
    }

    void testAddFileThrowsIfAlreadyExists()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::CompoundFolder root = createTestFolder(testPath);

        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);

        bool caught = false;
        try {
            theTeaSafe.addFile("/folderA/subFolderA/subFolderC/finalFile.txt");
        } catch (teasafe::TeaSafeException const &e) {
            caught = true;
            ASSERT_EQUAL(teasafe::TeaSafeException(teasafe::TeaSafeError::AlreadyExists), e,
                         "TeaSafeTest::testAddFileThrowsIfAlreadyExists() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "TeaSafeTest::testAddFileThrowsIfAlreadyExists() caught");
    }

    void testAddFolderThrowsIfAlreadyExists()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::CompoundFolder root = createTestFolder(testPath);

        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);

        bool caught = false;
        try {
            theTeaSafe.addFolder("/folderA/subFolderA/subFolderC/finalFile.txt");
        } catch (teasafe::TeaSafeException const &e) {
            caught = true;
            ASSERT_EQUAL(teasafe::TeaSafeException(teasafe::TeaSafeError::AlreadyExists), e,
                         "TeaSafeTest::testAddFolderThrowsIfAlreadyExists() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "TeaSafeTest::testAddFolderThrowsIfAlreadyExists() caught");
    }

    void testRemoveFile()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::CompoundFolder root = createTestFolder(testPath);
        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);
        theTeaSafe.removeFile("/folderA/subFolderA/subFolderC/finalFile.txt");
        teasafe::SharedEntryInfo info = root.getFolder("folderA")
            ->getFolder("subFolderA")
            ->getFolder("subFolderC")
            ->getEntryInfo("finalFile.txt");
        bool exists = info ? true : false;
        ASSERT_EQUAL(false, exists, "TeaSafeTest::testRemoveFile()");
    }

    void testRemoveFileThrowsIfBadParent()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::CompoundFolder root = createTestFolder(testPath);

        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);
        bool caught = false;
        try {
            theTeaSafe.removeFile("/folderA/subFolderA/subFolderX/finalFile.txt");
        } catch (teasafe::TeaSafeException const &e) {
            caught = true;
            ASSERT_EQUAL(teasafe::TeaSafeException(teasafe::TeaSafeError::NotFound), e,
                         "TeaSafeTest::testRemoveFileThrowsIfBadParent() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "TeaSafeTest::testRemoveFileThrowsIfBadParent() caught");
    }

    void testRemoveFileThrowsIfNotFound()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::CompoundFolder root = createTestFolder(testPath);

        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);
        bool caught = false;
        try {
            theTeaSafe.removeFile("/folderA/subFolderA/subFolderC/finalFileB.txt");
        } catch (teasafe::TeaSafeException const &e) {
            caught = true;
            ASSERT_EQUAL(teasafe::TeaSafeException(teasafe::TeaSafeError::NotFound), e,
                         "TeaSafeTest::testRemoveFileThrowsIfNotFound() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "TeaSafeTest::testRemoveFileThrowsIfNotFound() caught");
    }

    void testRemoveFileThrowsIfFolder()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::CompoundFolder root = createTestFolder(testPath);

        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);
        bool caught = false;
        try {
            theTeaSafe.removeFile("/folderA/subFolderA/subFolderC/finalFolder");
        } catch (teasafe::TeaSafeException const &e) {
            caught = true;
            ASSERT_EQUAL(teasafe::TeaSafeException(teasafe::TeaSafeError::NotFound), e,
                         "TeaSafeTest::testRemoveFileThrowsIfFolder() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "TeaSafeTest::testRemoveFileThrowsIfFolder() caught");
    }

    void testRemoveEmptyFolder()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::CompoundFolder root = createTestFolder(testPath);
        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);
        theTeaSafe.removeFolder("/folderA/subFolderA/subFolderC/finalFolder",
                                teasafe::FolderRemovalType::MustBeEmpty);


        teasafe::SharedEntryInfo info = root.getFolder("folderA")
            ->getFolder("subFolderA")
            ->getFolder("subFolderC")
            ->getEntryInfo("finalFolder");
        bool exists = info ? true : false;
        ASSERT_EQUAL(false, exists, "TeaSafeTest::testRemoveEmptyFolder()");
    }

    void testRemoveFolderWithMustBeEmptyThrowsIfNonEmpty()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::CompoundFolder root = createTestFolder(testPath);
        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);
        bool caught = false;
        try {
            theTeaSafe.removeFolder("/folderA/subFolderA/",
                                    teasafe::FolderRemovalType::MustBeEmpty);
        } catch (teasafe::TeaSafeException const &e) {
            caught = true;
            ASSERT_EQUAL(teasafe::TeaSafeException(teasafe::TeaSafeError::FolderNotEmpty), e,
                         "TeaSafeTest::testRemoveFolderWithMustBeEmptyThrowsIfNonEmpty() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "TeaSafeTest::testRemoveFolderWithMustBeEmptyThrowsIfNonEmpty() caught");
    }

    void testRemoveNonEmptyFolder()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::CompoundFolder root = createTestFolder(testPath);
        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);
        theTeaSafe.removeFolder("/folderA/subFolderA/",
                                teasafe::FolderRemovalType::Recursive);
        teasafe::SharedEntryInfo info = root.getFolder("folderA")->getEntryInfo("subFolderA");
        bool exists = info ? true : false;
        ASSERT_EQUAL(false, exists, "TeaSafeTest::testRemoveNonEmptyFolder()");
    }

    void testRemoveNonExistingFolderThrows()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::CompoundFolder root = createTestFolder(testPath);
        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);
        bool caught = false;
        try {
            theTeaSafe.removeFolder("/folderA/subFolderQ/",
                                    teasafe::FolderRemovalType::MustBeEmpty);
        } catch (teasafe::TeaSafeException const &e) {
            caught = true;
            ASSERT_EQUAL(teasafe::TeaSafeException(teasafe::TeaSafeError::NotFound), e,
                         "TeaSafeTest::testRemoveNonExistingFolderThrows() asserting error type");
        }
        ASSERT_EQUAL(true, caught, "TeaSafeTest::testRemoveNonExistingFolderThrows() caught");
    }

    void testWriteToStream()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::CompoundFolder root = createTestFolder(testPath);
        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);

        // open file and append to end of it
        std::string const &testString(createLargeStringToWrite());

        teasafe::FileDevice device = theTeaSafe.openFile("/folderA/subFolderA/fileX",
                                                                teasafe::OpenDisposition::buildAppendDisposition());

        std::streampos wrote = device.write(testString.c_str(), testString.length());
        ASSERT_EQUAL(static_cast<size_t>(wrote), testString.length(), "TeaSafeTest::testWriteToStream() bytesWrote");
        (void)device.seek(0, std::ios_base::beg);

        // check content
        std::vector<uint8_t> buffer;
        buffer.resize(testString.length());
        std::streampos bytesRead = device.read((char*)&buffer.front(), testString.length());
        ASSERT_EQUAL(testString.length(), static_cast<size_t>(bytesRead), "TeaSafeTest::testWriteToStream() bytesRead");
        std::string recovered(buffer.begin(), buffer.end());
        ASSERT_EQUAL(testString, recovered, "TeaSafeTest::testWriteToStream() content");
    }

    void testListAllEntriesEmpty()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);
        teasafe::CompoundFolder fe = theTeaSafe.getFolder("/");
        auto infos = fe.listAllEntries();
        ASSERT_EQUAL(infos.empty(), true, "TeaSafeTest::testListAllEntriesEmpty()");
    }

    void testMoveFileSameFolder()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        (void)createTestFolder(testPath);
        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);
        theTeaSafe.renameEntry("/folderA/fileA", "/folderA/renamed.txt");
        ASSERT_EQUAL(false, theTeaSafe.fileExists("/folderA/fileA"), "TeaSafeTest::testMoveFileSameFolder() original removed");
        ASSERT_EQUAL(true, theTeaSafe.fileExists("/folderA/renamed.txt"), "TeaSafeTest::testMoveFileSameFolder() new version");
    }

    void testMoveFileToSubFolder()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        (void)createTestFolder(testPath);
        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);
        theTeaSafe.renameEntry("/folderA/fileA", "/folderA/subFolderA/renamed.txt");
        ASSERT_EQUAL(false, theTeaSafe.fileExists("/folderA/fileA"), "TeaSafeTest::testMoveFileToSubFolderFolder() original removed");
        ASSERT_EQUAL(true, theTeaSafe.fileExists("/folderA/subFolderA/renamed.txt"), "TeaSafeTest::testMoveFileToSubFolderFolder() new version");
    }

    void testMoveFileFromSubFolderToParentFolder()
    {
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        (void)createTestFolder(testPath);
        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);
        theTeaSafe.renameEntry("/folderA/subFolderA/fileX", "/folderA/renamed.txt");
        ASSERT_EQUAL(false, theTeaSafe.fileExists("/folderA/subFolderA/fileX"), "TeaSafeTest::testMoveFileToSubFolderFolder() original removed");
        ASSERT_EQUAL(true, theTeaSafe.fileExists("/folderA/renamed.txt"), "TeaSafeTest::testMoveFileToSubFolderFolder() new version");
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
        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);

        // open file and append to end of it
        std::string const &testString(createLargeStringToWrite());
        {
            teasafe::FileDevice device = theTeaSafe.openFile("/folderA/subFolderA/fileX",
                                                                    teasafe::OpenDisposition::buildAppendDisposition());
            (void)device.write(testString.c_str(), testString.length());
        }

        // see what blocks are in use
        std::vector<long> blocksInUse;
        {
            teasafe::ContainerImageStream in(io, std::ios::in | std::ios::out | std::ios::binary);
            for (long i = 0; i < blocks; ++i) {

                if (teasafe::detail::isBlockInUse(i, blocks, in)) {
                    std::cout<<"before i: "<<i<<std::endl;
                    blocksInUse.push_back(i);
                }
            }
        }

        // now remove all content
        //theTeaSafe.removeFile("/test.txt");
        //theTeaSafe.removeFile("/some.log");
        theTeaSafe.removeFolder("/folderA", teasafe::FolderRemovalType::Recursive);
        //theTeaSafe.removeFile("/picture.jpg");
        //theTeaSafe.removeFile("/vai.mp3");
        //theTeaSafe.removeFolder("/folderB", teasafe::FolderRemovalType::Recursive);

        // check that all blocks except 0 which is the root folder entry point
        // have been deallocated
        {
            bool blockCheckPassed = true;
            teasafe::ContainerImageStream in(io, std::ios::in | std::ios::out | std::ios::binary);
            for (int i = 1; i<blocks; ++i) {

                if (teasafe::detail::isBlockInUse(i, blocks, in)) {
                    
                    blockCheckPassed = false;
                    break;
                }
            }
            ASSERT_EQUAL(true, blockCheckPassed,
                         "TeaSafeTest::testThatDeletingEverythingDeallocatesEverything() blocks dealloc'd");
        }

        // now re-add content and check that allocated blocks are same as previous allocation
        {
            (void)createTestFolder(testPath);
        }
        {
            teasafe::FileDevice device = theTeaSafe.openFile("/folderA/subFolderA/fileX",
                                                                    teasafe::OpenDisposition::buildAppendDisposition());
            (void)device.write(testString.c_str(), testString.length());
        }

        // see what blocks are in use this time around; should be same as first time around
        std::vector<long> blocksInUseB;
        {
            teasafe::ContainerImageStream in(io, std::ios::in | std::ios::out | std::ios::binary);
            for (long i = 0; i < blocks; ++i) {

                if (teasafe::detail::isBlockInUse(i, blocks, in)) {
                    std::cout<<"after i: "<<i<<std::endl;
                    blocksInUseB.push_back(i);
                }
            }
        }

        ASSERT_EQUAL(blocksInUse.size(), blocksInUseB.size(),
                     "TeaSafeTest::testThatDeletingEverythingDeallocatesEverything() same block counts");

        std::cout<<blocksInUse.size()<<"\t"<<blocksInUseB.size()<<std::endl;

        bool sameBlocks = true;
        for (size_t i = 0; i < blocksInUse.size(); ++i) {
            if (blocksInUse[i] != blocksInUseB[i]) {
                sameBlocks = false;
                break;
            }
        }

        ASSERT_EQUAL(true, sameBlocks,
                     "TeaSafeTest::testThatDeletingEverythingDeallocatesEverything() same blocks");

    }

    // in the context of debugging on branch debuggingSeek
    void testDebugging()
    {
        long const blocks = 2048;
        boost::filesystem::path testPath = buildImage(m_uniquePath);
        {
            (void)createTestFolder(testPath);
        }
        teasafe::SharedCoreIO io(createTestIO(testPath));
        teasafe::TeaSafe theTeaSafe(io);
        theTeaSafe.removeFolder("/folderA", teasafe::FolderRemovalType::Recursive);
        teasafe::CompoundFolder folder(io, 0, std::string("root"));
        folder.addFolder("/folderA");
        auto folderA = *folder.getFolder("/folderA");
        folderA.addFolder("subFolderA");
    }
};
