/*
  Copyright (c) <2015>, <BenHJ>
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

#pragma once

#include "teasafe/CoreTeaSafeIO.hpp"
#include "teasafe/EntryInfo.hpp"
#include "teasafe/TeaSafeFolder.hpp"

#include <boost/optional.hpp>
#include <string>

namespace teasafe
{

    class CompoundFolder
    {
      public:
        /**
         * @brief constructs a CompoundFolder to write to. In this case the
         * starting block is unknown
         * @param the core teasafe io (path, blocks, password)
         * @param startBlock the index of the starting file block making up entry data
         * @param writable if data entries can be added to folder
         * @param name the name of the entry
         * @param enforceRootBlock true if we want to enforce the starting root block
         */
        CompoundFolder(SharedCoreIO const &io,
                       std::string const &name = "root",
                       bool const enforceRootBlock = false);

        /**
         * @brief constructs a CompoundFolder to read from
         * @param the core teasafe io (path, blocks, password)
         * @param startBlock the index of the starting file block making up entry data
         * @param writable if data entries can be added to folder
         * @param name the name of the entry
         */
        CompoundFolder(SharedCoreIO const &io,
                      uint64_t const startBlock,
                      std::string const &name = "root");


        /**
         * @brief appends a new file entry and start index to the entry data
         * @param name the name of the entry
         * @return a copy of a TeaSafeFile that will be used to reference the file data
         */
        void addFile(std::string const &name);

        /**
         * @brief appends a new folder entry and start index of the entry data
         * @param name the name of the entry
         * @return a copy of a CompoundFolder that will be used to reference the folder data
         */
        void addFolder(std::string const &name);

        /**
         * @brief retrieves a TeaSafeFile with specific name
         * @param name the name of the entry to lookup
         * @param openDisposition open mode
         * @return a copy of the TeaSafeFile with name
         */
        TeaSafeFile getFile(std::string const &name,
                            OpenDisposition const &openDisposition) const;

        /**
         * @brief retrieves a CompoundFolder with specific name
         * @param name the name of the entry to lookup
         * @return a copy of the CompoundFolder with name
         */
        CompoundFolder getFolder(std::string const &name) const;

        /**
         * @brief retrieves the name of this folder
         * @return the name
         */
        std::string getName() const;

        /**
         * @brief retrieves an entry info it exists
         * @param name the name of the info
         * @return an entry info if it exsist
         */
        SharedEntryInfo getEntryInfo(std::string const &name) const;

        /**
         * @brief returns a vector of all entry infos
         * @return all entry infos
         */
        std::vector<EntryInfo> listAllEntries() const;

        /**
         * @brief returns a vector of file entry infos
         * @return all file entry infos
         */
        std::vector<EntryInfo> listFileEntries() const;

        /**
         * @brief returns a vector of folder entry infos
         * @return all folder entry infos
         */
        std::vector<EntryInfo> listFolderEntries() const;

        /**
         * @brief does what it says
         * @param name the name of the entry
         */
        void removeFile(std::string const &name);

        /**
         * @brief removes a sub folder and all its content
         * @param name the name of the entry
         */
        void removeFolder(std::string const &name);

      private:

        CompoundFolder();

        void doAddCompoundFolderEntry();

        // the underlying folder that stores index folders
        mutable TeaSafeFolder m_compoundFolder;

        // a compound folder will be composed of multiple sub-folders
        // which are there to build up a more efficient folder structure
        std::vector<TeaSafeFolder> m_compoundEntries;

        // stores the name of this folder
        std::string m_name;

        int m_compoundFolderCount;
    };

}


