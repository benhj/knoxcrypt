/*
  Copyright (c) <2015>, <BenHJ>
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

#include "Algorithms.hpp"
#include "CryptoByteTransformer.hpp"
#include "NullByteTransformer.hpp"
#include "EncryptionProperties.hpp"

#include "cryptopp/aes.h"
#include "cryptopp/camellia.h"
#include "cryptopp/mars.h"
#include "cryptopp/rc5.h"
#include "cryptopp/rc6.h"
#include "cryptopp/serpent.h"
#include "cryptopp/shacal2.h"
#include "cryptopp/twofish.h"
#include "cryptopp/cast.h"
#include "cryptopp/ccm.h"

#include <memory>

#define BUILD_CIPHER(X) \
  return std::make_shared<CryptoByteTransformer<X> > (props);     

namespace cryptostreampp
{

    inline
    std::shared_ptr<IByteTransformer> buildCipherType(EncryptionProperties const &props)
    {
        if(props.cipher == Algorithm::Twofish) {
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::Twofish>::Encryption);
        } else if(props.cipher == Algorithm::Serpent) {
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::Serpent>::Encryption);
        } else if(props.cipher == Algorithm::RC6) {
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::RC6>::Encryption);
        } else if(props.cipher == Algorithm::MARS) {
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::MARS>::Encryption);
        } else if(props.cipher == Algorithm::CAST256) {
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::CAST256>::Encryption);
        } else if(props.cipher == Algorithm::Camellia) {
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::Camellia>::Encryption);
        } else if(props.cipher == Algorithm::RC5) {
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::RC5>::Encryption);
        } else if(props.cipher == Algorithm::SHACAL2) {
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::SHACAL2>::Encryption);
        } else if(props.cipher == Algorithm::NONE) {
            return std::make_shared<NullByteTransformer>(props);
        } else {
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption);
        }
    }
}