#ifndef BFS_FILE_ENTRY_EXCEPTION_HPP__
#define BFS_FILE_ENTRY_EXCEPTION_HPP__

#include <exception>

namespace bfs
{

    enum class FileEntryError { NotReadable, NotWritable };

    class FileEntryException : public std::exception
    {
        public:
            FileEntryException(FileEntryError const &error)
                : m_error(error)
            {

            }

            virtual const char* what() const throw()
            {
                if(m_error == FileEntryError::NotReadable) {
                    return "File entry not readable";
                }

                if(m_error == FileEntryError::NotWritable) {
                    return "File entry not readable";
                }

                return "File entry unknown error";
            }

            bool operator==(FileEntryException const &other) const
            {
                return m_error == other.m_error;
            }

        private:
            FileEntryError m_error;
    };
}


#endif // BFS_FILE_ENTRY_EXCEPTION_HPP__
