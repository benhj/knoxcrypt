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

#ifndef TeaSafe_ENTRY_INFO_HPP__
#define TeaSafe_ENTRY_INFO_HPP__

#include "teasafe/EntryType.hpp"

#include <string>

namespace teasafe
{

    class EntryInfo
    {
      public:
        EntryInfo(std::string const &fileName,
                  uint64_t const &fileSize,
                  EntryType const &entryType,
                  bool const writable,
                  uint64_t const firstFileBlock,
                  uint64_t const folderIndex);

        /**
         * @brief  access the name of the entry
         * @return the entry name
         */
        std::string filename() const;

        /**
         * @brief  access the size of the entry; not a folder entry
         *         has a size of zero bytes
         * @return the size of the entry
         */
        uint64_t size() const;

        /**
         * @brief  accesses the type of the entry (file or folder)
         * @return EntryType::File if file, EntryType::Folder if folder
         */
        EntryType type() const;

        /**
         * @brief  indicates if entry is wrtiable or not. In the current
         *         implementation, all entries are writable
         * @return true if the entry is writable, false otherwise
         */
        bool writable() const;

        /**
         * @brief  reports the first file block index of the associated file data
         * @return the index of the first file block
         */
        uint64_t firstFileBlock() const;

        /**
         * @brief  reports the index of the entry's position in the folder's list of entries
         * @return the index of the entry in the folder
         */
        uint64_t folderIndex() const;

      private:
        std::string m_fileName;
        uint64_t m_fileSize;
        EntryType m_entryType;
        bool m_writable;
        uint64_t m_firstFileBlock;
        uint64_t m_folderIndex;
    };

}

#endif // TeaSafe_ENTRY_INFO_HPP__
