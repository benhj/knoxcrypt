/*
  Copyright (c) <2013-2014>, <BenHJ>
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
  3. Neither the name of the copyright holder nor the names of its contributors
  may be used to endorse or promote products derived from this software without
  specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TeaSafe_CIPHER_I_BYTE_TRANSFORMER_HPP__
#define TeaSafe_CIPHER_I_BYTE_TRANSFORMER_HPP__

// Crappy clang shipped with mac -- at least the version I have -- doesn't support
// std::move and std::forward
#if __APPLE__ && (__GNUC_LIBSTD__ <= 4) && (__GNUC_LIBSTD_MINOR__ <= 2)
#  define BOOST_NO_CXX11_RVALUE_REFERENCES
#endif

#include "utility/EventType.hpp"
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>
#include <iostream>
#include <string>

namespace teasafe { namespace cipher
{
    class IByteTransformer
    {
      public:
        IByteTransformer();

        /// must be implemented and called before anything else!!
        virtual void init() = 0;

        void transform(char *in, char *out, std::ios_base::streamoff startPosition, long length);

        /// to notify when different parts of the cipher have been initialized
        /// also see EventType for different event types
        virtual void registerSignalHandler(boost::function<void(EventType)> const &f);
        virtual ~IByteTransformer();
      private:
        virtual void doTransform(char *in, char *out, std::ios_base::streamoff startPosition, long length) const = 0;

      protected:
        // for emitting when something is done (e.g.
        // start of key generation)
        typedef boost::signals2::signal<void(EventType)> CipherSignal;
        typedef boost::shared_ptr<CipherSignal> SharedSignal;
        SharedSignal m_cipherSignal;

        void broadcastEvent(EventType const &event);

    };
}
}

#endif // TeaSafe_CIPHER_I_TRANSFORMER_HPP__
