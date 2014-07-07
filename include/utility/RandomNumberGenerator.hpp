/*
  Copyright (c) <2014>, <BenHJ>
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

/// For generating a cryptographically secure random number

#ifndef TeaSafe_UTILITY_RANDOM_NUMBER_GENERATOR_HPP__
#define TeaSafe_UTILITY_RANDOM_NUMBER_GENERATOR_HPP__

#include <boost/bind.hpp>
#include <boost/random.hpp>
#include <boost/nondet_random.hpp>

namespace teasafe
{

    namespace utility
    {

        /**
         * @brief  allegedly generates a secure 64 bit random number
         * @return a 64 bit random number
         */
        uint64_t random()
        {
            boost::random_device rd;
            boost::random::uniform_int_distribution<uint64_t> dis;
            boost::function<uint64_t()> gen = boost::bind(dis, boost::ref(rd));
            return gen();
        }

    }

}

#endif // TeaSafe_UTILITY_RANDOM_NUMBER_GENERATOR_HPP__
