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

#ifndef TeaSafe_CORE_TeaSafe_IO_HPP__
#define TeaSafe_CORE_TeaSafe_IO_HPP__

#include "utility/EventType.hpp"

#include <functional>
#include <boost/optional.hpp>
#include <string>
#include <memory>

namespace teasafe
{

    class FileBlockBuilder;
    typedef std::shared_ptr<FileBlockBuilder> SharedBlockBuilder;

    struct CoreTeaSafeIO
    {
        std::string path;                // path of the tea safe image
        uint64_t blocks;                 // total number of blocks
        uint64_t freeBlocks;             // number of free blocks
        std::string password;            // password used to generate encryption key
        uint64_t iv;                     // IV used to initialize the cipher stream
        uint64_t iv2;                    // IV used to initialize the cipher stream
        uint64_t iv3;                    // IV used to initialize the cipher stream
        uint64_t iv4;                    // IV used to initialize the cipher stream
        unsigned int rounds;             // number of rounds used by enc. process
        unsigned int cipher;             // an ID signifying the cipher type TODO: USE ENUM!!
        uint64_t rootBlock;              // the start block of the root folder
        SharedBlockBuilder blockBuilder; // a block factory / resource manage
        typedef std::function<void(teasafe::EventType)> Callback;
        typedef boost::optional<Callback> OptionalCallback;
        OptionalCallback ccb;            // call back for cipher
    };

    typedef std::shared_ptr<CoreTeaSafeIO> SharedCoreIO;

}


#endif //TeaSafe_CORE_TeaSafe_IO_HPP__
