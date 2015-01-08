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

#include "EncryptionProperties.hpp"

#include "cryptopp/pwdbased.h"
#include "cryptopp/sha.h"

#include <algorithm>
#include <iterator>
#include <cstdint>
#include <ios>
#include <string>

namespace cryptostreampp
{
    class IByteTransformer
    {
      public:
        IByteTransformer(EncryptionProperties const &encProps);

        /// must be implemented and called before anything else!!
        virtual void init() = 0;

        void encrypt(char *in, char *out, std::ios_base::streamoff startPosition, long length);
        void decrypt(char *in, char *out, std::ios_base::streamoff startPosition, long length);

        virtual ~IByteTransformer();

        /// has key and IV been initialized yet?
        static bool m_init; 

        /// 256 bit encryption / decryption key
        static uint8_t g_bigKey[32]; 

        /// 256 bit IV
        static uint8_t g_bigIV[32];  

      private:
        virtual void doEncrypt(char *in, char *out, std::ios_base::streamoff startPosition, long length) const = 0;
        virtual void doDecrypt(char *in, char *out, std::ios_base::streamoff startPosition, long length) const = 0;

      protected:

        /// stores password and iv all used to derive key
        EncryptionProperties m_props;

        /// build the key using scrypt and big IV
        void generateKeyAndIV();
    };

    namespace {
        inline void convertUInt64ToInt8Array(uint64_t const bigNum, uint8_t array[8])
        {
            array[0] = static_cast<uint8_t>((bigNum >> 56) & 0xFF);
            array[1] = static_cast<uint8_t>((bigNum >> 48) & 0xFF);
            array[2] = static_cast<uint8_t>((bigNum >> 40) & 0xFF);
            array[3] = static_cast<uint8_t>((bigNum >> 32) & 0xFF);
            array[4] = static_cast<uint8_t>((bigNum >> 24) & 0xFF);
            array[5] = static_cast<uint8_t>((bigNum >> 16) & 0xFF);
            array[6] = static_cast<uint8_t>((bigNum >> 8) & 0xFF);
            array[7] = static_cast<uint8_t>((bigNum) & 0xFF);
        }
    }

    inline
    IByteTransformer::IByteTransformer(EncryptionProperties const &encProps)
      : m_props(encProps)
    {
    }

    inline
    IByteTransformer::~IByteTransformer()
    {

    }

    inline
    void 
    IByteTransformer::generateKeyAndIV()
    {
        //
        // The following g_bigKey generation algorithm uses scrypt, with N = 2^20; r = 8; p = 1
        //
        if (!m_init) {

            // create a 256 bit IV out of 4 individual 64 bit IVs
            uint8_t salt[8];
            uint8_t saltB[8];
            uint8_t saltC[8];
            uint8_t saltD[8];
            convertUInt64ToInt8Array(m_props.iv, salt);
            convertUInt64ToInt8Array(m_props.iv2, saltB);
            convertUInt64ToInt8Array(m_props.iv3, saltC);
            convertUInt64ToInt8Array(m_props.iv4, saltD);


            // construct the big IV
            (void)std::copy(salt,  salt + 8 , g_bigIV);
            (void)std::copy(saltB, saltB + 8, g_bigIV + 8);
            (void)std::copy(saltC, saltC + 8, g_bigIV + 16);
            (void)std::copy(saltD, saltD + 8, g_bigIV + 24);

            // derive the key using a million iterations
            CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA512> pbkdf2;
            pbkdf2.DeriveKey(g_bigKey, sizeof(g_bigKey), 0, 
                             (const unsigned char*)&m_props.password.front(), m_props.password.length(), 
                             g_bigIV, sizeof(g_bigIV), 
                             1000000);

            IByteTransformer::m_init = true;
        }
    }

    inline
    void
    IByteTransformer::encrypt(char *in, char *out, std::ios_base::streamoff startPosition, long length)
    {
        this->doEncrypt(in, out, startPosition, length);
    }

    inline
    void
    IByteTransformer::decrypt(char *in, char *out, std::ios_base::streamoff startPosition, long length)
    {
        this->doDecrypt(in, out, startPosition, length);
    }
}

