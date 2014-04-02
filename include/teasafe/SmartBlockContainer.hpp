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

#ifndef TEASAFE_SMART_BLOCK_CONTAINER_HPP__
#define TEASAFE_SMART_BLOCK_CONTAINER_HPP__

#include "teasafe/CoreTeaSafeIO.hpp"
#include "teasafe/FileBlock.hpp"
#include "teasafe/OpenDisposition.hpp"

#include <boost/optional.hpp>

namespace teasafe
{



class SmartBlockContainer
{

    typedef boost::optional<FileBlock> WorkingFileBlock;

public:
    SmartBlockContainer(SharedCoreIO const &io,
                        uint64_t const rootBlockIndex,
                        OpenDisposition const &openDisposition);
    void push_back(FileBlock const &block);
    FileBlock operator[](size_t const n);
    size_t size();
private:
    SharedCoreIO m_io;
    uint64_t m_rootBlockIndex;
    OpenDisposition m_openDisposition;
    size_t m_size;
    WorkingFileBlock m_workingBlock;
    FileBlock findBlock(size_t const n);
    size_t countBlocks();
};

}

#endif // TEASAFE_SMART_BLOCK_CONTAINER_HPP__
