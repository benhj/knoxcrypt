#ifndef BFS_ENTRY_TYPE_HPP__
#define BFS_ENTRY_TYPE_HPP__

namespace bfs
{
    class EntryType
    {
      private:
        typedef enum {FILE_TYPE, FOLDER_TYPE} EntryTypeEnum;
        EntryTypeEnum m_Value;

        EntryType();
      public:

        static const EntryTypeEnum FileType;
        static const EntryTypeEnum FolderType;

        EntryType(EntryTypeEnum const value) : m_Value(value) {}

        bool operator==(EntryType const& rhs) const { return m_Value == rhs.m_Value; }
        bool operator!=(EntryType const& rhs) const { return m_Value != rhs.m_Value; }
    };
}


#endif // BFS_ENTRY_TYPE_HPP__
