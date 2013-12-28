// will implement some variant of ARCFOUR

#ifndef BFS_CIPHER_STREAM_CIPHER_HPP__
#define BFS_CIPHER_STREAM_CIPHER_HPP__


#include <string>
#include <vector>
#include <cassert>
#include <iostream>

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

    class StreamCipher
    {
        public:
            StreamCipher(std::string const &password)
                : m_password(password)
            {
                static bool init = false;
                if(!init) {
                    g_password = password;
                    initKeyAndSbox(m_key);
                    setSBox(m_key);
                    init = true;
                }
            }

            void transform(char *in, char *out,
                           std::ios_base::streamoff startPosition,
                           long length)
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
            std::string m_password;
            long m_key[256];
    };

}
}


#endif // BFS_CIPHER_STREAM_CIPHER_HPP__
