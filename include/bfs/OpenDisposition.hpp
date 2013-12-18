#ifndef BFS_OPEN_DISPOSITION_HPP__
#define BFS_OPEN_DISPOSITION_HPP__

#include "bfs/AppendOrOverwrite.hpp"
#include "bfs/CreateOrDontCreate.hpp"
#include "bfs/TruncateOrKeep.hpp"

namespace bfs
{
    class OpenDisposition
    {
        public:
            OpenDisposition(AppendOrOverwrite const &append,
                            CreateOrDontCreate const &create,
                            TruncateOrKeep const &trunc)
                : m_append(append)
                , m_create(create)
                , m_trunc(trunc)
            {
            }

            static OpenDisposition buildAppendDisposition()
            {
                return OpenDisposition(AppendOrOverwrite::Append,
                                       CreateOrDontCreate::Create,
                                       TruncateOrKeep::Keep);
            }

            static OpenDisposition buildReadOnlyDisposition()
            {
                return OpenDisposition(AppendOrOverwrite::Append,
                                       CreateOrDontCreate::Create,
                                       TruncateOrKeep::Keep);
            }

            static OpenDisposition buildOverwriteDisposition()
            {
                return OpenDisposition(AppendOrOverwrite::Overwrite,
                                       CreateOrDontCreate::DontCreate,
                                       TruncateOrKeep::Keep);
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
            AppendOrOverwrite m_append;
            CreateOrDontCreate m_create;
            TruncateOrKeep m_trunc;

    };
}


#endif // OPEN_DISPOSITION_HPP__
