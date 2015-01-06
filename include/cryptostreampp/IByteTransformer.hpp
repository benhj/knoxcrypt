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

#include <iostream>
#include <memory>
#include <string>

namespace cryptostreampp
{
    class IByteTransformer
    {
      public:
        IByteTransformer(std::string const &password,
                         uint64_t const iv,
                         uint64_t const iv2,
                         uint64_t const iv3,
                         uint64_t const iv4);

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

        // password used to build the 256 bit key
        mutable std::string m_password;

        // multiple 64 bit iv blocks used to build 256 bit IV
        mutable uint64_t m_iv;
        mutable uint64_t m_iv2;
        mutable uint64_t m_iv3;
        mutable uint64_t m_iv4;

        /// build the key using scrypt and big IV
        void generateKeyAndIV();
       
    };
}

