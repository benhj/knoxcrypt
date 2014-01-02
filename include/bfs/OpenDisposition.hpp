/*
  The MIT License (MIT)

  Copyright (c) 2013 Ben H.D. Jones

  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
  the Software, and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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
