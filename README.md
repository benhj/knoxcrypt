TeaSafe: An experimental fuse-based filesystem
----------------------------------------------

TeaSafe is an experimental filesystem designed with encryption in mind. 
Unlike its main contemporary (EncFS), the whole filesystem is block-based
meaning that the whole image is encrypted rather than each file element
of the filesystem.

### Technical overview

Files are stored as blocks; the block size is determined by the macro
`uint64_t const FILE_BLOCK_SIZE = 4096;` as found in the file `DetailTeaSafe.hpp`. 4096 represents 
a trade-off between file space usage and file access time efficiency. The smaller
the block size, the better the utilization of space but the poorer 
the write-time efficiency. A value of 4096 was found to represent a good tradeoff. (An
initial value of 512 was found to exhibit painfully slow write speeds.)

- Blocks are assigned to files as they are written and the file structure becomes 
what is essentially a linked list of file blocks. 
- Allocated blocks are represented as bits set in a volume bitmap stored near the beginning of the FS image.

For example, assuming a block size of 4096, a 1MB file system contains a bitmap volume size
of 256 bits (of course far larger filesystems are more common in practice).

### File block structure

A single file block is composed of a number of byte sequences.

- The first 4 bytes represent a 32 bit value which describes the number of data bytes stored
in the block. The total number of data bytes is N - 12 (e.g. N=4096).
- The next 8 bytes represent a 64 bit pointer to the next file block making up the file 
(this is why the structure is like a linked list).
- The remaining N-12 bytes represent actual file data.

### Folder entries

- folder entries are also stored as file data (and so are also composed of file blocks)
- a folder entry's file data is basically a description of its contents. 
- the folder contents is a set of metadata
- each metadata element of size 264 bytes describes a single file entry
- the first bit of the first byte of the element indicates if the metadata 'is in active use' (it is put out of use 
when the entry is deleted and can then be overwritten with the addition of new metadata).
- the second bit of the first byte represents the file entry's type (0 for file, 1 for folder) 
- t.b.d. for the remaining bits of the first byte 
- the next 255 bytes describe the entry's filename. 
- the final 8 bytes represents a 64 bit pointer to the file entry's first file block.

### The TeaSafe image

- The whole filesystem is represented as a file image
- The first 8 bytes represent a 64 bit counter describing FS size (basically the number of file blocks).
- The next N bits constitute the volume bit map
- The next 8 bytes represent how many 'root' entries there are. Note, this probably won't be used in future.
I envisage only one root entry, namely the 'root folder'.
- The next M x N bytes represent the file block data

When the file system container is created, the root directory is automatically
added which is set to having zero entries. As files and folders are added to
the container, the root directory is accordingly updated with new entry metadata. 
Any sub folders are accordingly updated in a similar manner.

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

Development requirements
------------------------

All development was undertaken on an osx machine running osx10.9.
The actual development requirements are thus:

- A c++11 compiler (this is only for the strongly typed enums)
- a couple of boost libraries (system and filesystem) and the boost headers. Note, the makefile will need 
updating according to where boost is installed
- the latest version of osxfuse (I'm using 2.6.2) and as with boost, you might need to update the makefile
(afaik, on linux, an implementation of FUSE is part of the kernel already).

I envisage no problems running and compiling on linux. Windows unfortunately is a completely different beast
not least of which is due to a lack of a FUSE implementation.

Compiling
---------

`make` or `make all` will compile everything and all binaries. To execute:

`./test` will run the test suite

`./maketeasafe 128000 image.teasafe` will create a 500MB TeaSafe image when the block
size is 4096. This will ask you for a password which is
used to seed a key used to encrypt / decrypt the file system.

`./teasafe image.teasafe testMount` will launch and mount image.teasafe under 
the directory testMount in fuse debug mode; note to disable debug
mode you need to specify `--debug 0' as an extra parameter. 
You will be asked for the password used to initially
encrypt the image.

To compile each component separately:-

`make test` will compile the test binary

`make maketeasafe` will compile the binary used to build a TeaSafe image

`make teasafe` will compile the fuse layer

