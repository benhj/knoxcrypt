#ifndef BFS_FILE_BLOCK_EXCEPTION_HPP__
#define BFS_FILE_BLOCK_EXCEPTION_HPP__

#include <exception>

namespace bfs
{

    enum class FileBlockError { NotReadable, NotWritable };

    class FileBlockException : public std::exception
    {
        public:
            FileBlockException(FileBlockError const &error)
                : m_error(error)
            {

            }

            virtual const char* what() const throw()
            {
                if(m_error == FileBlockError::NotReadable) {
                    return "File block not readable";
                }

                if(m_error == FileBlockError::NotWritable) {
                    return "File block not writable";
                }

                return "File block unknown error";
            }

            bool operator==(FileBlockException const &other) const
            {
                return m_error == other.m_error;
            }

        private:
            FileBlockError m_error;
    };
}


#endif // BFS_FILE_BLOCK_EXCEPTION_HPP__
