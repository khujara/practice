#define FOREACH_SHADING_TYPE(SHADINGTYPE) \
	SHADINGTYPE(Shading_phong) \
	SHADINGTYPE(Shading_normalmap) \
	SHADINGTYPE(Shading_pbr) \
	SHADINGTYPE(Shading_color) \
	SHADINGTYPE(Shading_diffuse) \
	SHADINGTYPE(Shading_count) \

enum ShadingType {
	FOREACH_SHADING_TYPE(GENERATE_ENUM)
};

char *G_SHADING_TYPES[] = {
	FOREACH_SHADING_TYPE(GENERATE_STR)
};