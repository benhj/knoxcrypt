/*
  Copyright (c) <2020>, <BenHJ>
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

#include "knoxcrypt/EntryInfo.hpp"
#include "knoxcrypt/File.hpp"

#include <boost/iterator/iterator_facade.hpp>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace knoxcrypt
{

    class ContentFolderEntryIterator 
    : public boost::iterator_facade <ContentFolderEntryIterator,
                                     std::shared_ptr<EntryInfo>,
                                     boost::forward_traversal_tag,
                                     std::shared_ptr<EntryInfo>>
    {
      public:

        ContentFolderEntryIterator(File * folderData,
                                   long const entryCount,
                                   std::function<std::shared_ptr<EntryInfo>
                                   (std::vector<uint8_t> const &metaData,
                                    uint64_t const entryIndex)>);

        ContentFolderEntryIterator();

        void increment();

        bool equal(ContentFolderEntryIterator const& other) const;

        std::shared_ptr<EntryInfo> dereference() const;

      private:
        File * m_folderData;
        long m_entryCount;
        std::function<std::shared_ptr<EntryInfo>
        (std::vector<uint8_t> const &metaData,
        uint64_t const entryIndex)> m_builder;
        long m_currentPosition;
        std::shared_ptr<EntryInfo> m_entry;
    };


}
