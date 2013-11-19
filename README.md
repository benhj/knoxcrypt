bfs
===

A custom container format that I'm experimenting with.

One of the problems with the zip container is that files can only be appended. 
This is because files are stored as files so when removed, the underlying
space cannot be reclaimed.

This container format will work more like a filesystem: files will be stored
as blocks and a volume bitmap will indicate those blocks that are allocated.

(Note, this is very similar to how the HFS filesystem works)

For example, a 1MB file contains 2048x512 byte blocks. Thus 2048 bits will indicate
which blocks are allocated. This will constitute the volume bitmap. 
The size of this bitmap is then dependent onthe number of blocks in the container. 

Idea
====

When a file is created the associated bits in the bitmap are set and the volume
bitmap is updated. When a file is deleted the associated bits are cleared, the 
map is updated and the blocks are free to be used in the storage of other files.

A set of metadata the size of which is calculated as a fraction of the size of 
the allocated space wills store information about each file including index of
first allocated block, total number of allocated blocks, parent entry index,
and file permissions.

Each 512 byte file block will also store the number of bytes stored by the block
(<=512), and the previous and next block indices. The first block making up a file
will store the filename with a max length of 255 characters.
