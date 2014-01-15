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

// an implementation of XTEA

#ifndef BFS_CIPHER_XTEA_BYTE_TRANSFORMER_HPP__
#define BFS_CIPHER_XTEA_BYTE_TRANSFORMER_HPP__

#include "cipher/IByteTransformer.hpp"

namespace bfs { namespace cipher
{

    namespace detail
    {
        // the xtea encipher algorithm as found on wikipedia
        void encipher(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4])
        {
            unsigned int i;
            uint32_t v0=v[0], v1=v[1], sum=0, delta=0x9E3779B9;
            for (i=0; i < num_rounds; i++) {
                v0 += ((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + key[sum & 3]);
                sum += delta;
                v1 += ((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + key[(sum>>11) & 3]);
            }
            v[0]=v0; v[1]=v1;
        }

        // the xtea encipher algorithm as found on wikipedia
        void decipher(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4])
        {
            unsigned int i;
            uint32_t v0=v[0], v1=v[1], delta=0x9E3779B9, sum=delta*num_rounds;
            for (i=0; i < num_rounds; i++) {
                v1 -= ((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + key[(sum>>11) & 3]);
                sum -= delta;
                v0 -= ((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + key[sum & 3]);
            }
            v[0]=v0; v[1]=v1;
        }

        // helper code found here:
        // http://codereview.stackexchange.com/questions/2050/codereview-tiny-encryption-algorithm-for-arbitrary-sized-data
        void convertBytesAndEncipher(unsigned int num_rounds, unsigned char * buffer, uint32_t const key[4])
        {
            uint32_t datablock[2];

            datablock[0] = (buffer[0] << 24) | (buffer[1] << 16)  | (buffer[2] << 8) | (buffer[3]);
            datablock[1] = (buffer[4] << 24) | (buffer[5] << 16)  | (buffer[6] << 8) | (buffer[7]);

            encipher(num_rounds, datablock, key);

            buffer[0] = static_cast<unsigned char>((datablock[0] >> 24) & 0xFF);
            buffer[1] = static_cast<unsigned char>((datablock[0] >> 16) & 0xFF);
            buffer[2] = static_cast<unsigned char>((datablock[0] >> 8) & 0xFF);
            buffer[3] = static_cast<unsigned char>((datablock[0]) & 0xFF);
            buffer[4] = static_cast<unsigned char>((datablock[1] >> 24) & 0xFF);
            buffer[5] = static_cast<unsigned char>((datablock[1] >> 16) & 0xFF);
            buffer[6] = static_cast<unsigned char>((datablock[1] >> 8) & 0xFF);
            buffer[7] = static_cast<unsigned char>((datablock[1]) & 0xFF);
        }
    }


    class XTEAByteTransformer : public IByteTransformer
    {
      public:
        explicit XTEAByteTransformer(std::string const &password)
            : IByteTransformer(password)
        {

        }

        ~XTEAByteTransformer()
        {

        }

      private:

        XTEAByteTransformer(); // not required

        void doTransform(char *in, char *out, std::ios_base::streamoff startPosition, long length,
                         bool encrypt) const
        {

        }

      private:
        long m_key[256];
    };

}
}


#endif // BFS_CIPHER_BASIC_TRANSFORMER_CIPHER_HPP__
