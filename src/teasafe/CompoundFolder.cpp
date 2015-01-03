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


#include "teasafe/CompoundFolder.hpp"

#include <sstream>

namespace teasafe
{


    CompoundFolder::CompoundFolder(SharedCoreIO const &io,
                                   std::string const &name,
                                   bool const enforceRootBlock)
      : m_compoundFolder(std::make_shared<LeafFolder>(io, name, enforceRootBlock))
      , m_leafFolders()
      , m_name(name)
      , m_leafFolderCount(m_compoundFolder->getEntryCount())
    {
        if(m_leafFolderCount > 0) {
            auto folderInfos(m_compoundFolder->listFolderEntries());
            for(auto const & f : folderInfos) {
                m_leafFolders.push_back(m_compoundFolder->getLeafFolder(f.filename()));
            }
        }
    }

    CompoundFolder::CompoundFolder(SharedCoreIO const &io,
                                   uint64_t const startBlock,
                                   std::string const &name)
      : m_compoundFolder(std::make_shared<LeafFolder>(io, startBlock, name))
      , m_leafFolders()
      , m_name(name)
      , m_leafFolderCount(m_compoundFolder->getEntryCount())
    {
        if(m_leafFolderCount > 0) {
            auto folderInfos(m_compoundFolder->listFolderEntries());
            for(auto const & f : folderInfos) {
                m_leafFolders.push_back(m_compoundFolder->getLeafFolder(f.filename()));
            }
        }
    }

    void 
    CompoundFolder::doAddLeafFolder()
    {
        std::ostringstream ss;
        ss << "index_" << m_leafFolderCount;
        m_compoundFolder->addLeafFolder(ss.str());
        m_leafFolders.push_back(m_compoundFolder->getLeafFolder(ss.str()));
        ++m_leafFolderCount;
    }

    void 
    CompoundFolder::addFile(std::string const &name)
    {
        // check if compound entries is empty. These are
        // compound 'leaf' sub-folders
        if(m_leafFolders.empty()) {
            doAddLeafFolder();
        }

        // each leaf folder can have 100 entries
        bool wasAdded = false;
        for(auto & f : m_leafFolders) {
            if(f->getEntryCount() < 50) {
                f->addTeaSafeFile(name);
                wasAdded = true;
            }
        }

        // wasn't added. Means that there wasn't room so create
        // another leaf folder
        if(!wasAdded) {
            doAddLeafFolder();
            m_leafFolders.back()->addTeaSafeFile(name);
        }
    }

    void
    CompoundFolder::addFolder(std::string const &name)
    {
        // check if compound entries is empty. These are
        // compound 'leaf' sub-folders
        if(m_leafFolders.empty()) {
            doAddLeafFolder();
        }

        // each leaf folder can have 100 entries
        bool wasAdded = false;
        for(auto & f : m_leafFolders) {
            if(f->getEntryCount() < 50) {
                f->addCompoundFolder(name);
                wasAdded = true;
            }
        }

        // wasn't added. Means that there wasn't room so create
        // another leaf folder
        if(!wasAdded) {
            doAddLeafFolder();
            m_leafFolders.back()->addCompoundFolder(name);
        }
    }

    TeaSafeFile 
    CompoundFolder::getFile(std::string const &name,
                            OpenDisposition const &openDisposition) const
    {
        for(auto & f : m_leafFolders) {
            auto file(f->getTeaSafeFile(name, openDisposition));
            if(file) {
                return *file;
            }
        }
        throw std::runtime_error("File not found");
    }

    std::shared_ptr<CompoundFolder>
    CompoundFolder::getFolder(std::string const &name) const
    {
        for(auto & f : m_leafFolders) {
            auto folder(f->getCompoundFolder(name));
            if(folder) {
                return folder;
            }
        }
        throw std::runtime_error("Compound folder not found");
    }

    std::shared_ptr<LeafFolder>
    CompoundFolder::getCompoundFolder() const
    {
        return m_compoundFolder;
    }

    std::string 
    CompoundFolder::getName() const
    {
        return m_name;
    }

    SharedEntryInfo 
    CompoundFolder::getEntryInfo(std::string const &name) const
    {
        // try and pull out of cache fisrt
        for(auto const & f : m_leafFolders) {
            auto info(f->getEntryInfo(name));
            if(info) { 
                return info; 
            }
        }
        return SharedEntryInfo();
    }

    EntryInfoCacheMap
    CompoundFolder::listAllEntries() const
    {
        EntryInfoCacheMap map;

        for(auto const & f : m_leafFolders) {
            auto & leafEntries(f->listAllEntries());
            for(auto const & entry : leafEntries) {
                map.insert(entry);
            }
        }

        return map;
    }

    std::vector<EntryInfo> 
    CompoundFolder::listFileEntries() const
    {
        std::vector<EntryInfo> infos;
        for(auto const & f : m_leafFolders) {
            auto leafEntries(f->listFileEntries());
            for(auto const & entry : leafEntries) {
                infos.push_back(entry);
            }
        }
        return infos;
    }

    std::vector<EntryInfo> 
    CompoundFolder::listFolderEntries() const
    {
        std::vector<EntryInfo> infos;
        for(auto const & f : m_leafFolders) {
            auto leafEntries(f->listFolderEntries());
            for(auto const & entry : leafEntries) {
                infos.push_back(entry);
            }
        }
        return infos;
    }

    void 
    CompoundFolder::removeFile(std::string const &name)
    {
        for(auto & f : m_leafFolders) {
            if(f->removeTeaSafeFile(name)) {                
                // decrement number of entries in leaf
                if(f->getEntryCount() == 0) {
                    m_compoundFolder->removeLeafFolder(f->getName());
                }
                return; 
            }
        }
    }

    void 
    CompoundFolder::removeFolder(std::string const &name)
    {
        for(auto & f : m_leafFolders) {
            if(f->removeCompoundFolder(name)) { 
                // decrement number of entries in leaf
                if(f->getEntryCount() == 0) {
                    m_compoundFolder->removeLeafFolder(f->getName());
                }
                return;
            }
        }
    }

    void
    CompoundFolder::putMetaDataOutOfUse(std::string const &name)
    {
        for(auto & f : m_leafFolders) {
            if(f->putMetaDataOutOfUse(name)) { return; }
        }
        throw std::runtime_error("Error putting metadata out of use");
    }

    void
    CompoundFolder::writeNewMetaDataForEntry(std::string const &name,
                                             EntryType const &entryType,
                                             uint64_t startBlock)
    {
        // each leaf folder can have 100 entries
        bool wasAdded = false;
        for(auto & f : m_leafFolders) {
            if(f->getEntryCount() < 50) {
                f->writeNewMetaDataForEntry(name, entryType, startBlock);
                wasAdded = true;
            }
        }

        // wasn't added. Means that there wasn't room so create
        // another leaf folder
        if(!wasAdded) {
            doAddLeafFolder();
            m_leafFolders.back()->writeNewMetaDataForEntry(name, entryType, startBlock);
        }
    }
}