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

#include "knoxcrypt/ContentFolder.hpp"
#include "knoxcrypt/ContentFolderEntryIterator.hpp"
#include "knoxcrypt/EntryInfo.hpp"

#include <boost/iterator/iterator_facade.hpp>
#include <map>
#include <memory>

namespace knoxcrypt
{

    class CompoundFolderEntryIterator : public boost::iterator_facade <CompoundFolderEntryIterator,
                                                                       std::shared_ptr<EntryInfo>,
                                                                       boost::forward_traversal_tag,
                                                                       std::shared_ptr<EntryInfo>>
    {
      public:

        CompoundFolderEntryIterator(std::vector<std::shared_ptr<ContentFolder>> contentFolders,
                                    std::map<std::string, SharedEntryInfo> & cache,
                                    bool const cacheShouldBeUpdated = true);

        CompoundFolderEntryIterator(std::map<std::string, SharedEntryInfo> & cache);

        void increment();

        bool equal(CompoundFolderEntryIterator const& other) const;

        std::shared_ptr<EntryInfo> dereference() const;

      private:

        std::vector<std::shared_ptr<ContentFolder>> m_contentFolders;
        std::map<std::string, SharedEntryInfo> & m_cache;

        // All leaf-folder buckets
        std::vector<std::shared_ptr<ContentFolder>>::iterator m_contentFoldersIterator;

        // For iterating over all entries in a single bucket
        ContentFolderEntryIterator m_bucketEntriesIterator;

        // If content is cached, we can iterate over the cache
        // using the following iterator instead
        std::map<std::string, SharedEntryInfo>::iterator m_cacheIterator;

        uint64_t m_bucketIndex;

        std::shared_ptr<EntryInfo> mutable m_entry;

        bool m_cacheShouldBeUpdated;

        bool nextContentFolder();
        bool nextEntry();

    };


}
