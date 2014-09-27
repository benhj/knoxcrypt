/*
  Copyright (c) <2014>, <BenHJ>
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

/// builds a specifc type of cipher that will be used by the main image stream

#ifndef TEASAFE_CIPHER_BUILDER_HPP__
#define TEASAFE_CIPHER_BUILDER_HPP__

#include "cipher/AESByteTransformer.hpp"
#include "cipher/TwofishByteTransformer.hpp"
#include "cipher/SerpentByteTransformer.hpp"
#include "cipher/RC6ByteTransformer.hpp"
#include "cipher/MARSByteTransformer.hpp"
#include "cipher/CASTByteTransformer.hpp"
#include "cipher/CamelliaByteTransformer.hpp"
#include "cipher/RC5ByteTransformer.hpp"
#include "cipher/SHACAL2ByteTransformer.hpp"
#include "cipher/NullByteTransformer.hpp"

#include "teasafe/CoreTeaSafeIO.hpp"

#include <memory>

namespace teasafe
{

    std::shared_ptr<cipher::IByteTransformer> buildCipherType(SharedCoreIO const &io)
    {
        if(io->cipher == 2) {
            return std::make_shared<cipher::TwofishByteTransformer>(io->password,
                                                                      io->iv,
                                                                      io->iv2,
                                                                      io->iv3,
                                                                      io->iv3);
        } else if(io->cipher == 3) {
            return std::make_shared<cipher::SerpentByteTransformer>(io->password,
                                                                      io->iv,
                                                                      io->iv2,
                                                                      io->iv3,
                                                                      io->iv3);
        } else if(io->cipher == 4) {
            return std::make_shared<cipher::RC6ByteTransformer>(io->password,
                                                                  io->iv,
                                                                  io->iv2,
                                                                  io->iv3,
                                                                  io->iv3);
        } else if(io->cipher == 5) {
            return std::make_shared<cipher::MARSByteTransformer>(io->password,
                                                                  io->iv,
                                                                  io->iv2,
                                                                  io->iv3,
                                                                  io->iv3);
        } else if(io->cipher == 6) {
            return std::make_shared<cipher::CASTByteTransformer>(io->password,
                                                                   io->iv,
                                                                   io->iv2,
                                                                   io->iv3,
                                                                   io->iv3);
        } else if(io->cipher == 7) {
            return std::make_shared<cipher::CamelliaByteTransformer>(io->password,
                                                                       io->iv,
                                                                       io->iv2,
                                                                       io->iv3,
                                                                       io->iv3);
        } else if(io->cipher == 8) {
            return std::make_shared<cipher::RC5ByteTransformer>(io->password,
                                                                  io->iv,
                                                                  io->iv2,
                                                                  io->iv3,
                                                                  io->iv3);
        } else if(io->cipher == 9) {
            return std::make_shared<cipher::SHACAL2ByteTransformer>(io->password,
                                                                      io->iv,
                                                                      io->iv2,
                                                                      io->iv3,
                                                                      io->iv3);
        } else if(io->cipher == 0) {
            return std::make_shared<cipher::NullByteTransformer>(io->password,
                                                                   io->iv,
                                                                   io->iv2,
                                                                   io->iv3,
                                                                   io->iv3);
        } else {
            return std::make_shared<cipher::AESByteTransformer>(io->password,
                                                                  io->iv,
                                                                  io->iv2,
                                                                  io->iv3,
                                                                  io->iv3);
        }
    }
}

#endif // TEASAFE_CIPHER_BUILDER_HPP__