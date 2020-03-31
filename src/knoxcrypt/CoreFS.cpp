/*
  Copyright (c) <2013-present>, <BenHJ>
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

#include "knoxcrypt/EntryType.hpp"
#include "knoxcrypt/CompoundFolderEntryIterator.hpp"
#include "knoxcrypt/CoreFS.hpp"
#include "knoxcrypt/KnoxCryptException.hpp"

namespace knoxcrypt
{

    CoreFS::CoreFS(SharedCoreIO const &io)
        : m_io(io)
        , m_rootFolder(std::make_shared<CompoundFolder>(m_io, m_io->rootBlock, "root"))
        , m_folderCache()
        , m_stateMutex()
        , m_cachedFileAndPath(nullptr)
    {
    }

    CompoundFolder
    CoreFS::getFolder(std::string const &path)
    {
        StateLock lock(m_stateMutex);
        auto thePath(path);
        char ch = *path.rbegin();
        // ignore trailing slash, but only if folder type
        // an entry of file type should never have a trailing
        // slash and is allowed to fail in this case
        if (ch == '/') {
            std::string(path.begin(), path.end() - 1).swap(thePath);
        }

        auto parentEntry(doGetParentCompoundFolder(thePath));
        if (!parentEntry) {
            return *m_rootFolder;
        }
        return *parentEntry->getFolder(boost::filesystem::path(thePath).filename().string());
    }

    EntryInfo
    CoreFS::getInfo(std::string const &path)
    {
        StateLock lock(m_stateMutex);
        auto thePath(path);
        char ch = *path.rbegin();
        // ignore trailing slash, but only if folder type
        // an entry of file type should never have a trailing
        // slash and is allowed to fail in this case
        if (ch == '/') {
            std::string(path.begin(), path.end() - 1).swap(thePath);
        }

        auto parentEntry(doGetParentCompoundFolder(thePath));
        if (!parentEntry) {
            throw KnoxCryptException(KnoxCryptError::NotFound);
        }

        auto fname(boost::filesystem::path(thePath).filename().string());
        auto childInfo = parentEntry->getEntryInfo(boost::filesystem::path(thePath).filename().string());

        if (!childInfo) {
            throw KnoxCryptException(KnoxCryptError::NotFound);
        }
        return *childInfo;
    }


    bool
    CoreFS::fileExists(std::string const &path) const
    {
        StateLock lock(m_stateMutex);
        return doFileExists(path);
    }

    bool
    CoreFS::folderExists(std::string const &path)
    {
        StateLock lock(m_stateMutex);
        return doFolderExists(path);
    }

    void
    CoreFS::addFile(std::string const &path)
    {
        StateLock lock(m_stateMutex);
        auto thePath(path);
        char ch = *path.rbegin();
        // file entries with trailing slash should throw
        if (ch == '/') {
            throw KnoxCryptException(KnoxCryptError::IllegalFilename);
        }

        auto parentEntry(doGetParentCompoundFolder(thePath));

        if (!parentEntry) {
            throw KnoxCryptException(KnoxCryptError::NotFound);
        }

        // throw if already exists
        throwIfAlreadyExists(path);

        parentEntry->addFile(boost::filesystem::path(thePath).filename().string());
    }

    void
    CoreFS::addFolder(std::string const &path) const
    {
        StateLock lock(m_stateMutex);
        auto thePath(path);
        char ch = *path.rbegin();
        // ignore trailing slash
        if (ch == '/') {
            std::string(path.begin(), path.end() - 1).swap(thePath);
        }

        auto parentEntry(doGetParentCompoundFolder(thePath));
        if (!parentEntry) {
            throw KnoxCryptException(KnoxCryptError::NotFound);
        }

        // throw if already exists
        throwIfAlreadyExists(path);

        parentEntry->addFolder(boost::filesystem::path(thePath).filename().string());

        parentEntry->getCompoundFolder()->getStream()->close();
    }

    void
    CoreFS::renameEntry(std::string const &src, std::string const &dst)
    {
        StateLock lock(m_stateMutex);
        auto srcPath(src);
        char ch = *src.rbegin();
        // ignore trailing slash
        if (ch == '/') {
            std::string(src.begin(), src.end() - 1).swap(srcPath);
        }
        auto dstPath(dst);
        char chDst = *dst.rbegin();
        // ignore trailing slash
        if (chDst == '/') {
            std::string(dst.begin(), dst.end() - 1).swap(dstPath);
        }

        // throw if source parent doesn't exist
        auto parentSrc(doGetParentCompoundFolder(srcPath));
        if (!parentSrc) {
            throw KnoxCryptException(KnoxCryptError::NotFound);
        }

        // throw if destination parent doesn't exist
        auto parentDst(doGetParentCompoundFolder(dstPath));
        if (!parentSrc) {
            throw KnoxCryptException(KnoxCryptError::NotFound);
        }

        // throw if destination already exists
        throwIfAlreadyExists(dstPath);

        // throw if source doesn't exist
        auto const srcPathBoost = ::boost::filesystem::path(srcPath);
        auto const filename(srcPathBoost.filename().string());
        auto childInfo(parentSrc->getEntryInfo(filename));
        if (!childInfo) {
            throw KnoxCryptException(KnoxCryptError::NotFound);
        }

        // do moving / renaming
        // (i) Remove original entry metadata entry
        // (ii) Add new metadata entry with new file name
        // NOTE: As an optimization, if entry is to be moved to
        // same parent folder, don't bother invalidating parent metadata,
        // just update the name.
        auto const dstPathBoost = ::boost::filesystem::path(dstPath);
        auto const destPathParent(dstPathBoost.parent_path());
        auto const srcPathParent(srcPathBoost.parent_path());
        auto dstFilename(dstPathBoost.filename().string());

        if(destPathParent == srcPathParent) {
            parentSrc->updateMetaDataWithNewFilename(filename, dstFilename);
        } else {
            parentSrc->putMetaDataOutOfUse(filename);
            parentDst->writeNewMetaDataForEntry(dstFilename, childInfo->type(), childInfo->firstFileBlock());
        }

        // Need to remove parent entry from cache
        removeFolderFromCache(srcPathParent);

        // Also need to remove src from cache if folder
        if(childInfo->type() == EntryType::FolderType) {
            removeFolderFromCache(srcPath);
        }

        // need to also walk over children??

        // need to also check if this now fucks up the cached file
        resetCachedFile(srcPathBoost);
    }

    void
    CoreFS::resetCachedFile(::boost::filesystem::path const &thePath)
    {
        if(m_cachedFileAndPath) {

            auto cachedPath = boost::filesystem::path(m_cachedFileAndPath->first);
            auto boostFolderPath = thePath;
            do {
                cachedPath = cachedPath.parent_path();
                if(cachedPath == boostFolderPath) {
                    m_cachedFileAndPath.reset();
                    m_cachedFileAndPath = nullptr;
                    break;
                }

            } while (cachedPath.has_parent_path());
        }
    }

    void
    CoreFS::removeFile(std::string const &path)
    {
        StateLock lock(m_stateMutex);
        auto thePath(path);
        char ch = *path.rbegin();
        // ignore trailing slash
        if (ch == '/') {
            std::string(path.begin(), path.end() - 1).swap(thePath);
        }
        auto parentEntry(doGetParentCompoundFolder(thePath));
        if (!parentEntry) {
            throw KnoxCryptException(KnoxCryptError::NotFound);
        }

        try {
            parentEntry->removeFile(boost::filesystem::path(thePath).filename().string());
        } catch (...) {
            throw KnoxCryptException(KnoxCryptError::NotFound);
        }
    }

    void
    CoreFS::removeFolder(std::string const &path, FolderRemovalType const &removalType)
    {
        StateLock lock(m_stateMutex);
        auto thePath(path);
        char ch = *path.rbegin();
        // ignore trailing slash, but only if folder type
        // an entry of file type should never have a trailing
        // slash and is allowed to fail in this case
        if (ch == '/') {
            std::string(path.begin(), path.end() - 1).swap(thePath);
        }

        auto parentEntry(doGetParentCompoundFolder(thePath));
        if (!parentEntry) {
            throw KnoxCryptException(KnoxCryptError::NotFound);
        }

        auto boostPath = ::boost::filesystem::path(thePath);
        if (removalType == FolderRemovalType::MustBeEmpty) {
            auto childEntry(parentEntry->getFolder(boostPath.filename().string()));
            auto entryIt = childEntry->listAllEntries();
            CompoundFolderEntryIterator end;
            if (entryIt != end) {
                throw KnoxCryptException(KnoxCryptError::FolderNotEmpty);
            }
        }

        try {
            parentEntry->removeFolder(boostPath.filename().string());
        } catch (...) {
            throw KnoxCryptException(KnoxCryptError::NotFound);
        }

        // also remove entry and its parent from parent cache
        removeFolderFromCache(boostPath);
        removeFolderFromCache(boostPath.parent_path());

        // need to also check if this now fucks up the cached file
        resetCachedFile(thePath);
    }

    FileDevice
    CoreFS::openFile(std::string const &path, OpenDisposition const &openMode)
    {
        StateLock lock(m_stateMutex);
        char ch = *path.rbegin();
        if (ch == '/') {
            throw KnoxCryptException(KnoxCryptError::NotFound);
        }

        auto parentEntry(doGetParentCompoundFolder(path));
        if (!parentEntry) {
            throw KnoxCryptException(KnoxCryptError::NotFound);
        }

        setCachedFile(path, parentEntry, openMode);
        return FileDevice(m_cachedFileAndPath->second);
    }

    void
    CoreFS::truncateFile(std::string const &path, std::ios_base::streamoff offset)
    {
        StateLock lock(m_stateMutex);
        auto parentEntry(doGetParentCompoundFolder(path));
        if (!parentEntry) {
            throw KnoxCryptException(KnoxCryptError::NotFound);
        }

        setCachedFile(path, parentEntry, OpenDisposition::buildOverwriteDisposition());
        m_cachedFileAndPath->second->truncate(offset);
    }

    void
    CoreFS::setCachedFile(std::string const &path,
                           SharedCompoundFolder const &parentEntry,
                           OpenDisposition openMode) const
    {
        auto theName = boost::filesystem::path(path).filename().string();
        if(m_cachedFileAndPath) {
            if( m_cachedFileAndPath->first != path ||
               !m_cachedFileAndPath->second->getOpenDisposition().equals(openMode)) {
                m_cachedFileAndPath->second = std::make_shared<File>(parentEntry->getFile(theName, openMode));
            }
        } else {
            m_cachedFileAndPath.reset(new FileAndPathPair(path,
                                                          std::make_shared<File>(parentEntry->getFile(theName,
                                                                                                      openMode))));
        }
    }

    /**
     * @brief gets file system info; used when a 'df' command is issued
     * @param buf stores the filesystem stats data
     */
    void
    CoreFS::statvfs(struct statvfs *buf)
    {
        StateLock lock(m_stateMutex);
        buf->f_bsize   = detail::FILE_BLOCK_SIZE;
        buf->f_blocks  = m_io->blocks;
        buf->f_bfree   = m_io->freeBlocks;
        buf->f_bavail  = m_io->freeBlocks;

        // in CoreFS, the concept of an inode doesn't really exist so the
        // number of inodes is set to corresponds to the number of blocks
        buf->f_files   = m_io->blocks;
        buf->f_ffree   = m_io->freeBlocks;
        buf->f_favail  = m_io->freeBlocks;
        buf->f_namemax = detail::MAX_FILENAME_LENGTH;
    }

    void
    CoreFS::throwIfAlreadyExists(std::string const &path) const
    {
        // throw if already exists
        boost::filesystem::path processedPath(path);
        if (doFileExists(processedPath.string())) {
            throw KnoxCryptException(KnoxCryptError::AlreadyExists);
        }
        if (doFolderExists(processedPath.string())) {
            throw KnoxCryptException(KnoxCryptError::AlreadyExists);
        }
    }

    bool
    CoreFS::doFileExists(std::string const &path) const
    {
        return doExistanceCheck(path, EntryType::FileType);
    }

    bool
    CoreFS::doFolderExists(std::string const &path) const
    {
        return doExistanceCheck(path, EntryType::FolderType);
    }

    CoreFS::SharedCompoundFolder
    CoreFS::doGetParentCompoundFolder(std::string const &path) const
    {
        boost::filesystem::path pathToCheck(path);
        if (pathToCheck.parent_path().string() == "/") {
            return m_rootFolder;
        }

        pathToCheck = pathToCheck.relative_path().parent_path();

        // prefer to pull out of cache if it exists
        auto cacheIt(m_folderCache.find(pathToCheck.string()));
        if (cacheIt != m_folderCache.end()) {
            return cacheIt->second;
        }

        // iterate over path parts extracting sub folders along the way
        auto folderOfInterest(m_rootFolder);
        boost::filesystem::path pathBuilder;
        for (auto const & it : pathToCheck) {

            SharedEntryInfo entryInfo(folderOfInterest->getEntryInfo(it.string()));

            if (!entryInfo) {
                return SharedCompoundFolder();
            }

            pathBuilder /= entryInfo->filename();

            if (pathBuilder == pathToCheck) {

                if (entryInfo->type() == EntryType::FolderType) {
                    auto folder(folderOfInterest->getFolder(entryInfo->filename()));
                    m_folderCache.emplace(pathToCheck.string(), folder);
                    return folder;
                } else {
                    return SharedCompoundFolder();
                }
            }
            // recurse deeper
            folderOfInterest = folderOfInterest->getFolder(entryInfo->filename());
        }
        return SharedCompoundFolder();
    }

    bool
    CoreFS::doExistanceCheck(std::string const &path, EntryType const &entryType) const
    {
        auto thePath(path);

        // special case, check if we're the root folder
        if(thePath == "/" && entryType == EntryType::FolderType) {
            return true;
        }

        char ch = *path.rbegin();
        // ignore trailing slash, but only if folder type
        // an entry of file type should never have a trailing
        // slash and is allowed to fail in this case
        if (ch == '/' && entryType == EntryType::FolderType) {
            std::string(path.begin(), path.end() - 1).swap(thePath);
        }

        auto parentEntry = doGetParentCompoundFolder(thePath);
        if (!parentEntry) {
            return false;
        }

        auto filename(boost::filesystem::path(thePath).filename().string());

        auto entryInfo(parentEntry->getEntryInfo(filename));

        if (!entryInfo) {
            return false;
        }

        if (entryInfo->type() == entryType) {
            return true;
        }

        return false;
    }

    void
    CoreFS::removeAllChildFoldersToo(::boost::filesystem::path const &path,
                                     SharedCompoundFolder const &f)
    {
        std::vector<SharedEntryInfo> infos = f->listFolderEntries();
        for (auto const & entry : infos) {
            auto entryPath = path;
            entryPath /= entry->filename();
            removeFolderFromCache(entryPath);
        }
    }

    void
    CoreFS::removeFolderFromCache(::boost::filesystem::path const &path)
    {

        auto strPath = path.relative_path().string();
        // need to reset root path if root, otherwise
        // we'll continue to use 'cached' version
        if(strPath == "/") {
            removeAllChildFoldersToo(path, m_rootFolder);
            m_rootFolder = std::make_shared<CompoundFolder>(m_io, m_io->rootBlock, "root");
            return;
        }

        // else belongs to cache
        auto it(m_folderCache.find(strPath));
        if (it != m_folderCache.end()) {
            removeAllChildFoldersToo(path, it->second);
            m_folderCache.erase(it);
        }
    }
}
