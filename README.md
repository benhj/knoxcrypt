TeaSafe: An encrypted container format
--------------------------------------

##### What is it?

- an encrypted container format
- employs a very simple and custom developed filesystem (see wiki!)
- TeaSafe containers can be browsed using either of the provided shell or gui interfaces
- alternatively, you can implement your own. 
- can also use the provided FUSE-layer for more realistic filesystem interoperability
- utilizes XTEA, a publicly documented and easily understood cipher
- further utilizes the scrypt algorithm for key derivation

### Compiling

Note, only tested on Linux and Mac. With a bit of work, will probably build (sans fuse-bits) on windows
too.

Also note, requires some of the boost headers and libraries to build (see makefile).

`make` or `make all` will compile the following command-line binaries:

<pre>
test         : unit tests various parts of the main api
maketeasafe  : builds teasafe containers
teasafe      : fuse layer used for mounting teasafe containers
teashell     : shell utility used for accessing and modifying teasafe containers
</pre>

Examples:

<pre>
./maketeasafe ./test.bfs 128000
</pre>

Builds a 500MB (128000 x 4096 byte) container called `test.bfs` in the current folder

<pre>
./teasafe ./test.bfs /testMount
</pre>

Mount the container `./test.bfs` to `/testMount`

<pre>
./teashell ./test.bfs
</pre>

Runs the interactive shell on `test.bfs`

For more info, please post up on `https://groups.google.com/forum/#!forum/teasafe`.

### Building the GUI

The GUI uses Qt. Please download and install the latest version (Qt 5.3 at the time
of writing) and open gui.pro in QtCreator. Build and run by clicking on the build icon.

The GUI provides a simple interface to browsing and manipulating TeaSafe containers.

![TeaSafe GUI](screenshots/gui.png?raw=true)

Note, building the gui requires building libtesafe.la first by doing a make in the top level folder.

Licensing
---------

TeaSafe follows the BSD 3-Clause licence. 


