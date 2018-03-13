/*NOTE(flo) :

	Separate Direct and Indirect illumination

	PSEUDO CODE :
	v3 trace_path(Ray *ray, u32 depth) {
		if(depth == max_depth) return(kh_vec3(0,0,0));

		SurfaceIntersevction isect = {};
		intersect_bvh(bvh, bvh->root, ray, &isect);
	
		if(!isect.hit) return(kh_vec3(0,0,0));

		SurfaceMaterial m = isect.material;
		v3 emittance = m.emittance;

		Ray new_ray;
		new_ray.origin = isect.pos;
		new_ray.dir = random_distribution/specular/dielectrices/???; function here;

		// NOTE(flo): assuming it is lambertian material
		f32 cos_theta = kh_dot_v3(ray.dir, isect.normal);
		v3 brdf = m.reflectance * cos_theta;
		v3 reflected = trace_path(&new_ray, depth + 1);

		v3 rendering_equation = emittance + kh_hadamard_v3(brdf, reflected));
		return(rendering_equation);
	}
*/
// TODO(flo): remove this
#include <algorithm>

#define MAX_BOUNCE_PER_RAY 0

struct PathRay {
	v3 orig;
	v3 dir;
	f32 tmin;
	f32 tmax;
	f32 t;
	// TODO(flo): implement medium/particpating media
	// ParticpatingMedia *media;
};

struct LensCamera {
	v3 origin;
	v3 lower_left_corner;
	v3 horizontal;
	v3 vertical;
	v3 x_axis;
	v3 y_axis;
	v3 z_axis;
	f32 lens_radius;

	f32 shutter_t0;
	f32 shutter_t1;
};

struct Reflection {
	b32 scattered;
	PathRay path;
	v3 attenuation;
};

struct Refraction {
	b32 refracted;
	v3 dir;
};

struct SurfaceInteraction;
struct Shape;
struct PathSolution;

// TODO(flo): instead of precomputing pos/normal/uv0 use a v3 for barycentric coordinates in the triangle to blend


struct SurfaceInteraction {
	b32 hit;
	f32 t;
	u32 prim_index;
	v3 pos;
	v3 normal_nn;
	f32 normal_len;
};

struct Primitive {
	u32 pool_entity;
	u32 first_index_offset;
	// Triangle tri;
};

struct PrimitiveList {
	Primitive *ptr;
	u32 count;
	u32 max_count;
};

enum BVHNodeType {
	BVHNode_root,
	BVHNode_interior,
	BVHNode_leaf,
};

struct BVHNode {
	BVHNodeType type;
	BVHNode *left;
	BVHNode *right;
	AABB box;
	u32 first_prim_offset;
	u32 prim_count;
	u32 split_axis;
};

struct BVHTree {
	BVHNode *root;
	LinearArena *arena;
	u32 interior_count;
	u32 leaf_count;
};

struct PathImage {
	u32 w;
	u32 h;
	u32 bpp;
	u8 *pixels;
};


struct PathSkybox {
	i32 w,h;
	i32 bpp;
	f32 *pixels;
};

struct PathSolution {
	// TODO(flo): CLEANUP(flo) : remove these once we're done
	Assets *assets;
	RenderManager *render;
	Scene *scene;

	PathSkybox skybox;

	RandomXOR entropy;
	u32 rays_per_pixel;
	Entity entity;
	AssetID out_texture;
	PrimitiveList prim_list;
	BVHTree bvh;
};