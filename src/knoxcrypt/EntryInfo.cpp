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

#include "knoxcrypt/EntryInfo.hpp"

namespace knoxcrypt
{
    EntryInfo::EntryInfo(std::string const &fileName,
                         uint64_t const &fileSize,
                         EntryType const &entryType,
                         bool const writable,
                         uint64_t const firstFileBlock,
                         uint64_t const folderIndex)
        : m_fileName(fileName)
        , m_fileSize(fileSize)
        , m_entryType(entryType)
        , m_writable(writable)
        , m_firstFileBlock(firstFileBlock)
        , m_folderIndex(folderIndex)
        , m_hasBucketIndex(false)
        , m_bucketIndex(0) // TODO, is this initialization wise?
    {

    }

    std::string
    EntryInfo::filename() const
    {
        return m_fileName;
    }

    uint64_t
    EntryInfo::size() const
    {
        return m_fileSize;
    }

    void
    EntryInfo::updateSize(uint64_t newSize)
    {
        m_fileSize = newSize;
    }

    EntryType
    EntryInfo::type() const
    {
        return m_entryType;
    }

    bool
    EntryInfo::writable() const
    {
        return m_writable;
    }

    uint64_t
    EntryInfo::firstFileBlock() const
    {
        return m_firstFileBlock;
    }

    uint64_t
    EntryInfo::folderIndex() const
    {
        return m_folderIndex;
    }

    void 
    EntryInfo::setBucketIndex(uint64_t const bucketIndex)
    {
        m_bucketIndex = bucketIndex;
        m_hasBucketIndex = true;
    }

    bool
    EntryInfo::hasBucketIndex() const 
    {
        return m_hasBucketIndex;
    }

    uint64_t
    EntryInfo::bucketIndex() const
    {
        return m_bucketIndex;
    }

}
