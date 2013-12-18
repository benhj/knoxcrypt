#ifndef BFS_OPEN_DISPOSITION_HPP__
#define BFS_OPEN_DISPOSITION_HPP__

#include "bfs/AppendOrOverwrite.hpp"
#include "bfs/CreateOrDontCreate.hpp"
#include "bfs/ReadOrWriteOrBoth.hpp"
#include "bfs/TruncateOrKeep.hpp"

namespace bfs
{
    class OpenDisposition
    {
        public:
            OpenDisposition(ReadOrWriteOrBoth const &readWrite,
                            AppendOrOverwrite const &append,
                            CreateOrDontCreate const &create,
                            TruncateOrKeep const &trunc)
                : m_readWrite(readWrite)
                , m_append(append)
                , m_create(create)
                , m_trunc(trunc)
            {
            }

            static OpenDisposition buildAppendDisposition()
            {
                return OpenDisposition(ReadOrWriteOrBoth::ReadWrite,
                                       AppendOrOverwrite::Append,
                                       CreateOrDontCreate::Create,
                                       TruncateOrKeep::Keep);
            }

            static OpenDisposition buildReadOnlyDisposition()
            {
                // note it only matters that the first paramter is readonly
                // the others don't matter and can be anything
                return OpenDisposition(ReadOrWriteOrBoth::ReadOnly,
                                       AppendOrOverwrite::Append,
                                       CreateOrDontCreate::DontCreate,
                                       TruncateOrKeep::Keep);
            }

            static OpenDisposition buildWriteOnlyDisposition()
            {
                return OpenDisposition(ReadOrWriteOrBoth::WriteOnly,
                                       AppendOrOverwrite::Append,
                                       CreateOrDontCreate::DontCreate,
                                       TruncateOrKeep::Keep);
            }

            static OpenDisposition buildOverwriteDisposition()
            {
                return OpenDisposition(ReadOrWriteOrBoth::ReadWrite,
                                       AppendOrOverwrite::Overwrite,
                                       CreateOrDontCreate::DontCreate,
                                       TruncateOrKeep::Keep);
            }

            ReadOrWriteOrBoth readWrite() const
            {
                return m_readWrite;
            }

            AppendOrOverwrite append() const
            {
                return m_append;
            }

            CreateOrDontCreate create() const
            {
                return m_create;
            }

            TruncateOrKeep trunc() const
            {
                return m_trunc;
            }

        private:
            ReadOrWriteOrBoth m_readWrite;
            AppendOrOverwrite m_append;
            CreateOrDontCreate m_create;
            TruncateOrKeep m_trunc;

    };
}


#endif // OPEN_DISPOSITION_HPP__
