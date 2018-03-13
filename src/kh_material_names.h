enum MaterialType {
	Type_Material_none,
	Type_Material_T2F2,
	Type_Material_T1F3,
	Type_Material_T2F3,
	Type_Material_T5,
	Type_Material_F4,
	Type_Material_T1,
	Type_Material_count,
};

char *G_MATERIAL_TYPES[Type_Material_count] = {
	"Material_none",
	"Material_T2F2",
	"Material_T1F3",
	"Material_T2F3",
	"Material_T5",
	"Material_F4",
	"Material_T1",
};

struct Material_none {
};

struct Material_T2F2 {
	AssetID diffuse;
	AssetID normal;
	f32 spec_power;
	f32 spec_intensity;
};

struct Material_T1F3 {
	AssetID diffuse;
	f32 spec_power;
	f32 spec_intensity;
	f32 reflect_intensity;
};

struct Material_T2F3 {
	AssetID diffuse;
	AssetID normal;
	f32 roughness;
	f32 metallic;
	f32 ao;
};

struct Material_T5 {
	AssetID albedo;
	AssetID normal;
	AssetID roughness;
	AssetID metalness;
	AssetID ao;
};

struct Material_F4 {
	v4 color;
};

struct Material_T1 {
	AssetID diffuse;
};

