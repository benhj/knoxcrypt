/*
  Copyright (c) <2013-2015>, <BenHJ>
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

#include "cryptostreampp/EncryptionProperties.hpp"

#include "utility/EventType.hpp"

#include <functional>
#include <boost/optional.hpp>
#include <string>
#include <memory>

namespace knoxcrypt
{

    class FileBlockBuilder;
    using SharedBlockBuilder = std::shared_ptr<FileBlockBuilder>;

    struct CoreKnoxCryptIO
    {
        std::string path;                // path of the tea safe image
        uint64_t blocks;                 // total number of blocks
        uint64_t freeBlocks;             // number of free blocks
        cryptostreampp::EncryptionProperties encProps; // stuff like password and iv
        unsigned int rounds;             // number of rounds used by enc. process
        uint64_t rootBlock;              // the start block of the root folder
        SharedBlockBuilder blockBuilder; // a block factory / resource manage
        using Callback = std::function<void(knoxcrypt::EventType)>;
        using OptionalCallback = boost::optional<Callback>;
        OptionalCallback ccb;            // call back for cipher
        bool useBlockCache;              // cache available file blocks for faster retrieval
        bool firstTimeInit;              // initialized very first time
        
        // Should key be initialized very first time?
        CoreKnoxCryptIO() : firstTimeInit(false) {}
        
    };

    using SharedCoreIO = std::shared_ptr<CoreKnoxCryptIO>;

}
