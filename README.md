BFS: A fuse-based encrypted filesystem
======================================

- Files are stored as 512 byte blocks. 
- Blocks are assigned to files as they are written. 
- Allocated blocks are represented as bits set in a volume bitmap.

For example, a 1MB file contains 2048x512 byte blocks so a volume bitmap
of size 2048.

Folder entries
==============

- folder entries are also stored as file data. 
- a folder entry's file data is basically a description of its contents. 
- a single entry is desribed by 264 bytes
- descriptor is a metadata byte describing 
- the first bit of the first byte indicates if the data 'is in use' 
(it is put out of use when deleted).
- the second bit of the first bute represents the entry's type 
- tbd for the remaining bits of the first byte 
- the next 255 bytes describe the entry's filename. 
- the final 8 bytes represents a 64 bit pointer to the entry's first file block.

When the file system container is created, the root directory is automatically
added which is set to having zero entries. As files and folders are added to
the container, the root directory is accordingly updated. In a similar
manner, any sub folders will also be accordingly updated.

The whole filesystem image is also encrypted although currently the
implementation is not even as good as ARCFOUR and ARCFOUR is known
to be pretty bad in light of recent developments.
