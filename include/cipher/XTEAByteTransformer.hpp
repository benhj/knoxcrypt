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
*/

// an implementation of XTEA

#ifndef TeaSafe_CIPHER_XTEA_BYTE_TRANSFORMER_HPP__
#define TeaSafe_CIPHER_XTEA_BYTE_TRANSFORMER_HPP__

#include "cipher/IByteTransformer.hpp"

#include <openssl/sha.h>
#include <cstring>

#include <iostream>
#include <vector>

namespace teasafe { namespace cipher
{

    namespace detail
    {
        // the xtea encipher algorithm as found on wikipedia. Use this to encrypt
        // a sequence of numbers. The original plain-text is then xor-ed with this
        // sequence
        void encipher(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4])
        {
            unsigned int i;
            uint32_t v0=v[0], v1=v[1], sum=0, delta=0x9E3779B9;
            for (i=0; i < num_rounds; i++) {
                v0 += ((v1 << 4 ^ v1 >> 5) + v1) ^(sum + key[sum & 3]);
                sum += delta;
                v1 += ((v0 << 4 ^ v0 >> 5) + v0) ^(sum + key[(sum>>11) & 3]);
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


    // prob a bad idea all these static vars; still for the sake of optimizing.. :-)
    static bool g_init = false;

    // used for holding the hash-generated key
    static uint32_t g_key[4];

    // used to optimize crypto process. This will store
    // the first 256MB of cipher stream
    static std::vector<char> g_bigCipherBuffer;

    // the size of the cipher buffer (prefer a #define rather than a const
    // for minimal memory footprint and minimal time required to instantiate).
#define CIPHER_BUFFER_SIZE 268435456

    class XTEAByteTransformer : public IByteTransformer
    {
      public:
        explicit XTEAByteTransformer(std::string const &password)
            : IByteTransformer(password)
        {
            // is this key-gen secure? probably not...use at own risk
            // initialize the key by taking a hash of the password and then creating
            // a uint32_t array out of it
            if (!g_init) {
                unsigned char temp[32];
                SHA256((unsigned char*)password.c_str(), password.size(), temp);
                int c = 0;
                for (int i = 0; i < 16; i += 4) {
                    unsigned char buf[4];
                    buf[0] = temp[i];
                    buf[1] = temp[i + 1];
                    buf[2] = temp[i + 2];
                    buf[3] = temp[i + 3];
                    g_key[c] = (buf[0] << 24) | (buf[1] << 16)  | (buf[2] << 8) | (buf[3]);
                    ++c;
                }
                //buildBigCipherBuffer();
                g_init = true;
            }

        }

        ~XTEAByteTransformer()
        {

        }

      private:

        XTEAByteTransformer(); // not required

        void buildBigCipherBuffer()
        {
            std::cout<<"Building big xtea cipher stream buffer. Please wait..."<<std::endl;
            std::vector<char> in;
            in.resize(CIPHER_BUFFER_SIZE);
            g_bigCipherBuffer.resize(CIPHER_BUFFER_SIZE);
            doTransform(&in.front(), &g_bigCipherBuffer.front(), 0, CIPHER_BUFFER_SIZE);
            std::cout<<"Built big xtea cipher stream buffer."<<std::endl;
        }

        void doTransform(char *in, char *out, std::ios_base::streamoff startPosition, long length) const
        {
            // big cipher buffer has been initialized
            /*
            if (g_init) {
                // prefer to use cipher buffer
                if ((startPosition + length) < CIPHER_BUFFER_SIZE) {
                    for (long j = 0; j < length; ++j) {
                        out[j] = in[j] ^ g_bigCipherBuffer[j + startPosition];
                    }
                    return;
                }
            }*/

            // how many blocks required? defaults to 1, if length greater
            // than 8 bytes then more blocks are needed
            long blocksRequired = 0;

            std::ios_base::streamoff const startPositionOffset = startPosition % 8;

            if (length > 8) {
                long remainder = length % 8;
                long roundedDown = length - remainder;
                blocksRequired += (roundedDown / 8);
                long c = 0;

                doSubTransformations(in,                         // input buffer
                                     out,                        // output buffer
                                     startPosition,              // seeked-to position
                                     startPositionOffset,        // 8-byte block offset
                                     blocksRequired,             // number of iterations
                                     c,                          // stream position
                                     8 + startPositionOffset);   // upper bound of xor loop

                if (remainder > 0) {
                    doSubTransformations(in,
                                         out,
                                         startPosition,
                                         startPositionOffset,
                                         1, // do remaining bytes once
                                         c,
                                         remainder + startPositionOffset);
                }

                return;
            }

            long c = 0;
            doSubTransformations(in,
                                 out,
                                 startPosition,
                                 startPositionOffset,
                                 1, // only need to do once
                                 c,
                                 length + startPositionOffset);
        }

        void doSubTransformations(char *in,
                                  char *out,
                                  std::ios_base::streamoff const startPosition,
                                  std::ios_base::streamoff const startPositionOffset,
                                  long const blocksOfSize8BeingTransformed,
                                  long &c,
                                  int const uptoBit) const
        {
            // the number of rounds is an XTEA thing. Reducing this number speeds
            // things up but reduces the encryption strength. Note that the literature
            // suggests a minimum of 64 rounds
            // I wonder if we should have a large static cipher stream buffer that gets
            // initialized to the most commonly used part of the stream (i.e., the
            // start of the stream, say the first 256MB) when the class is first constructed;
            // then, the cipher stream to xor the plain text against can be pulled from
            // this 256MB buffer rather than generated on the fly as is currently done.
            int rounds = 32;
            uint8_t a[8];
            uint8_t b[8];
            for (long i = 0; i < blocksOfSize8BeingTransformed; ++i) {
                uint8_t cipherStream[16];
                for (int j = 0; j < 16; ++j) {
                    cipherStream[j] = startPosition + c - startPositionOffset;
                    ++c;
                }
                c -= 16;

                // optimization
                if (i == 0) {
                    memcpy(&a, cipherStream, 8 * sizeof(uint8_t));
                } else {
                    memcpy(&a, b, 8 * sizeof(uint8_t));
                }
                memcpy(&b, cipherStream + 8, 8 * sizeof(uint8_t));

                // part of above optimization
                if (i == 0) {
                    detail::convertBytesAndEncipher(rounds, a, g_key);
                }
                detail::convertBytesAndEncipher(rounds, b, g_key);

                // now xor plain with key stream
                int k = 0;
                for (int j = startPositionOffset; j < uptoBit; ++j) {
                    if (j >= 8) {
                        out[c] = in[c] ^ b[k];
                        ++k;
                    } else {
                        out[c] = in[c] ^ a[j];
                    }
                    ++c;
                }
            }
        }


    };
}
}


#endif // TeaSafe_CIPHER_XTEA_BYTE_TRANSFORMER_HPP__
