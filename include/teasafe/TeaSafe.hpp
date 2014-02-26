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
#include "teasafe/FileEntryDevice.hpp"
#include "teasafe/FolderEntry.hpp"
#include "teasafe/FolderRemovalType.hpp"
#include "teasafe/OpenDisposition.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include <string>

#include <sys/statvfs.h>

namespace teasafe
{
    class TeaSafe
    {
        typedef boost::optional<FolderEntry> OptionalFolderEntry;

      public:
        explicit TeaSafe(SharedCoreIO const &io);

        FolderEntry getCurrent(std::string const &path);

        EntryInfo getInfo(std::string const &path);

        bool fileExists(std::string const &path) const;
        bool folderExists(std::string const &path);

        void addFile(std::string const &path);

        void renameEntry(std::string const &src, std::string const &dst);

        void addFolder(std::string const &path) const;

        void removeFile(std::string const &path);

        void removeFolder(std::string const &path, FolderRemovalType const &removalType);

        FileEntryDevice openFile(std::string const &path, OpenDisposition const &openMode);

        void truncateFile(std::string const &path, std::ios_base::streamoff offset);

        /**
         * @brief gets file system info; used when a 'df' command is issued
         * @param buf stores the filesystem stats data
         */
        void statvfs(struct statvfs *buf);

      private:

        // the core teasafe io (path, blocks, password)
        SharedCoreIO m_io;

        TeaSafe(); // not required

        void throwIfAlreadyExists(std::string const &path, FolderEntry &fe) const;

        bool doFileExists(std::string const &path, FolderEntry &fe) const;

        bool doFolderExists(std::string const &path, FolderEntry &fe) const;

        OptionalFolderEntry doGetParentFolderEntry(std::string const &path, FolderEntry &fe) const;

        bool doExistanceCheck(std::string const &path, EntryType const &entryType, FolderEntry &fe) const;
    };
}

#endif // TeaSafe_TeaSafe_HPP__
