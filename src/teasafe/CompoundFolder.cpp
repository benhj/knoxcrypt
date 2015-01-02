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

namespace teasafe
{


    CompoundFolder::CompoundFolder(SharedCoreIO const &io,
                                   std::string const &name,
                                   bool const enforceRootBlock)
      : m_compoundFolder(io, name, enforceRootBlock)
    {

    }

    CompoundFolder::CompoundFolder(SharedCoreIO const &io,
                                   uint64_t const startBlock,
                                   std::string const &name)
      : m_compoundFolder(io, startBlock, name)
    {

    }

    void 
    CompoundFolder::addFile(std::string const &name)
    {

    }

    void
    CompoundFolder::addFolder(std::string const &name)
    {

    }

    TeaSafeFile 
    CompoundFolder::getFile(std::string const &name,
                            OpenDisposition const &openDisposition) const
    {

    }

    CompoundFolder 
    CompoundFolder::getFolder(std::string const &name) const
    {

    }

    std::string 
    CompoundFolder::getName() const
    {
        return m_name;
    }

    SharedEntryInfo 
    CompoundFolder::getEntryInfo(std::string const &name) const
    {

    }

    std::vector<EntryInfo> 
    CompoundFolder::listAllEntries() const
    {

    }

    std::vector<EntryInfo> 
    CompoundFolder::listFileEntries() const
    {

    }

    std::vector<EntryInfo> 
    CompoundFolder::listFolderEntries() const
    {

    }

    void 
    CompoundFolder::removeFile(std::string const &name)
    {

    }

    void 
    CompoundFolder::removeFolder(std::string const &name)
    {

    }
}