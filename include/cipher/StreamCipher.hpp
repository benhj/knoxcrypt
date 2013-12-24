// will implement some variant of ARCFOUR

#ifndef BFS_CIPHER_STREAM_CIPHER_HPP__
#define BFS_CIPHER_STREAM_CIPHER_HPP__

#include <string>
#include <vector>

namespace bfs { namespace cipher
{

    inline void swapints(int *array, int ndx1, int ndx2)
    {
        int temp = array[ndx1];
        array[ndx1] = array[ndx2];
        array[ndx2] = temp;
    }

    class StreamCipher
    {
        public:
            StreamCipher(std::string const &password)
            {
                initKeyAndSbox(password);
                setSBox();
            }

            void transform(std::vector<char> &in, std::vector<char> &out,
                           int startPosition,
                           int length)
            {
                std::vector<int> sbox;
                sbox.assign(m_sbox, m_sbox + 256);
                int i = 0, j = 0, k = 0;
                for (int a=0; a < startPosition; a++) {
                    i = (i + 1) % 256;
                    j = (j + sbox[i]) % 256;
                    swapints(&sbox.front(), i, j);
                    k = sbox[(sbox[i] + sbox[j]) % 256];
                }
                for (int a=0; a < length; a++) {
                    i = (i + 1) % 256;
                    j = (j + sbox[i]) % 256;
                    swapints(&sbox.front(), i, j);
                    k = sbox[(sbox[i] + sbox[j]) % 256];
                    out[a] = in[a] ^ k;
                }
            }

        private:
            int m_sbox[256];
            int m_key[256];

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
