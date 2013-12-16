#ifndef BFS_APPEND_OR_OVERWRITE_HPP__
#define BFS_APPEND_OR_OVERWRITE_HPP__

namespace bfs
{
    class AppendOrOverwrite
    {
      private:
        typedef enum {APPEND, OVERWRITE} AppendOrOverwriteEnum;
        AppendOrOverwriteEnum m_Value;

        AppendOrOverwrite();
      public:

        static const AppendOrOverwriteEnum Append;
        static const AppendOrOverwriteEnum Overwrite;

        AppendOrOverwrite(AppendOrOverwriteEnum const value) : m_Value(value) {}

        bool operator==(AppendOrOverwrite const& rhs) const { return m_Value == rhs.m_Value; }
        bool operator!=(AppendOrOverwrite const& rhs) const { return m_Value != rhs.m_Value; }
    };
}


#endif // BFS_APPEND_OR_OVERWRITE_HPP__
