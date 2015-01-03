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

#ifndef TeaSafe_TEASAFE_FOLDER_HPP__
#define TeaSafe_TEASAFE_FOLDER_HPP__

#include "teasafe/CoreTeaSafeIO.hpp"
#include "teasafe/EntryInfo.hpp"
#include "teasafe/TeaSafeFile.hpp"

#include <boost/optional.hpp>
  
#include <memory>
#include <map>

namespace teasafe
{

    typedef boost::optional<std::ios_base::streamoff> OptionalOffset;
    typedef std::shared_ptr<EntryInfo> SharedEntryInfo;
    typedef std::map<std::string, SharedEntryInfo> EntryInfoCacheMap;

    class CompoundFolder;

    class LeafFolder
    {
      public:
        /**
         * @brief constructs a LeafFolder to write to. In this case the
         * starting block is unknown
         * @param the core teasafe io (path, blocks, password)
         * @param startBlock the index of the starting file block making up entry data
         * @param writable if data entries can be added to folder
         * @param name the name of the entry
         * @param enforceRootBlock true if we want to enforce the starting root block
         */
        LeafFolder(SharedCoreIO const &io,
                      std::string const &name = "root",
                      bool const enforceRootBlock = false);

        /**
         * @brief constructs a LeafFolder to read from
         * @param the core teasafe io (path, blocks, password)
         * @param startBlock the index of the starting file block making up entry data
         * @param writable if data entries can be added to folder
         * @param name the name of the entry
         */
        LeafFolder(SharedCoreIO const &io,
                      uint64_t const startBlock,
                      std::string const &name = "root");


        /**
         * @brief appends a new file entry and start index to the entry data
         * @param name the name of the entry
         * @return a copy of a TeaSafeFile that will be used to reference the file data
         */
        void addTeaSafeFile(std::string const &name);

        /**
         * @brief appends a new folder entry and start index of the entry data
         * @param name the name of the entry
         * @return a copy of a LeafFolder that will be used to reference the folder data
         */
        void addLeafFolder(std::string const &name);

        /// for adding a compound folder
        void addCompoundFolder(std::string const &name);

        /**
         * @brief retrieves a TeaSafeFile with specific name
         * @param name the name of the entry to lookup
         * @param openDisposition open mode
         * @return a copy of the TeaSafeFile with name
         */
        boost::optional<TeaSafeFile> getTeaSafeFile(std::string const &name,
                                                    OpenDisposition const &openDisposition) const;

        /**
         * @brief retrieves a LeafFolder with specific name
         * @param name the name of the entry to lookup
         * @return a copy of the LeafFolder with name
         */
        std::shared_ptr<LeafFolder> getLeafFolder(std::string const &name) const;

        /// for retrieving a compound folder
        std::shared_ptr<CompoundFolder> getCompoundFolder(std::string const &name) const;

        /**
         * @brief retrieves the name of this folder
         * @return the name
         */
        std::string getName() const;

        SharedImageStream getStream() const;


        /**
         * @brief retrieves the entry info attributes of a given indexed entry
         * @param index the index of the entry to get the attributes of
         * @return the entry information
         */
        EntryInfo getEntryInfo(uint64_t const index) const;


        /**
         * @brief retrieves an entry info it exists
         * @param name the name of the info
         * @return an entry info if it exsist
         */
        SharedEntryInfo getEntryInfo(std::string const &name) const;


        /**
         * @brief  a private version of getEntryInfo
         * @see    getEntryInfo
         * @param  name
         * @return entryInfo
         */
        SharedEntryInfo doGetNamedEntryInfo(std::string const &name) const;

        /**
         * @brief retrieves an entry info of a file if it exists
         * @param name the name of the info
         * @return an entry info if it exsist
         */
        SharedEntryInfo getFolderEntryInfo(std::string const &name) const;

        /**
         * @brief returns a vector of all entry infos
         * @return all entry infos
         */
        EntryInfoCacheMap & listAllEntries() const;

        /**
         * @brief returns a vector of file entry infos
         * @return all file entry infos
         */
        std::vector<SharedEntryInfo> listFileEntries() const;

        /**
         * @brief returns a vector of folder entry infos
         * @return all folder entry infos
         */
        std::vector<SharedEntryInfo> listFolderEntries() const;

        /**
         * @brief does what it says
         * @param name the name of the entry
         * @return true if successful
         */
        bool removeTeaSafeFile(std::string const &name);

        /**
         * @brief removes a sub folder and all its content
         * @param name the name of the entry
         * @return true if successful
         */
        bool removeLeafFolder(std::string const &name);

        /**
         * @brief removes a compound sub folder and all its content
         * @param name the name of the entry
         * @return true if successful
         */
        bool removeCompoundFolder(std::string const &name);

        /**
         * @brief puts metadata for given entry out of use
         * @param name name of entry
         * @return true if successful
         */
        bool putMetaDataOutOfUse(std::string const &name);

        /**
         * @brief for writing new entry metadata
         * @param name name of entry
         * @param entryType the type of the entry
         * @param startBlock start block of entry
         */
        void writeNewMetaDataForEntry(std::string const &name,
                                      EntryType const& entryType,
                                      uint64_t startBlock);

        uint64_t getEntryCount() const;

      private:

        LeafFolder();

        /**
         * @brief for writing new entry metadata
         * @param name name of entry
         * @param entryType the type of the entry
         * @param startBlock start block of entry
         */
        void doWriteNewMetaDataForEntry(std::string const &name,
                                        EntryType const& entryType,
                                        uint64_t startBlock);

        /**
         * @brief a private accessor for getting file entry from metadata
         * @param metaData the entry metadata
         * @param index the index of the entry
         * @return the info metadata in entry info struct
         */
        SharedEntryInfo doGetEntryInfo(std::vector<uint8_t> const &metaData, uint64_t const index) const;

        /**
         * @brief puts metadata for given entry out of use
         * @param name name of entry
         * @return true if successful
         */
        bool doPutMetaDataOutOfUse(std::string const &name);

        /**
         * @brief returns the entry index given the name
         * @param name the name of the entry
         * @return the index
         */
        boost::optional<long> doGetMetaDataIndexForEntry(std::string const &name) const;

        /**
         * @brief write metadata to this folder entry
         * @note assumes in correct position
         * @param buf
         * @param n
         * @return numberof bytes written
         */
        std::streamsize doWrite(char const * buf, std::streampos n);

        /**
         * @brief write the first byte of the file metadata data
         * @note assumes in correct position
         * @return
         */
        std::streamsize doWriteFirstByteToEntryMetaData(EntryType const &entryType);

        /**
         * @brief write filename file metadata
         * @return number of bytes writeen
         */
        std::streamsize doWriteFilenameToEntryMetaData(std::string const &name);

        /**
         * @brief writes the first block index bytes to the file metadata
         * @param first block, the block index of the file entry
         * @return number of bytes written
         */
        std::streamsize doWriteFirstBlockIndexToEntryMetaData(uint64_t firstBlock);

        /**
         * @brief seeks to where the metadata should be written. If
         * metadata for a previous entry has been deleted, we should
         * use that position instead to write new data
         * @return true if pre-existing metadata is to be overwritten,
         * false otherwise
         */
        OptionalOffset
        doFindOffsetWhereMetaDataShouldBeWritten();

        /**
         * @brief lists a particular type of entry, file or folder
         * @return a list of entries of specified type
         */
        std::vector<SharedEntryInfo> doListEntriesBasedOnType(EntryType entryType) const;

        /**
         * @brief after unlinking this will be called to indicate that cache entry
         * should be removed
         * @param name entry name
         */
        void invalidateEntryInEntryInfoCache(std::string const &name);

        // the core teasafe io (path, blocks, password)
        SharedCoreIO m_io;

        // the underlying file blocks storing the folder entry data
        mutable TeaSafeFile m_folderData;

        uint64_t m_startVolumeBlock;

        // stores the name of this folder
        std::string m_name;

        // as an optimization, store the number of entries so that we
        // don't have to read each time (read once during construction)
        long m_entryCount;

        // An experimental optimization: a map will store entry infos as they
        // are generated so that in future, they don't have to be regenerated.
        // Question: when to invalidate/update an entry in the cache?
        mutable EntryInfoCacheMap m_entryInfoCacheMap;

        // when an entry is deleted, its metadata is put out of use meaning that
        // there might be somewhere before the end that metadata for a new file can
        // be written so should check list of entries to find 'blank' space.
        bool m_checkForEarlyMetaData;

    };

}



#endif // TeaSafe_TEASAFE_FOLDER_HPP__
