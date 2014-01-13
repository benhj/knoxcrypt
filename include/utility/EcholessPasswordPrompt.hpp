#ifndef BFS_UTILITY_ECHOLESS_PASSWORD_PROMPT_HPP__
#define BFS_UTILITY_ECHOLESS_PASSWORD_PROMPT_HPP__

#include <iostream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

namespace bfs
{

    namespace utility {

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

#endif // BFS_UTILITY_ECHOLESS_PASSWORD_PROMPT_HPP__
