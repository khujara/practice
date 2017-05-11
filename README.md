This is my experiment code (no dependency) related to engine game programming.

Features
===

### Available

- some simple libraries (math, work-based multithreading, asset import, memory management (ring/frame/static allocators), lexer)
- 3D SIMD software rasterizer (half-space approach, scanline available with non SIMD instructions)
- data structures viewer (linked list, stacks, queues, some trees...)
- asset packaging tool

### In Progress

- memory library improvements (more allocators : general purpose (ft LRU), linked/free list (pool) allocator, buddy memory)
- math library improvements
- math visualization tool
- openGL PBR rendering

### Future

- lexer improvements and parser for some metaprogramming features
- scene graph and spatial partitioning
- raytracing renderer
- fluid simulation
- basic animations/physics system

Building
===

build with Microsoft Visual Studio 14.0 for Windows

clang compiler and other platforms are not supported yet.

### Procedure

1- launch build.bat (in the src folder)

2- make sure that the asset package and the executable are in the same folder

you can find the asset package and a build version here : 

https://www.dropbox.com/sh/pztteq8za6nd45x/AABXQ8hMoonAclBoB-mWAyQsa?dl=0
