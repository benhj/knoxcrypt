BFS: A fuse-based encrypted filesystem
======================================

### Basic idea


- Files are stored as 512 byte blocks. 
- Blocks are assigned to files as they are written. 
- Allocated blocks are represented as bits set in a volume bitmap.

For example, a 1MB file contains 2048x512 byte blocks so a volume bitmap
of size 2048.

### File blocks

- File blocks are 512 bytes in length
- The first 4 bytes represents a 32 bit value describing the number of bytes represented by the fileblock
- The next 8 bytes represent a 64 bit pointer to the next file block making up a given file entry
- The remaining 500 bytes represent actual file data

### Folder entries

- folder entries are also stored as file data (and so are alos composed of file blocks)
- a folder entry's file data is basically a description of its contents. 
- a single entry is described by 264 bytes
- the first bit of the first byte indicates if the data 'is in use' (it is put out of use when the entry is deleted).
- the second bit of the first byte represents the entry's type 
- t.b.d. for the remaining bits of the first byte 
- the next 255 bytes describe the entry's filename. 
- the final 8 bytes represents a 64 bit pointer to the entry's first file block.

### The BFS image

- The whole bfs filesystem is represented as a file image
- The first 8 bytes represent a 64 bit count of how many file blocks exist
- The next N bits constitute the volume bit map (2048 bits for a 1MB image, i.e., 2048 x 512 byte blocks)
- The next 8 bytes represent how many 'root' entries there are. Note, this probably won't be used in future.
I envisage only one root entry, namely the 'root folder'.
- The next N x 512 bytes represent the file block data

When the file system container is created, the root directory is automatically
added which is set to having zero entries. As files and folders are added to
the container, the root directory is accordingly updated. In a similar
manner, any sub folders will also be accordingly updated.

### Encryption

The whole filesystem image can also be encrypted for extra security. 

Compiling
---------

- Requires boost filesystem libraries and boost headers.
- Makefile needs to be updated according to where boost is installed

`make test` will compile the test binary used to unit test the functionality

`./test` will run the unit test suite

`make makebfs` will compile the tool used to build a bfs image

`./makebfs 204800 image.bfs` will create a 100MB bfs image

`make bfs` will compile the fuse layer

`./bfs_ image.bfs 204800 testMount -o debug` will launch and mount image.bfs under the directory test mount in single-threaded mode.

More details to follow.
