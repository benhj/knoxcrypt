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

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
*/


// will implement some variant of ARCFOUR

#ifndef TeaSafe_CIPHER_STREAM_CIPHER_HPP__
#define TeaSafe_CIPHER_STREAM_CIPHER_HPP__


#include <string>
#include <vector>
#include <cassert>
#include <iostream>

namespace teasafe { namespace cipher
{

    inline void swapints(long *array, long ndx1, long ndx2)
    {
        int temp = array[ndx1];
        array[ndx1] = array[ndx2];
        array[ndx2] = temp;
    }

    static long g_sbox[256];

    void initKeyAndSbox(std::string const &password, long key[256])
    {
        int ilen = password.length();
        for (int a=0; a < 256; ++a) {
            key[a] = password[a % ilen];
            g_sbox[a] = a;
        }
    }

    void setSBox(long key[256])
    {
        for (int a=0, b=0; a < 256; a++) {
            b = (b + g_sbox[a] + key[a]) % 256;
            swapints(g_sbox, a, b);
        }
    }


    class StreamCipher
    {
      public:
        StreamCipher(std::string const &password)
            : m_password(password)
        {
            initKeyAndSbox(password);
            setSBox();
        }

        void transform(char *in, char *out,
                       std::ios_base::streamoff startPosition,
                       long length)
        {
            /*
              long sbox[256];
              for(int i = 0;i<256;++i){
              sbox[i]=m_sbox[i];
              }

              int i = 0, j = 0, k = 0;
              for (int a=0; a < startPosition; ++a) {
              i = (i + 1) % 256;
              j = (j + sbox[i]) % 256;
              swapints(sbox, i, j);
              }



              for (int a=0; a < length; ++a) {
              i = (i + 1) % 256;
              j = (j + sbox[i]) % 256;
              swapints(sbox, i, j);
              k = sbox[(sbox[i] + sbox[j]) % 256];
              out[a] = in[a] ^ k;
              }
            */
            /*
              long sbox[256];
              for(int i = 0;i<256;++i){
              sbox[i] = m_sbox[i];
              }
              long i = startPosition, j = startPosition, k = 0;
              for (int a=0; a < length; ++a) {
              i = ((startPosition) + (a+1)) % 256;
              j = (j + sbox[i]) % 256;
              swapints(sbox, i, j);
              k = sbox[(sbox[i] + sbox[j]) % 256];
              out[a] = in[a] ^ k;
              }*/

            //                long i = startPosition % 256;
            //              long j = (startPosition - 1 + sbox[i]) % 256;

            //std::cout<<"c"<<std::endl;

            //--startPosition;
            /*
              long sbox[256];
              for(int i = 0;i<256;++i){
              sbox[i] = m_sbox[i];
              }*/
            long j = 0;
            for (long a=0; a < length; a++) {
                long i = ((startPosition) + (a+1)) % 256;
                long j = ((startPosition) + (a+1) + m_sbox[i]) % 256;
                long k = m_sbox[(m_sbox[i] + m_sbox[j]) % 256];
                out[a] = in[a] ^ k;
            }
        }

      private:
        std::string m_password;
        long m_sbox[256];
        long m_key[256];

        void initKeyAndSbox(std::string const &password)
        {
            int ilen = password.length();
            for (int a=0; a < 256; ++a) {
                m_key[a] = password[a % ilen];
                m_sbox[a] = a;
            }
        }

        void setSBox()
        {
            for (int a=0, b=0; a < 256; a++) {
                b = (b + m_sbox[a] + m_key[a]) % 256;
                swapints(m_sbox, a, b);
            }
        }
    };

}
}


#endif // TeaSafe_CIPHER_STREAM_CIPHER_HPP__
