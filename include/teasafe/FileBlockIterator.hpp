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

#ifndef TEASAFE_FILE_BLOCK_ITERATOR_HPP__
#define TEASAFE_FILE_BLOCK_ITERATOR_HPP__

#include "teasafe/CoreTeaSafeIO.hpp"
#include "teasafe/FileBlock.hpp"
#include "teasafe/OpenDisposition.hpp"

#include <boost/iterator/iterator_facade.hpp>

namespace teasafe
{

    class FileBlockIterator : public boost::iterator_facade <FileBlockIterator ,
                                                             FileBlock,
                                                             boost::forward_traversal_tag,
                                                             FileBlock>
    {
    public:
        /**
         * @brief for constructing begin iterator
         * @param io
         * @param rootBlock
         * @param openDisposition
         */
        FileBlockIterator(SharedCoreIO const &io,
                          uint64_t rootBlock,
                          OpenDisposition const &openDisposition);

        /**
         * @brief for constructing end iterator
         */
        FileBlockIterator();

        void increment();

        bool equal(FileBlockIterator const& other) const;

        FileBlock dereference() const;

    private:

        SharedCoreIO m_io;
        OpenDisposition m_openDisposition;

        typedef boost::optional<FileBlock> WorkingFileBlock;
        WorkingFileBlock m_workingFileBlock;

    };


}

#endif // TEASAFE_FILE_BLOCK_ITERATOR_HPP__
