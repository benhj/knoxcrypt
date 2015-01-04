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

// a basic stream transformation algorithm *very loosely* based on ARCFOUR

#pragma once

#include "cipher/IByteTransformer.hpp"

namespace teasafe { namespace cipher
{

    static long g_sbox[256];
    static std::string g_password;

    inline void swapints(long *array, long ndx1, long ndx2)
    {
        int temp = array[ndx1];
        array[ndx1] = array[ndx2];
        array[ndx2] = temp;
    }

    inline void initKeyAndSbox(long key[256])
    {
        int ilen = g_password.length();
        for (int a=0; a < 256; ++a) {
            key[a] = g_password[a % ilen];
            g_sbox[a] = a;
        }
    }

    inline void setSBox(long key[256])
    {
        for (int a=0, b=0; a < 256; a++) {
            b = (b + g_sbox[a] + key[a]) % 256;
            swapints(g_sbox, a, b);
        }
    }

    class BasicByteTransformer : public IByteTransformer
    {
      public:
        explicit BasicByteTransformer(std::string const &password)
            : IByteTransformer(password, uint64_t(12345) /*not used*/)
        {
            // we statically initialize as an optimization
            static bool init = false;
            if (!init) {
                g_password = password;
                initKeyAndSbox(m_key);
                setSBox(m_key);
                init = true;
            }
        }

        ~BasicByteTransformer()
        {

        }

      private:

        BasicByteTransformer(); // not required

        void doEncrypt(char *in, char *out, std::ios_base::streamoff startPosition, long length) const
        {
          this->doTransform(in, out, startPosition, length);
        }

        void doDecrypt(char *in, char *out, std::ios_base::streamoff startPosition, long length) const
        {
          this->doTransform(in, out, startPosition, length);
        }

        void doTransform(char *in, char *out, std::ios_base::streamoff startPosition, long length) const
        {
            long j = 0;
            for (long a=0; a < length; a++) {
                long i = ((startPosition) + (a+1)) % 256;
                long j = ((startPosition) + (a+1) + g_sbox[i]) % 256;
                long k = g_sbox[(g_sbox[i] + g_sbox[j]) % 256];
                out[a] = in[a] ^ k;
            }
        }

      private:
        long m_key[256];
    };

}
}

