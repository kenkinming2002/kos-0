Objects Hierachy in VFS: Vnode -> File -> Inode
1. Vnode
 - Form the entire fs tree
 - contain *Shared* pointer to File
 - This is the layer which support mounting since it has shared semantics

2. File
 - FS-Independent representation of inode with optional caching
 - contain *Unique* pointer to Inode

3. Inode
 - FS-Dependent with only minimal funtionality implemented
 - most of the functionality especially related to caching is to be supplemented
   by File

We have to separate Vnode and Inode because we do not want to have duplicate
file caches if the same filesystem is mounted in two places.
