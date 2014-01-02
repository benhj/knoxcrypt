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

#ifndef BFS_BFS_HPP__
#define BFS_BFS_HPP__

#include "bfs/BFSException.hpp"
#include "bfs/CoreBFSIO.hpp"
#include "bfs/EntryType.hpp"
#include "bfs/FileEntryDevice.hpp"
#include "bfs/FolderEntry.hpp"
#include "bfs/FolderRemovalType.hpp"
#include "bfs/OpenDisposition.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>
#include <boost/thread/lock_guard.hpp>

#include <string>

namespace bfs
{
    class BFS
    {

        typedef boost::optional<FolderEntry> OptionalFolderEntry;

      public:
        explicit BFS(CoreBFSIO const &io)
            : m_io(io)
            , m_accessMutex()
            , m_rootFolder(m_io, 0, "root")
        {

        }

        FolderEntry getCurrent(std::string const &path)
        {
            FolderEntry rootFolder(m_io, 0, "root");
            std::string thePath(path);
            char ch = *path.rbegin();
            // ignore trailing slash, but only if folder type
            // an entry of file type should never have a trailing
            // slash and is allowed to fail in this case
            if (ch == '/') {
                std::string(path.begin(), path.end() - 1).swap(thePath);
            }

            OptionalFolderEntry parentEntry = doGetParentFolderEntry(thePath, rootFolder);
            if (!parentEntry) {
                return rootFolder;
            }

            OptionalEntryInfo childInfo = parentEntry->getEntryInfo(boost::filesystem::path(path).filename().string());
            if (!childInfo) {
                throw BFSException(BFSError::NotFound);
            }
            if (childInfo->type() == EntryType::FileType) {
                throw BFSException(BFSError::NotFound);
            }
            return parentEntry->getFolderEntry(boost::filesystem::path(path).filename().string());


        }

        EntryInfo getInfo(std::string const &path)
        {
            FolderEntry rootFolder(m_io, 0, "root");
            std::string thePath(path);
            char ch = *path.rbegin();
            // ignore trailing slash, but only if folder type
            // an entry of file type should never have a trailing
            // slash and is allowed to fail in this case
            if (ch == '/') {
                std::string(path.begin(), path.end() - 1).swap(thePath);
            }

            OptionalFolderEntry parentEntry = doGetParentFolderEntry(thePath, rootFolder);
            if (!parentEntry) {
                throw BFSException(BFSError::NotFound);
            }

            std::string fname = boost::filesystem::path(thePath).filename().string();
            OptionalEntryInfo childInfo = parentEntry->getEntryInfo(boost::filesystem::path(thePath).filename().string());

            if (!childInfo) {
                throw BFSException(BFSError::NotFound);
            }
            return *childInfo;
        }


        bool fileExists(std::string const &path) const
        {
            FolderEntry rootFolder(m_io, 0, "root");
            return doFileExists(path, rootFolder);
        }

        bool folderExists(std::string const &path)
        {
            FolderEntry rootFolder(m_io, 0, "root");
            return doFolderExists(path, rootFolder);
        }

        void addFile(std::string const &path)
        {
            FolderEntry rootFolder(m_io, 0, "root");
            std::string thePath(path);
            char ch = *path.rbegin();
            // file entries with trailing slash should throw
            if (ch == '/') {
                throw BFSException(BFSError::IllegalFilename);
            }

            OptionalFolderEntry parentEntry = doGetParentFolderEntry(thePath, rootFolder);
            if (!parentEntry) {
                throw BFSException(BFSError::NotFound);
            }

            // throw if already exists
            throwIfAlreadyExists(path, rootFolder);

            parentEntry->addFileEntry(boost::filesystem::path(thePath).filename().string());

        }

        void renameEntry(std::string const &src, std::string const &dst)
        {
            std::string srcPath(src);
            char ch = *src.rbegin();
            // ignore trailing slash
            if (ch == '/') {
                std::string(src.begin(), src.end() - 1).swap(srcPath);
            }
            std::string dstPath(dst);
            char chDst = *dst.rbegin();
            // ignore trailing slash
            if (chDst == '/') {
                std::string(dst.begin(), dst.end() - 1).swap(dstPath);
            }
            FolderEntry rootFolder(m_io, 0, "root");

            // throw if source parent doesn't exist
            OptionalFolderEntry parentSrc = doGetParentFolderEntry(srcPath, rootFolder);
            if (!parentSrc) {
                throw BFSException(BFSError::NotFound);
            }

            // throw if destination parent doesn't exist
            OptionalFolderEntry parentDst = doGetParentFolderEntry(dstPath, rootFolder);
            if (!parentSrc) {
                throw BFSException(BFSError::NotFound);
            }

            // throw if destination already exists
            throwIfAlreadyExists(dstPath, rootFolder);

            // throw if source doesn't exist
            std::string const filename = boost::filesystem::path(srcPath).filename().string();
            OptionalEntryInfo childInfo = parentSrc->getEntryInfo(filename);
            if (!childInfo) {
                throw BFSException(BFSError::NotFound);
            }

            // do moving / renaming
            // (i) Remove original entry metadata entry
            // (ii) Add new metadata entry with new file name
            parentSrc->putMetaDataOutOfUse(filename);
            std::string dstFilename = boost::filesystem::path(dstPath).filename().string();
            parentDst->writeNewMetaDataForEntry(dstFilename, childInfo->type(), childInfo->firstFileBlock());

        }

        void addFolder(std::string const &path) const
        {
            FolderEntry rootFolder(m_io, 0, "root");
            std::string thePath(path);
            char ch = *path.rbegin();
            // ignore trailing slash
            if (ch == '/') {
                std::string(path.begin(), path.end() - 1).swap(thePath);
            }

            OptionalFolderEntry parentEntry = doGetParentFolderEntry(thePath, rootFolder);
            if (!parentEntry) {
                throw BFSException(BFSError::NotFound);
            }

            // throw if already exists
            throwIfAlreadyExists(path, rootFolder);

            parentEntry->addFolderEntry(boost::filesystem::path(thePath).filename().string());
        }

        void removeFile(std::string const &path)
        {
            FolderEntry rootFolder(m_io, 0, "root");
            std::string thePath(path);
            char ch = *path.rbegin();
            // ignore trailing slash, but only if folder type
            // an entry of file type should never have a trailing
            // slash and is allowed to fail in this case
            if (ch == '/') {
                std::string(path.begin(), path.end() - 1).swap(thePath);
            }

            OptionalFolderEntry parentEntry = doGetParentFolderEntry(thePath, rootFolder);
            if (!parentEntry) {
                throw BFSException(BFSError::NotFound);
            }

            OptionalEntryInfo childInfo = parentEntry->getEntryInfo(boost::filesystem::path(thePath).filename().string());
            if (!childInfo) {
                throw BFSException(BFSError::NotFound);
            }
            if (childInfo->type() == EntryType::FolderType) {
                throw BFSException(BFSError::NotFound);
            }

            parentEntry->removeFileEntry(boost::filesystem::path(thePath).filename().string());
        }

        void removeFolder(std::string const &path, FolderRemovalType const &removalType)
        {
            FolderEntry rootFolder(m_io, 0, "root");
            std::string thePath(path);
            char ch = *path.rbegin();
            // ignore trailing slash, but only if folder type
            // an entry of file type should never have a trailing
            // slash and is allowed to fail in this case
            if (ch == '/') {
                std::string(path.begin(), path.end() - 1).swap(thePath);
            }

            OptionalFolderEntry parentEntry = doGetParentFolderEntry(thePath, rootFolder);
            if (!parentEntry) {
                throw BFSException(BFSError::NotFound);
            }

            OptionalEntryInfo childInfo = parentEntry->getEntryInfo(boost::filesystem::path(thePath).filename().string());
            if (!childInfo) {
                throw BFSException(BFSError::NotFound);
            }
            if (childInfo->type() == EntryType::FileType) {
                throw BFSException(BFSError::NotFound);
            }

            if (removalType == FolderRemovalType::MustBeEmpty) {

                FolderEntry childEntry = parentEntry->getFolderEntry(boost::filesystem::path(thePath).filename().string());
                if (!childEntry.listAllEntries().empty()) {
                    throw BFSException(BFSError::FolderNotEmpty);
                }
            }

            parentEntry->removeFolderEntry(boost::filesystem::path(thePath).filename().string());
        }

        FileEntryDevice openFile(std::string const &path, OpenDisposition const &openMode)
        {
            FolderEntry rootFolder(m_io, 0, "root");
            char ch = *path.rbegin();
            if (ch == '/') {
                throw BFSException(BFSError::NotFound);
            }

            OptionalFolderEntry parentEntry = doGetParentFolderEntry(path, rootFolder);
            if (!parentEntry) {
                throw BFSException(BFSError::NotFound);
            }

            OptionalEntryInfo childInfo = parentEntry->getEntryInfo(boost::filesystem::path(path).filename().string());
            if (!childInfo) {
                throw BFSException(BFSError::NotFound);
            }
            if (childInfo->type() == EntryType::FolderType) {
                throw BFSException(BFSError::NotFound);
            }

            FileEntry fe = parentEntry->getFileEntry(boost::filesystem::path(path).filename().string(), openMode);

            return FileEntryDevice(fe);
        }

        void truncateFile(std::string const &path, std::ios_base::streamoff offset)
        {
            FolderEntry rootFolder(m_io, 0, "root");
            OptionalFolderEntry parentEntry = doGetParentFolderEntry(path, rootFolder);
            if (!parentEntry) {
                throw BFSException(BFSError::NotFound);
            }

            OptionalEntryInfo childInfo = parentEntry->getEntryInfo(boost::filesystem::path(path).filename().string());
            if (!childInfo) {
                throw BFSException(BFSError::NotFound);
            }
            if (childInfo->type() == EntryType::FolderType) {
                throw BFSException(BFSError::NotFound);
            }

            FileEntry fe = parentEntry->getFileEntry(boost::filesystem::path(path).filename().string(), OpenDisposition::buildOverwriteDisposition());
            fe.truncate(offset);
        }

      private:

        // the core bfs io (path, blocks, password)
        CoreBFSIO m_io;

        mutable boost::mutex m_accessMutex;
        typedef boost::lock_guard<boost::mutex> LockType;

        mutable FolderEntry m_rootFolder;

        BFS(); // not required

        void throwIfAlreadyExists(std::string const &path, FolderEntry &fe) const
        {
            // throw if already exists
            boost::filesystem::path processedPath(path);
            if (doFileExists(processedPath.string(), fe)) {
                throw BFSException(BFSError::AlreadyExists);
            }
            if (doFolderExists(processedPath.string(), fe)) {
                throw BFSException(BFSError::AlreadyExists);
            }
        }

        bool doFileExists(std::string const &path, FolderEntry &fe) const
        {
            return doExistanceCheck(path, EntryType::FileType, fe);
        }

        bool doFolderExists(std::string const &path, FolderEntry &fe) const
        {
            return doExistanceCheck(path, EntryType::FolderType, fe);
        }

        OptionalFolderEntry doGetParentFolderEntry(std::string const &path, FolderEntry &fe) const
        {
            boost::filesystem::path pathToCheck(path);

            if (pathToCheck.parent_path().string() == "/") {
                return OptionalFolderEntry(fe);
            }

            pathToCheck = pathToCheck.relative_path().parent_path();

            // iterate over path parts extracting sub folders along the way
            boost::filesystem::path::iterator it = pathToCheck.begin();
            FolderEntry folderOfInterest = fe;
            boost::filesystem::path pathBuilder;
            for (; it != pathToCheck.end(); ++it) {

                OptionalEntryInfo entryInfo = folderOfInterest.getEntryInfo(it->string());

                if (!entryInfo) {
                    return false;
                }

                pathBuilder /= entryInfo->filename();

                if (pathBuilder == pathToCheck) {

                    if (entryInfo->type() == EntryType::FolderType) {
                        return OptionalFolderEntry(folderOfInterest.getFolderEntry(entryInfo->filename()));
                    } else {
                        return OptionalFolderEntry();
                    }
                }
                // recurse deeper
                folderOfInterest = folderOfInterest.getFolderEntry(entryInfo->filename());
            }


            return OptionalFolderEntry();

        }

        bool doExistanceCheck(std::string const &path, EntryType const &entryType, FolderEntry &fe) const
        {
            std::string thePath(path);
            char ch = *path.rbegin();
            // ignore trailing slash, but only if folder type
            // an entry of file type should never have a trailing
            // slash and is allowed to fail in this case
            if (ch == '/' && entryType == EntryType::FolderType) {
                std::string(path.begin(), path.end() - 1).swap(thePath);
            }

            OptionalFolderEntry parentEntry = doGetParentFolderEntry(thePath, fe);
            if (!parentEntry) {
                return false;
            }

            std::string filename = boost::filesystem::path(thePath).filename().string();

            OptionalEntryInfo entryInfo = parentEntry->getEntryInfo(filename);

            if (!entryInfo) {
                return false;
            }

            if (entryInfo->type() == entryType) {
                return true;
            }

            return false;
        }
    };
}

#endif // BFS_BFS_HPP__
