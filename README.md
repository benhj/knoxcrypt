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

Folder entries will also be treated as file entries that are constructed from file 
blocks. Their data will be formed of file name and other folder name entries that
link to the respective index values of the first file block making up a given file.

When the file system container is created, the root directory is automatically
added which is set to having zero entries. As files and folders are added to
the container, the root directory entry is accordingly updated. In a similar
manner, any sub folders will also be accordingly updated.
