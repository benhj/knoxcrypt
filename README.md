bfs
===

A custom container format that I'm experimenting with.

One of the problems with the zip container is that files can only be appended. 
This is because files are stored sequentially, so when removed, the underlying
space cannot be easily reused or reclaimed.

This container format will work more like a filesystem: files will be stored
as blocks and a volume bitmap will indicate those blocks that are allocated.

(Note, this is very similar to how the HFS filesystem works)

For example, a 1MB file contains 2048x512 byte blocks. Thus 2048 bits will indicate
which blocks are allocated. This will constitute the volume bitmap. 
The size of this bitmap is then dependent on the number of blocks in the container. 

Idea
====

When a file is created the associated bits in the bitmap are set and the volume
bitmap is updated. When a file is deleted the associated bits are cleared, the 
map is updated and the blocks are free to be used in the storage of other files.

A set of metadata the size of which is calculated as a fraction of the size of 
the allocated space will store information about each file including whether the
metablock is currently in use (stored by the first bit of the very first byte),
the index of the first allocated block (8 bytes), the total number of allocated blocks
(8 bytes), and the parent entry index (8 bytes). Thus each metablock is 25 bytes.

Each 512 byte file block will also store the number of bytes stored by the block 
(<=512-20 stored as a 4 byte value), and the previous and next block indices
(stored as 8 byte values). The first block making up a file will also store the 
filename in the first 255 bytes of data space (null terminated).
