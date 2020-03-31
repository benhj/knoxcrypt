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

#include "knoxcrypt/ContentFolderEntryIterator.hpp"
#include "knoxcrypt/detail/DetailKnoxCrypt.hpp"
#include "knoxcrypt/File.hpp"
#include <algorithm>

namespace knoxcrypt
{

    std::vector<uint8_t> doSeekAndReadOfEntryMetaData(File folderData,
                                                      int n,
                                                      uint32_t bufSize = 0,
                                                      uint64_t seekOff = 0)
    {
        // note in the following the '1' represents first byte,
        // the first bit of which indicates if entry is in use
        // the '8' is number of bytes representing the first block index
        // of the given file entry
        uint32_t bufferSize = 1 + detail::MAX_FILENAME_LENGTH + 8;

        // note in the following the '8' bytes represent the number of
        // entries in the folder
        if (folderData.seek(8 + (n * bufferSize) + seekOff) != -1) {
            std::vector<uint8_t> metaData(bufSize == 0 ? bufferSize : bufSize);
            folderData.read((char*)&metaData.front(), bufSize==0 ? bufferSize : bufSize);
            return metaData;
        }
        throw std::runtime_error("Problem retrieving metadata");
    }

    bool entryMetaDataIsEnabled(std::vector<uint8_t> const &bytes)
    {
        uint8_t byte = bytes[0];
        return detail::isBitSetInByte(byte, 0);
    }


    ContentFolderEntryIterator::ContentFolderEntryIterator(File * folderData,
                                   long const entryCount,
                                   std::function<std::shared_ptr<EntryInfo>
                                   (std::vector<uint8_t> const &metaData,
                                   uint64_t const entryIndex)> builder)
    : m_folderData(folderData)
    , m_entryCount(entryCount)
    , m_builder(std::move(builder))
    , m_currentPosition(0)
    , m_entry{nullptr}
    {
        increment();
    }

    ContentFolderEntryIterator::ContentFolderEntryIterator()
    : m_folderData(nullptr)
    , m_entryCount(static_cast<long>(0))
    , m_builder()
    , m_currentPosition(static_cast<long>(0))
    , m_entry{nullptr}
    {
    }

    void ContentFolderEntryIterator::increment()
    {

        if(m_currentPosition < m_entryCount) {
            auto metadata = doSeekAndReadOfEntryMetaData(*m_folderData, m_currentPosition);
            while (!entryMetaDataIsEnabled(metadata)) {
                ++m_currentPosition;
                if(m_currentPosition == m_entryCount) {
                    m_entry = nullptr;
                    return;
                }
                metadata = doSeekAndReadOfEntryMetaData(*m_folderData, m_currentPosition);
            }
            m_entry = m_builder(metadata, m_currentPosition);
            ++m_currentPosition;
            return;
        }
        m_entry = nullptr;
    }

    bool ContentFolderEntryIterator::equal(ContentFolderEntryIterator const& other) const
    {
        return m_entry == other.m_entry;
    }

    std::shared_ptr<EntryInfo> ContentFolderEntryIterator::dereference() const
    {
        return m_entry;
    }
}
