#ifndef KH_RENDERER_SOFTWARE_H

struct SoftwarePixelsBuffer {
	void *memory;
	i32 w;
	i32 h;
	i32 pitch;
};

struct SoftwareDepthBuffer {
	f32 *memory;
};

struct SoftwareFrameBuffer {
	SoftwarePixelsBuffer pixels;
	SoftwareDepthBuffer zbuffer;
};

struct TriMeshRenderWork {
	SoftwareFrameBuffer *target;
	f32 min_x;
	f32 max_x;
	f32 min_y;
	f32 max_y;


	DataCache *cache;
	DirectionalLight *light;
	
	u32 mesh_off;
	u32 diff_off;
	VertexAttribute attrib;
	mat4 mvp;
	mat4 wld;
	v4 color;
};

struct DepthBufferClearWork {
	SoftwareDepthBuffer *buffer;
	u32 start;
	u32 end;
};

struct PixelsBufferClearWork {
	SoftwarePixelsBuffer *target;
	u32 min_y, max_y;
	v4 color;
};

#define KH_RENDERER_SOFTWARE_H
#endif //KH_RENDERER_SOFTWARE_H