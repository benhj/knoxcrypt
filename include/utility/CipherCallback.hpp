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


#ifndef TeaSafe_UTILITY_CIPHER_CALLBACK_HPP__
#define TeaSafe_UTILITY_CIPHER_CALLBACK_HPP__

#include "utility/EventType.hpp"
#include <boost/progress.hpp>

#include <memory>

namespace teasafe
{

    void cipherCallback(EventType eventType, long const amount)
    {
        static std::shared_ptr<boost::progress_display> pd;
        if(eventType == EventType::KeyGenBegin) {
            std::cout<<"Generating key...\n"<<std::endl;
        }
        if(eventType == EventType::KeyGenEnd) {
            std::cout<<"Key generated.\n"<<std::endl;
        }
        if(eventType == EventType::BigCipherBuildBegin) {
            std::cout<<"Building big xtea cipher stream buffer. Please wait..."<<std::endl;
            pd = std::make_shared<boost::progress_display>(amount);
        }
        if(eventType == EventType::BigCipherBuildEnd) {
            std::cout<<"\nBuilt big xtea cipher stream buffer.\n"<<std::endl;
        }
        if(eventType == EventType::CipherBuildUpdate) {
            ++(*pd);
        }
    }

}

#endif // TeaSafe_UTILITY_CIPHER_CALLBACK_HPP__
