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

#include "teasafe/EntryType.hpp"
#include "teasafe/TeaSafe.hpp"
#include "teasafe/TeaSafeException.hpp"

namespace teasafe
{

    TeaSafe::TeaSafe(SharedCoreIO const &io)
        : m_io(io)
        , m_rootFolder(std::make_shared<CompoundFolder>(io, io->rootBlock, "root"))
        , m_folderCache()
        , m_stateMutex()
        , m_fileCache()
    {
    }
    
    CompoundFolder
    TeaSafe::getFolder(std::string const &path)
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

/*
        auto childInfo(parentEntry->getEntryInfo(boost::filesystem::path(thePath).filename().string()));
        if (!childInfo) {
            throw TeaSafeException(TeaSafeError::NotFound);
        }
        if (childInfo->type() == EntryType::FileType) {
            throw TeaSafeException(TeaSafeError::NotFound);
        }
*/
        return *parentEntry->getFolder(boost::filesystem::path(thePath).filename().string());
    }

    EntryInfo
    TeaSafe::getInfo(std::string const &path)
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
            throw TeaSafeException(TeaSafeError::NotFound);
        }

        auto fname(boost::filesystem::path(thePath).filename().string());
        auto childInfo = parentEntry->getEntryInfo(boost::filesystem::path(thePath).filename().string());

        if (!childInfo) {
            throw TeaSafeException(TeaSafeError::NotFound);
        }
        return *childInfo;
    }


    bool
    TeaSafe::fileExists(std::string const &path) const
    {
        StateLock lock(m_stateMutex);
        return doFileExists(path);
    }

    bool
    TeaSafe::folderExists(std::string const &path)
    {
        StateLock lock(m_stateMutex);
        return doFolderExists(path);
    }

    void
    TeaSafe::addFile(std::string const &path)
    {
        StateLock lock(m_stateMutex);
        auto thePath(path);
        char ch = *path.rbegin();
        // file entries with trailing slash should throw
        if (ch == '/') {
            throw TeaSafeException(TeaSafeError::IllegalFilename);
        }

        auto parentEntry(doGetParentCompoundFolder(thePath));

        if (!parentEntry) {
            throw TeaSafeException(TeaSafeError::NotFound);
        }

        // throw if already exists
        throwIfAlreadyExists(path);

        parentEntry->addFile(boost::filesystem::path(thePath).filename().string());
    }

    void
    TeaSafe::addFolder(std::string const &path) const
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
            throw TeaSafeException(TeaSafeError::NotFound);
        }

        // throw if already exists
        throwIfAlreadyExists(path);

        parentEntry->addFolder(boost::filesystem::path(thePath).filename().string());

        parentEntry->getCompoundFolder()->getStream()->close();
    }

    void
    TeaSafe::renameEntry(std::string const &src, std::string const &dst)
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
            throw TeaSafeException(TeaSafeError::NotFound);
        }

        // throw if destination parent doesn't exist
        auto parentDst(doGetParentCompoundFolder(dstPath));
        if (!parentSrc) {
            throw TeaSafeException(TeaSafeError::NotFound);
        }

        // throw if destination already exists
        throwIfAlreadyExists(dstPath);

        // throw if source doesn't exist
        auto const filename(boost::filesystem::path(srcPath).filename().string());
        auto childInfo(parentSrc->getEntryInfo(filename));
        if (!childInfo) {
            throw TeaSafeException(TeaSafeError::NotFound);
        }

        // do moving / renaming
        // (i) Remove original entry metadata entry
        // (ii) Add new metadata entry with new file name
        parentSrc->putMetaDataOutOfUse(filename);
        auto dstFilename(boost::filesystem::path(dstPath).filename().string());
        parentDst->writeNewMetaDataForEntry(dstFilename, childInfo->type(), childInfo->firstFileBlock());
    }

    void
    TeaSafe::removeFile(std::string const &path)
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
            throw TeaSafeException(TeaSafeError::NotFound);
        }

        /*
        auto childInfo(parentEntry->getEntryInfo(boost::filesystem::path(thePath).filename().string()));
        if (!childInfo) {
            throw TeaSafeException(TeaSafeError::NotFound);
        }
        if (childInfo->type() == EntryType::FolderType) {
            throw TeaSafeException(TeaSafeError::NotFound);
        }*/

        parentEntry->removeFile(boost::filesystem::path(thePath).filename().string());

        // also remove it from the cache if it exists
        this->removeFileFromFileCache(path);

    }

    void
    TeaSafe::removeFolder(std::string const &path, FolderRemovalType const &removalType)
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
            throw TeaSafeException(TeaSafeError::NotFound);
        }

        /*
        auto childInfo(parentEntry->getEntryInfo(boost::filesystem::path(thePath).filename().string()));
        if (!childInfo) {
            throw TeaSafeException(TeaSafeError::NotFound);
        }
        if (childInfo->type() == EntryType::FileType) {
            throw TeaSafeException(TeaSafeError::NotFound);
        }*/

        if (removalType == FolderRemovalType::MustBeEmpty) {

            auto childEntry(parentEntry->getFolder(boost::filesystem::path(thePath).filename().string()));
            if (!childEntry->listAllEntries().empty()) {
                throw TeaSafeException(TeaSafeError::FolderNotEmpty);
            }
        }

        parentEntry->removeFolder(boost::filesystem::path(thePath).filename().string());

        // also remove entry from parent cache
        this->removeDeletedParentFromCache(boost::filesystem::path(thePath));

    }

    TeaSafeFileDevice
    TeaSafe::openFile(std::string const &path, OpenDisposition const &openMode)
    {
        StateLock lock(m_stateMutex);
        char ch = *path.rbegin();
        if (ch == '/') {
            throw TeaSafeException(TeaSafeError::NotFound);
        }

        auto parentEntry(doGetParentCompoundFolder(path));
        if (!parentEntry) {
            throw TeaSafeException(TeaSafeError::NotFound);
        }

        /*
        auto childInfo(parentEntry->getEntryInfo(boost::filesystem::path(path).filename().string()));
        if (!childInfo) {
            throw TeaSafeException(TeaSafeError::NotFound);
        }
        if (childInfo->type() == EntryType::FolderType) {
            throw TeaSafeException(TeaSafeError::NotFound);
        }*/

        auto fe(this->setAndGetCachedFile(path, parentEntry, openMode));
        return TeaSafeFileDevice(fe);
    }

    void
    TeaSafe::truncateFile(std::string const &path, std::ios_base::streamoff offset)
    {
        StateLock lock(m_stateMutex);
        auto parentEntry(doGetParentCompoundFolder(path));
        if (!parentEntry) {
            throw TeaSafeException(TeaSafeError::NotFound);
        }

        /*
        auto childInfo(parentEntry->getEntryInfo(boost::filesystem::path(path).filename().string()));
        if (!childInfo) {
            throw TeaSafeException(TeaSafeError::NotFound);
        }
        if (childInfo->type() == EntryType::FolderType) {
            throw TeaSafeException(TeaSafeError::NotFound);
        }*/

        auto fe(this->setAndGetCachedFile(path, parentEntry, OpenDisposition::buildOverwriteDisposition()));
        fe->truncate(offset);
    }

    void
    TeaSafe::resetFileCache()
    {
        StateLock lock(m_stateMutex);
        FileCache().swap(m_fileCache);
    }

    SharedTeaSafeFile
    TeaSafe::setAndGetCachedFile(std::string const &path,
                                 SharedCompoundFolder const &parentEntry,
                                 OpenDisposition openMode) const
    {

        // very strictly limit the size of the cache to being able to hold
        // on to one entry only. TODO. does this mean that a map is a map
        // idea? We could just use a single pair.
        if(m_fileCache.size() > 1) {
            FileCache().swap(m_fileCache);
        }

        auto it(m_fileCache.find(path));
        if(it != m_fileCache.end()) {

            // note: need to also check if the openMode is different to the cached
            // version in which case the cached version should probably be rebuilt
            if(!it->second->getOpenDisposition().equals(openMode)) {
                m_fileCache.erase(path);
            } else {
                return it->second;
            }
        }

        auto sf(std::make_shared<TeaSafeFile>(parentEntry->getFile(boost::filesystem::path(path).filename().string(),
                                                                   openMode)));
        m_fileCache.insert(std::make_pair(path, sf));
        return sf;
    }

    /**
     * @brief gets file system info; used when a 'df' command is issued
     * @param buf stores the filesystem stats data
     */
    void
    TeaSafe::statvfs(struct statvfs *buf)
    {
        StateLock lock(m_stateMutex);
        buf->f_bsize   = detail::FILE_BLOCK_SIZE;
        buf->f_blocks  = m_io->blocks;
        buf->f_bfree   = m_io->freeBlocks;
        buf->f_bavail  = m_io->freeBlocks;

        // in teasafe, the concept of an inode doesn't really exist so the
        // number of inodes is set to corresponds to the number of blocks
        buf->f_files   = m_io->blocks;
        buf->f_ffree   = m_io->freeBlocks;
        buf->f_favail  = m_io->freeBlocks;
        buf->f_namemax = detail::MAX_FILENAME_LENGTH;
    }

    void
    TeaSafe::throwIfAlreadyExists(std::string const &path) const
    {
        // throw if already exists
        boost::filesystem::path processedPath(path);
        if (doFileExists(processedPath.string())) {
            throw TeaSafeException(TeaSafeError::AlreadyExists);
        }
        if (doFolderExists(processedPath.string())) {
            throw TeaSafeException(TeaSafeError::AlreadyExists);
        }
    }

    bool
    TeaSafe::doFileExists(std::string const &path) const
    {
        return doExistanceCheck(path, EntryType::FileType);
    }

    bool
    TeaSafe::doFolderExists(std::string const &path) const
    {
        return doExistanceCheck(path, EntryType::FolderType);
    }

    TeaSafe::SharedCompoundFolder
    TeaSafe::doGetParentCompoundFolder(std::string const &path) const
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
                    m_folderCache.insert(std::make_pair(pathToCheck.string(), folder));
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
    TeaSafe::doExistanceCheck(std::string const &path, EntryType const &entryType) const
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
    TeaSafe::removeDeletedParentFromCache(boost::filesystem::path const &path)
    {
        auto it(m_folderCache.find(path.relative_path().string()));
        if (it != m_folderCache.end()) {
            m_folderCache.erase(it);
        }
    }

    void
    TeaSafe::removeFileFromFileCache(std::string const &path)
    {
        auto it(m_fileCache.find(path));
        if(it != m_fileCache.end()) {
            m_fileCache.erase(it);
        }
    }

}
