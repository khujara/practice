This is my experiment code (no dependency) related to game engine programming.

Features
===

### Available

- Simple libraries (3D math, job-based multithreading, memory management (ring/frame/static allocators), lexer, renderer api)
- 3D SIMD/multithreaded software rasterizer (half-space approach, scanline available with non SIMD instructions)
- OpenGL 4.5 renderer 
	- AZDO techniques : sparse texture, bindless texture, texture arrays, buffer storage, multidraw
	- Shadow mapping
	- Skybox
	- Normal mapping
	- Phong Lighting
- Asset packaging tool
- Skeletal Animation import and display

### In Progress

- IK system
- Memory library improvements (more allocators : general purpose (ft LRU), linked/free list (pool) allocator, buddy memory)
- Collision Detection (GJK)

### Future plans

- Math visualization tool
- PBR rendering
- Raytracer
- Scene graph and spatial partitioning
- Fluid simulation

Build and Requirements
===

### Requirements

OpenGL 4.5  
64 bits Microsoft Windows  
Microsoft Visual Studio 14.0 C++ compiler  

### Support plans :

OpenGL 3.1  
Vulkan  
Clang++ compiler  
other 64 bits OS with SDL  

### Building

1- cd src, build.bat
2- make sure that the asset package (link below), the 'shaders' folder and the executable are in the same directory

### Dependencies

Assimp 3.1.1  
STB libraries (stb_image, stb_rectpack, stb_sprintf, stb_truetype)  

## Keyboard shortcuts :
- 'S' : Switch from OpenGL 4.5 renderer to Software renderer
- 'Q' : Toggle full screen
- '0' : Basic Scene Sample
- '1' : Basic Mesh Sample

you can find a build version here : 
https://www.dropbox.com/sh/pztteq8za6nd45x/AABXQ8hMoonAclBoB-mWAyQsa?dl=0  

Thanks to Philémon Belhomme for these assets!
