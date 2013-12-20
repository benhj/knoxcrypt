#ifndef BFS_BFS_EXCEPTION_HPP__
#define BFS_BFS_EXCEPTION_HPP__

#include <exception>

namespace bfs
{

    enum class BFSError { NotFound, AlreadyExists, IllegalFilename, FolderNotEmpty };

    class BFSException : public std::exception
    {
        public:
            BFSException(BFSError const &error)
                : m_error(error)
            {

            }

            virtual const char* what() const throw()
            {
                if(m_error == BFSError::NotFound) {
                    return "BFS: Entry not found";
                }

                if(m_error == BFSError::AlreadyExists) {
                    return "BFS: Entry already exists";
                }

                if(m_error == BFSError::IllegalFilename) {
                    return "BFS: illegal filename";
                }

                if(m_error == BFSError::FolderNotEmpty) {
                    return "BFS: Folder not empty";
                }

                return "BFS: Unknown error";
            }

            bool operator==(BFSException const &other) const
            {
                return m_error == other.m_error;
            }

        private:
            BFSError m_error;
    };
}


#endif // BFS_BFS_EXCEPTION_HPP__
