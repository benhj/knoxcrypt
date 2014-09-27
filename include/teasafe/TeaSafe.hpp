/*
  Copyright (c) <2013-2014>, <BenHJ>
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

#ifndef TEASAFE_TEASAFE_HPP__
#define TEASAFE_TEASAFE_HPP__

#include "teasafe/CoreTeaSafeIO.hpp"
#include "teasafe/TeaSafeFileDevice.hpp"
#include "teasafe/TeaSafeFolder.hpp"
#include "teasafe/FolderRemovalType.hpp"
#include "teasafe/OpenDisposition.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

#include <map>
#include <memory>
#include <string>

#include <sys/statvfs.h>

namespace teasafe
{
    class TeaSafe;
    typedef std::shared_ptr<TeaSafe> SharedTeaSafe;

    class TeaSafe
    {
        typedef std::shared_ptr<TeaSafeFolder> SharedTeaSafeFolder;

      public:
        explicit TeaSafe(SharedCoreIO const &io);

        /**
         * @brief  retrieves folder entry for given path
         * @param  path the path to retrieve entry for
         * @return the TeaSafeFolder
         * @throw  TeaSafeException if path cannot be found
         */
        TeaSafeFolder getTeaSafeFolder(std::string const &path);

        /**
         * @brief  retrieves metadata for given path
         * @param  path the path to retrieve metadata for
         * @return the meta data
         * @throw  TeaSafeException if path cannot be found
         */
        EntryInfo getInfo(std::string const &path);

        /**
         * @brief  file existence check
         * @param  path the path to check
         * @return true if path exists, false otherwise
         */
        bool fileExists(std::string const &path) const;

        /**
         * @brief  folder existence check
         * @param  path the path to check
         * @return true if path exists, false otherwise
         */
        bool folderExists(std::string const &path);

        /**
         * @brief adds an empty file
         * @param path the path of new file
         * @throw TeaSafeException illegal filename if path has '/' on end
         * @throw TeaSafeException if parent path cannot be found
         * @throw TeaSafeException AlreadyExists if path exists
         */
        void addFile(std::string const &path);

        /**
         * @brief creates a new folder
         * @param path the path of new folder
         * @throw TeaSafeException if parent path cannot be found
         * @throw TeaSafeException AlreadyExists if path exists
         */
        void addFolder(std::string const &path) const;

        /**
         * @brief for renaming
         * @param src entry to rename from
         * @param dst entry to rename to
         * @throw TeaSafeException NotFound if src or parent of dst cannot be found
         * @throw TeaSafeException AlreadyExists if dst already present
         */
        void renameEntry(std::string const &src, std::string const &dst);

        /**
         * @brief removes a file
         * @param path file to remove
         * @throw TeaSafeException NotFound if not found
         */
        void removeFile(std::string const &path);

        /**
         * @brief removes a folder
         * @param path folder to remove
         * @throw TeaSafeException NotFound if not found
         * @throw TeaSafeException NotEmpty if removalType is MustBeEmpty and
         * folder isn't empty
         */
        void removeFolder(std::string const &path, FolderRemovalType const &removalType);

        /**
         * @brief  opens a file
         * @param  path the file to open
         * @param  openMode the open mode
         * @return a seekable device to the opened file
         * @throw  TeaSaFeException not found if can't be found
         */
        TeaSafeFileDevice openFile(std::string const &path, OpenDisposition const &openMode);

        /**
         * @brief chops off end a file at given offset
         * @param path the file to truncate
         * @param offset the position at which to 'chop' the file
         */
        void truncateFile(std::string const &path, std::ios_base::streamoff offset);

        /**
         * @brief gets file system info; used when a 'df' command is issued
         * @param buf stores the filesystem stats data
         */
        void statvfs(struct statvfs *buf);

        void resetFileCache();

      private:

        // the core teasafe io (path, blocks, password)
        SharedCoreIO m_io;

        // the root of the tea safe filesystem
        mutable SharedTeaSafeFolder m_rootFolder;

        // so that folders don't have to be consistently rebuilt store
        // them as they are built in map and prefer to query map in future
        typedef std::map<std::string, SharedTeaSafeFolder> FolderCache;
        mutable FolderCache m_folderCache;

        typedef boost::mutex StateMutex;
        typedef boost::lock_guard<StateMutex> StateLock;
        mutable StateMutex m_stateMutex;

        // so that a new file doesn't need to be created each time the same file is opened
        typedef std::map<std::string, SharedTeaSafeFile> FileCache;
        mutable FileCache m_fileCache;

        TeaSafe(); // not required

        void throwIfAlreadyExists(std::string const &path) const;

        bool doFileExists(std::string const &path) const;

        bool doFolderExists(std::string const &path) const;

        SharedTeaSafeFolder doGetParentTeaSafeFolder(std::string const &path) const;

        bool doExistanceCheck(std::string const &path, EntryType const &entryType) const;

        /**
         * @brief when a folder is deleted, need to remove it from the parent cache
         * if it exists. Use this function to do so
         * @param path parent path
         */
        void removeDeletedParentFromCache(boost::filesystem::path const &path);

        /**
         * @brief  updates the file cache / retrieves a cached entry
         * @param  path the path of the file to update / retrieve
         * @param  parentEntry the parent of the entry
         * @param  openMode the mode to open the file in
         * @return the cached file
         */
        SharedTeaSafeFile setAndGetCachedFile(std::string const &path,
                                              SharedTeaSafeFolder const &parentEntry,
                                              OpenDisposition openMode) const;

        /**
         * @brief removed a file from the cache if it exists in cache
         * @param path the path of the file to remove
         */
        void removeFileFromFileCache(std::string const &path);
    };
}

#endif // TeaSafe_TeaSafe_HPP__
