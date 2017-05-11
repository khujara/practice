This is my experiment code (no dependency) related to game engine programming.

Features
===

### Available

- Simple libraries (3D math, job-based multithreading, memory management (ring/frame/static allocators), lexer, renderer api)
- 3D SIMD/multithreaded software rasterizer (half-space approach, scanline available with non SIMD instructions)
- OpenGL 4.5 renderer (AZDO techniques : sparse texture, bindless texture, buffer storage)
- Asset packaging tool

### In Progress

- Animation (FK/IK) system
- Memory library improvements (more allocators : general purpose (ft LRU), linked/free list (pool) allocator, buddy memory)
- Math visualization tool
- Raytracer
- PBR rendering

### Future

- Scene graph and spatial partitioning
- Fluid simulation
- Collision Detection (GJK)

Building
===

build with Microsoft Visual Studio 14.0 for Windows

clang compiler and other platforms are not supported yet.

### Procedure

1- launch build.bat (in the src folder)

2- make sure that the asset package and the executable are in the same folder

## Keyboard shortcuts :
- 'S' : Switch from Opengl renderer to Software renderer
- '0' : Basic Scene Sample
- '1' : Basic Mesh Sample

you can find the asset package and a build version here : 

https://www.dropbox.com/sh/pztteq8za6nd45x/AABXQ8hMoonAclBoB-mWAyQsa?dl=0
