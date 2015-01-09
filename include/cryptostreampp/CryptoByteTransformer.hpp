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

#include "EncryptionProperties.hpp"
#include "IByteTransformer.hpp"
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
#include <cstdint>
#include <ios>
#include <string>

namespace cryptostreampp
{

    template <typename Algorithm>
    class CryptoByteTransformer : public IByteTransformer
    {
      public:
        CryptoByteTransformer(EncryptionProperties const &encProps);

        void init();

        ~CryptoByteTransformer();

      private:

        CryptoByteTransformer(); // not required

        void doEncrypt(char *in, char *out, std::ios_base::streamoff startPosition, long length) const;
        void doDecrypt(char *in, char *out, std::ios_base::streamoff startPosition, long length) const;
    };

    template <typename T>
    CryptoByteTransformer<T>::CryptoByteTransformer(EncryptionProperties const &encProps)
      : IByteTransformer(encProps)
    {
    }

    template <typename T>
    inline
    void
    CryptoByteTransformer<T>::init()
    {
        IByteTransformer::generateKeyAndIV();
    }

    template <typename T>
    CryptoByteTransformer<T>::~CryptoByteTransformer()
    {

    }

    template <typename T>
    inline
    void 
    CryptoByteTransformer<T>::doEncrypt(char *in, char *out, std::ios_base::streamoff startPosition, long length) const
    {
        typename CryptoPP::CTR_Mode<T>::Encryption encryptor;
        encryptor.SetKeyWithIV(IByteTransformer::g_bigKey, 
                               sizeof(IByteTransformer::g_bigKey), 
                               IByteTransformer::g_bigIV);
        encryptor.Seek(startPosition);
        encryptor.ProcessData((uint8_t*)out, (uint8_t*)in, length);
    }

    template <typename T>
    inline
    void 
    CryptoByteTransformer<T>::doDecrypt(char *in, char *out, std::ios_base::streamoff startPosition, long length) const
    {
        typename CryptoPP::CTR_Mode<T>::Decryption decryptor;
        decryptor.SetKeyWithIV(IByteTransformer::g_bigKey, 
                                  sizeof(IByteTransformer::g_bigKey), 
                                  IByteTransformer::g_bigIV);
        decryptor.Seek(startPosition);
        decryptor.ProcessData((uint8_t*)out, (uint8_t*)in, length);
    }

}

