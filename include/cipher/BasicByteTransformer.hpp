/*
  The MIT License (MIT)

  Copyright (c) 2013 Ben H.D. Jones

  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
  the Software, and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// a basic stream transformation algorithm *very loosely* based on ARCFOUR

#ifndef BFS_CIPHER_BASIC_BYTE_TRANSFORMER_HPP__
#define BFS_CIPHER_BASIC_BYTE_TRANSFORMER_HPP__

#include "cipher/IByteTransformer.hpp"

namespace bfs { namespace cipher
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
            : IByteTransformer(password)
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


#endif // BFS_CIPHER_BASIC_TRANSFORMER_CIPHER_HPP__
