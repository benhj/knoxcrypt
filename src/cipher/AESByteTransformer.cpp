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

#include "cipher/AESByteTransformer.hpp"
#include "cipher/scrypt/crypto_scrypt.hpp"
#include "teasafe/detail/DetailTeaSafe.hpp"
#include "cryptopp/aes.h"
#include "cryptopp/ccm.h"

namespace teasafe { namespace cipher
{

    static uint8_t key[16];
    static std::vector<uint8_t> bigSalt;

      // used for holding the hash-generated key


    AESByteTransformer::AESByteTransformer(std::string const &password,
                                           uint64_t const iv)
      : IByteTransformer()
      , m_password(password)
      , m_iv(iv)
    {

    }

    void
    AESByteTransformer::init()
    {
        //
        // The following key generation algorithm uses scrypt, with N = 2^20; r = 8; p = 1
        //
        if (!IByteTransformer::m_init) {
            broadcastEvent(EventType::KeyGenBegin);
            uint8_t salt[8];
            teasafe::detail::convertUInt64ToInt8Array(m_iv, salt);
            ::crypto_scrypt((uint8_t*)m_password.c_str(), m_password.size(), salt, 8,
                            1048576, 8, 1, key, 16);
            broadcastEvent(EventType::KeyGenEnd);
            IByteTransformer::m_init = true;

            bigSalt.resize(16);
            int c =0;
            for(; c < 8; ++c) {
              bigSalt[c] = salt[c];
            }
            for(; c < 16; ++c) {
              bigSalt[c] = salt[c-8];
            }
        }
    }

    AESByteTransformer::~AESByteTransformer()
    {

    }

    void 
    AESByteTransformer::doEncrypt(char *in, char *out, std::ios_base::streamoff startPosition, long length) const
    {
        CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption aesEncryptor;
        aesEncryptor.SetKeyWithIV(key, sizeof(key), &bigSalt.front());
        aesEncryptor.Seek(startPosition);
        aesEncryptor.ProcessData((uint8_t*)out, (uint8_t*)in, length);
    }

    void 
    AESByteTransformer::doDecrypt(char *in, char *out, std::ios_base::streamoff startPosition, long length) const
    {
        CryptoPP::CTR_Mode<CryptoPP::AES>::Decryption aesDecryptor;
        aesDecryptor.SetKeyWithIV(key, sizeof(key), &bigSalt.front());
        aesDecryptor.Seek(startPosition);
        aesDecryptor.ProcessData((uint8_t*)out, (uint8_t*)in, length);
    }
}
}