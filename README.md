TeaSafe: An experimental fuse-based filesystem
----------------------------------------------

TeaSafe is an experimental filesystem designed with encryption in mind. 
The whole filesystem is block-based, exising as a single 'image'
that can be mounted to a user-specified mount-point. Rather than encrypting each file entry
individually, the whole image is transformed using a variant of the XTEA algorithm.
TeaSafe also incorporates an experimental
'coffee mode' in which a 'hidden partition' can be specified at the time
of image creation. At the time of mounting, the user can choose to mount this
rather than the default root folder.

### The TeaSafe image

- The whole filesystem is represented as a file image
- The first 8 bytes represent a 64 bit counter describing FS size (basically the number of file blocks).
- The next N bits constitute the volume bit map
- The next 8 bytes represent how many 'root' entries there are. The root constitutes
the root folder of the FS, so there is normally only one root entry. This indicator
is obsolete and is always set to '1' even if 'coffee mode' is used.
- The next M x N bytes represent the file block data

### Encryption

The filesystem is encrypted using a varient of the XTEA algorithm. 
I don't consider myself an expert in encryption so I would suggest you
review my code before you consider it secure or not.
The more keen developer is encouraged to implement their own transformational cipher. All she 
needs to do is implement the function `doTransform` in `IByteTransformer` as defined in `IByteTransformer.hpp`.
See file `BasicByteTransformer.hpp` as an example of how this is done. The pointer type of `m_byteTransformer`
as initialized in the constructor argument list of `TeaSafeImageStream.cpp` then needs to be updated to
the developer's new implementation e.g.:

`m_byteTransformer(boost::make_shared<cipher::BasicByteTransformer>(io.password))` --->
`m_byteTransformer(boost::make_shared<cipher::SomeOtherImplementation>(io.password))`

### Development requirements

All development was undertaken on a machine running osx10.9.
The actual development requirements are thus:

- Because of the use of strongly-typed enums, a c++11 compiler 
- a couple of boost libraries (system and filesystem) and the boost headers. Note, the makefile will need 
updating according to where boost is installed
- OpenSSL crypto library and sha.h header which is only used for generating SHA256 hashes
- the latest version of osxfuse (I'm using 2.6.2). As with boost, you might need to update the makefile
(afaik, on linux, an implementation of FUSE is part of the kernel already).

I envisage no problems running and compiling on linux. Windows unfortunately is a completely different beast
not least of which is due to a lack of a FUSE implementation.

### Compiling

`make` or `make all` will compile everything and all binaries. Please see above notes
on modifying the Makefile to point to correct library and header paths.

### Running

`./test` will run the test suite. This unit tests test various parts of TeaSafe. As I uncover
new bugs and attemp to fix them, I will add new units to verify the fixes.

`./maketeasafe 128000 image.teasf` will create a 500MB TeaSafe image when the block
size is 4096 (note the block size is hardcoded into DetailTeaSafe.hpp and represents
a good compromise between file speed and space efficiency). 

`./teasafe image.teasf testMount` will launch and mount image.teasf under 
the directory testMount in fuse debug mode; note to disable debug
mode you need to specify `--debug 0' as an extra parameter. Disabling
debug mode will mount the image in single-threaded mode. Multi-threaded mode
is not currently supported.
