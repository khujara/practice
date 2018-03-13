/* TODO(flo): this file should be generated from a text file at packaging time
(that can be generated in the asset packager once we have ui for it)
*/
#define FOREACH_ASSET_NAME(ASSETNAME) \
	ASSETNAME(AssetName_bricks) \
	ASSETNAME(AssetName_bricks_normal) \
	ASSETNAME(AssetName_wall) \
	ASSETNAME(AssetName_wall_normal) \
	ASSETNAME(AssetName_randy_albedo) \
	ASSETNAME(AssetName_randy_normal) \
	ASSETNAME(AssetName_randy_ao) \
	ASSETNAME(AssetName_randy_metalness) \
	ASSETNAME(AssetName_randy_gloss) \
	ASSETNAME(AssetName_skybox_back) \
	ASSETNAME(AssetName_skybox_bottom) \
	ASSETNAME(AssetName_skybox_front) \
	ASSETNAME(AssetName_skybox_left) \
	ASSETNAME(AssetName_skybox_right) \
	ASSETNAME(AssetName_skybox_top) \
	ASSETNAME(AssetName_randy_mesh) \
	ASSETNAME(AssetName_randy_mesh_tangent) \
	ASSETNAME(AssetName_randy_mesh_skinned) \
	ASSETNAME(AssetName_randy_skeleton) \
	ASSETNAME(AssetName_randy_skin) \
	ASSETNAME(AssetName_randy_idle) \
	ASSETNAME(AssetName_randy_idle_skin) \
	ASSETNAME(AssetName_icosphere) \
	ASSETNAME(AssetName_terrain_test) \
	ASSETNAME(AssetName_cube) \
	ASSETNAME(AssetName_font) \
	ASSETNAME(AssetName_lulu_color_chart) \
	ASSETNAME(AssetName_lulu) \
	ASSETNAME(AssetName_lulu_skeleton) \
	ASSETNAME(AssetName_lulu_skin) \
	ASSETNAME(AssetName_lulu_idle) \
	ASSETNAME(AssetName_lulu_idle_skin) \
	ASSETNAME(AssetName_lulu_run) \
	ASSETNAME(AssetName_lulu_run_skin) \
	ASSETNAME(AssetName_gold_albedo) \
	ASSETNAME(AssetName_gold_ao) \
	ASSETNAME(AssetName_gold_metallic) \
	ASSETNAME(AssetName_gold_normal) \
	ASSETNAME(AssetName_gold_roughness) \
	ASSETNAME(AssetName_count) \

#define FOREACH_ASSET_TAG(ASSETTAG) \
	ASSETTAG(AssetTag_vertical_wall) \
	ASSETTAG(AssetTag_greyness) \
	ASSETTAG(AssetTag_count) \

enum AssetName {
	FOREACH_ASSET_NAME(GENERATE_ENUM)
};

enum AssetTagKeys {
	FOREACH_ASSET_TAG(GENERATE_ENUM)
};

#ifdef KH_EDITOR_MODE
char *G_ASSET_NAMES[] = {
	FOREACH_ASSET_NAME(GENERATE_STR)
};

char *G_ASSET_TAGS[] = {
	FOREACH_ASSET_TAG(GENERATE_STR)
};

char *G_ASSET_TYPES[] = {
	FOREACH_ASSET_TYPE_KEY(GENERATE_STR)
};
#endif