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

#include "knoxcrypt/CompoundFolderEntryIterator.hpp"
#include <algorithm>

namespace knoxcrypt
{

    CompoundFolderEntryIterator::CompoundFolderEntryIterator(std::vector<std::shared_ptr<ContentFolder>> contentFolders,
                                                             std::map<std::string, SharedEntryInfo> cache)
    : m_contentFolders(std::move(contentFolders))
    , m_cache(std::move(cache))
    , m_contentFoldersIterator(std::begin(m_contentFolders))
    , m_bucketEntriesIterator()
    , m_bucketIndex(0)
    , m_entry(nullptr)
    {
        if(m_contentFoldersIterator != std::end(m_contentFolders)) {
            m_bucketEntriesIterator = (*m_contentFoldersIterator)->listAllEntries();
            ++m_contentFoldersIterator;
            nextEntry();
        }
    }

    CompoundFolderEntryIterator::CompoundFolderEntryIterator()
    : m_contentFolders()
    , m_cache()
    , m_contentFoldersIterator()
    , m_bucketEntriesIterator()
    , m_bucketIndex(0)
    , m_entry(nullptr)
    {
    }

    void CompoundFolderEntryIterator::increment()
    {
        if(nextEntry()) {
            return;
        }
        if(nextContentFolder() && nextEntry()) {
            return;
        }
        m_entry = nullptr;
    }

    bool CompoundFolderEntryIterator::nextContentFolder()
    {
        if(m_contentFoldersIterator != std::end(m_contentFolders)) {
            m_bucketEntriesIterator = (*m_contentFoldersIterator)->listAllEntries();
            ++m_contentFoldersIterator;
            ++m_bucketIndex;
            return true;
        }
        return false;
    }
    bool CompoundFolderEntryIterator::nextEntry()
    {
        if(m_bucketEntriesIterator != ContentFolderEntryIterator()) {
            auto entry = *m_bucketEntriesIterator;
            if(m_cache.find(entry->filename()) == m_cache.end()) {
                entry->setBucketIndex(m_bucketIndex);
                m_cache.emplace(entry->filename(), *m_bucketEntriesIterator);
            }
            m_entry = entry;
            ++m_bucketEntriesIterator;
            return true;
        }
        return false;
    }

    bool CompoundFolderEntryIterator::equal(CompoundFolderEntryIterator const& other) const
    {
        return m_entry == other.m_entry;
    }

    std::shared_ptr<EntryInfo> CompoundFolderEntryIterator::dereference() const
    {
        return m_entry;
    }
}
