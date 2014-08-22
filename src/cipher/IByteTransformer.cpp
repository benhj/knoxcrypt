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

#include "cipher/IByteTransformer.hpp"
#include "teasafe/detail/DetailTeaSafe.hpp"
#include "cipher/scrypt/crypto_scrypt.hpp"

#include <boost/make_shared.hpp>

namespace teasafe { namespace cipher
{

    bool IByteTransformer::m_init = false;

    uint8_t IByteTransformer::g_bigKey[32]; // the 256 bit key to be used
    uint8_t IByteTransformer::g_bigIV[32];  // for storing a 256 bit IV

    IByteTransformer::IByteTransformer(std::string const &password,
                                       uint64_t const iv,
                                       uint64_t const iv2,
                                       uint64_t const iv3,
                                       uint64_t const iv4)
      : m_password(password)
      , m_iv(iv)
      , m_iv2(iv2)
      , m_iv3(iv3)
      , m_iv4(iv4)
      , m_cipherSignal(boost::make_shared<CipherSignal>())
    {
    }

    IByteTransformer::~IByteTransformer()
    {

    }

    void 
    IByteTransformer::generateKeyAndIV()
    {
        //
        // The following g_bigKey generation algorithm uses scrypt, with N = 2^20; r = 8; p = 1
        //
        if (!m_init) {
            broadcastEvent(EventType::KeyGenBegin);

            // create a 256 bit IV out of 4 individual 64 bit IVs
            uint8_t salt[8];
            uint8_t saltB[8];
            uint8_t saltC[8];
            uint8_t saltD[8];
            teasafe::detail::convertUInt64ToInt8Array(m_iv, salt);
            teasafe::detail::convertUInt64ToInt8Array(m_iv2, saltB);
            teasafe::detail::convertUInt64ToInt8Array(m_iv3, saltC);
            teasafe::detail::convertUInt64ToInt8Array(m_iv4, saltD);

            // initialize the 256 bit g_bigKey from IV1 salt
            ::crypto_scrypt((uint8_t*)m_password.c_str(), m_password.size(), salt, 8,
                            1048576, 8, 1, g_bigKey, 32);

            broadcastEvent(EventType::KeyGenEnd);
            IByteTransformer::m_init = true;

            // construct the big IV
            int c =0;
            for(; c < 8; ++c) {
              g_bigIV[c] = salt[c];
            }
            for(; c < 16; ++c) {
              g_bigIV[c] = saltB[c-8];
            }
            for(; c < 24; ++c) {
              g_bigIV[c] = saltC[c-16];
            }
            for(; c < 32; ++c) {
              g_bigIV[c] = saltD[c-24];
            }
            m_init = true;
        }
    }

    void
    IByteTransformer::registerSignalHandler(boost::function<void(EventType)> const &f)
    {
        m_cipherSignal->connect(f);
    }

    void
    IByteTransformer::broadcastEvent(EventType const &event)
    {
        (*m_cipherSignal)(event);
    }

    void
    IByteTransformer::encrypt(char *in, char *out, std::ios_base::streamoff startPosition, long length)
    {
        this->doEncrypt(in, out, startPosition, length);
    }

        void
    IByteTransformer::decrypt(char *in, char *out, std::ios_base::streamoff startPosition, long length)
    {
        this->doDecrypt(in, out, startPosition, length);
    }
}
}
