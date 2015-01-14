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
#include "cryptopp/blowfish.h"
#include "cryptopp/camellia.h"
#include "cryptopp/cast.h"
#include "cryptopp/ccm.h"
#include "cryptopp/des.h"
#include "cryptopp/idea.h"
#include "cryptopp/mars.h"
#include "cryptopp/rc5.h"
#include "cryptopp/rc6.h"
#include "cryptopp/seed.h"
#include "cryptopp/serpent.h"
#include "cryptopp/shacal2.h"
#include "cryptopp/skipjack.h"
#include "cryptopp/tea.h"
#include "cryptopp/twofish.h"


#include <memory>

#define BUILD_CIPHER(X) \
  return std::make_shared<CryptoByteTransformer<X> > (props);     

namespace cryptostreampp
{

    inline
    std::shared_ptr<IByteTransformer> buildCipherType(EncryptionProperties &props)
    {
        if(props.cipher == Algorithm::Twofish) {
            props.keyBytes = 32; // 256 bit
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::Twofish>::Encryption);
        } else if(props.cipher == Algorithm::Serpent) {
            props.keyBytes = 32; // 256 bit
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::Serpent>::Encryption);
        } else if(props.cipher == Algorithm::RC6) {
            props.keyBytes = 32; // 256 bit
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::RC6>::Encryption);
        } else if(props.cipher == Algorithm::MARS) {
            props.keyBytes = 32; // 256 bit
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::MARS>::Encryption);
        } else if(props.cipher == Algorithm::CAST256) {
            props.keyBytes = 32; // 256 bit
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::CAST256>::Encryption);
        } else if(props.cipher == Algorithm::Camellia) {
            props.keyBytes = 32; // 256 bit
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::Camellia>::Encryption);
        } else if(props.cipher == Algorithm::RC5) {
            props.keyBytes = 32; // 256 bit
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::RC5>::Encryption);
        } else if(props.cipher == Algorithm::SHACAL2) {
            props.keyBytes = 32; // 256 bit
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::SHACAL2>::Encryption);
        } else if(props.cipher == Algorithm::Blowfish) {
            props.keyBytes = 32; // 256 bit
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::Blowfish>::Encryption);
        } else if(props.cipher == Algorithm::SKIPJACK) {
            props.keyBytes = 10; // 80 bit
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::SKIPJACK>::Encryption);
        } else if(props.cipher == Algorithm::IDEA) {
            props.keyBytes = 16; // 128 bit
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::IDEA>::Encryption);
        } else if(props.cipher == Algorithm::SEED) {
            props.keyBytes = 16; // 128 bit
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::SEED>::Encryption);
        } else if(props.cipher == Algorithm::TEA) {
            props.keyBytes = 16; // 128 bit
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::TEA>::Encryption);
        } else if(props.cipher == Algorithm::XTEA) {
            props.keyBytes = 16; // 128 bit
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::XTEA>::Encryption);
        } else if(props.cipher == Algorithm::DES_EDE2) {
            props.keyBytes = 16; // 128 bit
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::DES_EDE2>::Encryption);
        } else if(props.cipher == Algorithm::DES_EDE3) {
            props.keyBytes = 24; // 192 bit
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::DES_EDE3>::Encryption);
        } else if(props.cipher == Algorithm::NONE) {
            return std::make_shared<NullByteTransformer>(props);
        } else {
            props.keyBytes = 32; // 256 bit
            BUILD_CIPHER(CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption);
        }
    }
}