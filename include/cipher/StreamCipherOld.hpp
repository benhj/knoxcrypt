// will implement some variant of ARCFOUR

#ifndef BFS_CIPHER_STREAM_CIPHER_HPP__
#define BFS_CIPHER_STREAM_CIPHER_HPP__


#include <string>
#include <vector>
#include <cassert>
#include <iostream>

namespace bfs { namespace cipher
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


#endif // BFS_CIPHER_STREAM_CIPHER_HPP__
