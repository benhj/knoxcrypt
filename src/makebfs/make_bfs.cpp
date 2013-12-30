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

#include "bfs/CoreBFSIO.hpp"
#include "bfs/MakeBFS.hpp"

#include <iostream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

int main(int argc, char *argv[])
{
    if(argc < 2) {
        std::cout<<"Insufficient number of arguments\n\nUsage:\n\nmake_bfs <blocks> <path>\n\n"<<std::endl;
        return 1;
    }

    int blocks = atoi(argv[1]);

    bfs::CoreBFSIO io;
    io.path = argv[2];
    io.blocks = blocks;

    // reading echoless password, based on solution here:
    // http://stackoverflow.com/questions/1196418/getting-a-password-in-c-without-using-getpass-3
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
      return EXIT_FAILURE;
    }

    printf("password: ");
    fgets(password, sizeof(password), stdin);
    password[strlen(password) - 1] = 0;

    // restore terminal
    if (tcsetattr(fileno(stdin), TCSANOW, &oflags) != 0) {
      perror("tcsetattr");
      return EXIT_FAILURE;
    }

    io.password.append(password);

    bfs::MakeBFS bfs(io);

    return 0;
}
