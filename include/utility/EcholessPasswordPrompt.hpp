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


#ifndef TeaSafe_UTILITY_ECHOLESS_PASSWORD_PROMPT_HPP__
#define TeaSafe_UTILITY_ECHOLESS_PASSWORD_PROMPT_HPP__

#include <iostream>
#include <string>

#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

namespace teasafe
{

    namespace utility
    {

        /**
         * @brief retrieves an echoless password from the user
         * @note based on solution found here:
         * http://stackoverflow.com/questions/1196418/getting-a-password-in-c-without-using-getpass-3
         * @todo handle error conditions
         * @return the password
         */
        inline std::string getPassword(std::string const &prompt = "password: ")
        {
            // read password in
            struct termios oflags, nflags;
            char password[64];

            // disabling echo
            tcgetattr(fileno(stdin), &oflags);
            nflags = oflags;
            nflags.c_lflag &= ~ECHO;
            nflags.c_lflag |= ECHONL;

            if (tcsetattr(fileno(stdin), TCSANOW, &nflags) != 0) {
                perror("tcsetattr");
            }

            std::cout<<prompt;
            fgets(password, sizeof(password), stdin);
            password[strlen(password) - 1] = 0;

            // restore terminal
            if (tcsetattr(fileno(stdin), TCSANOW, &oflags) != 0) {
                perror("tcsetattr");
            }
            return std::string(password);
        }

    }
}

#endif // TeaSafe_UTILITY_ECHOLESS_PASSWORD_PROMPT_HPP__
