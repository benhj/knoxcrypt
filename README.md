bfs
===

A custom fuse filesystem that I'm experimenting with.

File are stored as 512 byte blocks. 
Blocks are assigned to files as they are written. 
Allocated blocks are represented as bits set in a volume bitmap.

For example, a 1MB file contains 2048x512 byte blocks. Thus 2048 bits will indicate
which blocks are allocated. This will constitute the volume bitmap. 
The size of this bitmap is then dependent on the number of blocks in the container. 

Folder entries are also stored as file data. A folder entry's file data is basically
a description of its contents. One entry descriptor is a metadata byte describing 
if the entry is in use and its type (one bit for each; other bits can be assigned
for other things, e.g. if entry is read-only). The next 256 bytes describe the entry's
filename. The final 8 bytes represents a 64 bit pointer to the entry's first file block.

When the file system container is created, the root directory is automatically
added which is set to having zero entries. As files and folders are added to
the container, the root directory is accordingly updated. In a similar
manner, any sub folders will also be accordingly updated.

The whole filesystem image is also encrypted although currently the
implementation is not even as good as ARCFOUR and ARCFOUR is known
to be pretty bad in light of recent developments.
