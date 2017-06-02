#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <unordered_map>

#define MAX_NODE_IN_STACK 64
struct AssimpStackNode {
	u32 children;
	i32 index;
	struct aiNode *node;
};

struct AssimpFileInfo {
	std::unordered_map<std::string, u32> joints_map;
	u32 joint_count;
	AssimpStackNode stack_nodes[MAX_NODE_IN_STACK];
};

KH_INTERN void
load_assimp_file(Assimp::Importer *importer, void *contents, u32 size) {
	u32 flags = aiProcess_Triangulate|aiProcess_GenSmoothNormals|aiProcess_LimitBoneWeights|aiProcess_JoinIdenticalVertices|aiProcess_MakeLeftHanded|aiProcess_ImproveCacheLocality|aiProcess_CalcTangentSpace;
	const aiScene* scene = importer->ReadFileFromMemory(contents, size, flags);
	bool no_err = (scene && scene->mFlags != AI_SCENE_FLAGS_INCOMPLETE && scene->mRootNode); 
	if(!no_err) {
		const char *err = importer->GetErrorString();
		kh_assert(err);
	}
}

KH_INTERN void
close_assimp_file(Assimp::Importer *importer) {
	importer->FreeScene();
}

KH_INTERN AssimpFileInfo
get_assimp_file_info(const aiScene *scene) {
	AssimpFileInfo res;
	res.joint_count = 0;

	// TODO(flo): support multiple meshes
	kh_assert(scene->mNumMeshes == 1);

	aiMesh *mesh = scene->mMeshes[0];

	for(u32 i = 0; i < mesh->mNumBones; ++i) {
		aiBone *bone = mesh->mBones[i];
		kh_assert(res.joints_map.find(bone->mName.data) == res.joints_map.end());
		std::string str(bone->mName.data);
		std::pair<std::string, u32> pair(str, i);
		res.joints_map.insert(pair);
	}

	aiNode *root = scene->mRootNode;
	i32 top_stack = 0;
	res.stack_nodes[top_stack++] = {0, -1, root};
	for(AssimpStackNode *cur = res.stack_nodes + (top_stack - 1);;) {
		kh_assert(top_stack < MAX_NODE_IN_STACK);
		if(cur->children < cur->node->mNumChildren) {
			AssimpStackNode *parent = cur;
			cur = res.stack_nodes + (top_stack++);
			aiNode *child = parent->node->mChildren[parent->children++];
			cur->node = child; 
			cur->children = 0;
			cur->index = res.joint_count++;

		} else {
			if(top_stack > 1) {
				cur = res.stack_nodes + (--top_stack - 1);
			} else {
				break;
			}
		}
	}

	return(res);
}

KH_INTERN AssimpFileInfo
load_trimesh(AssetContents *contents, StackAllocator *allocator, const aiScene *scene, VertexFormat format) {
	AssimpFileInfo res = {};
	kh_assert(scene->HasMeshes());

	// TODO(flo): support multiple meshes
	kh_assert(scene->mNumMeshes == 1);

	aiMesh *mesh = scene->mMeshes[0];
	kh_assert(mesh);

	u32 vertex_size = get_size_from_vertex_format(format);
	u32 vertices_size = mesh->mNumVertices * vertex_size;
	u32 indices_size = mesh->mNumFaces * 3 * sizeof(u32); 


	contents->type.trimesh.vert_c = mesh->mNumVertices;
	contents->type.trimesh.tri_c = mesh->mNumFaces;
	contents->type.trimesh.vertex_size = vertex_size;
	contents->type.trimesh.format = format;
	contents->type.trimesh.indices_offset = vertices_size; 
	contents->type.key = AssetType_trimesh;
	contents->size = vertices_size + indices_size;
	contents->memory = kh_push(allocator, contents->size);

	if(has_skin(format)) {
		kh_assert(mesh->HasBones());
		VertexSkinnedAttribute skin_attrib = get_skinned_attribute_offset(format);

		for(u32 i = 0; i < mesh->mNumVertices; ++i) {
			u8 *vertex = (u8 *)contents->memory + (i * vertex_size);

			u32 *ids = (u32 *)(vertex + skin_attrib.offset);
			f32 *weights = (f32 *)(ids + MAX_JOINTS_PER_VERTEX);
			for(u32 j = 0; j < MAX_JOINTS_PER_VERTEX; ++j) {
				ids[j] = 0;
				weights[j] = 0.0f;
			}
		}

		for(u32 i = 0; i < mesh->mNumBones; ++i) {
			aiBone *bone = mesh->mBones[i];
			kh_assert(res.joints_map.find(bone->mName.data) == res.joints_map.end());
			std::string str(bone->mName.data);
			std::pair<std::string, u32> pair(str, i);
			res.joints_map.insert(pair);
		}

		aiNode *root = scene->mRootNode;
		i32 top_stack = 0;
		res.stack_nodes[top_stack++] = {0, -1, root};
		for(AssimpStackNode *cur = res.stack_nodes + (top_stack - 1);;) {
			kh_assert(top_stack < MAX_NODE_IN_STACK);
			if(cur->children < cur->node->mNumChildren) {
				AssimpStackNode *parent = cur;
				cur =  res.stack_nodes + (top_stack++);

				aiNode *child = parent->node->mChildren[parent->children++];
				cur->node = child; 
				cur->children = 0;
				cur->index = res.joint_count++;

				std::string str(child->mName.data);
				auto child_search = res.joints_map.find(str);

				if(child_search != res.joints_map.end()) {
					aiBone *bone = mesh->mBones[child_search->second];
					for(u32 i = 0; i < bone->mNumWeights; ++i) {
						aiVertexWeight *vert_weight = bone->mWeights + i;
						u32 vertex_id = vert_weight->mVertexId;
						kh_assert(vertex_id < mesh->mNumVertices);

						u8 *vertex = (u8 *)contents->memory + (vertex_id * vertex_size);
						u32 *ids = (u32 *)(vertex + skin_attrib.offset);
						f32 *weights = (f32 *)(ids + MAX_JOINTS_PER_VERTEX);
						f32 weight = vert_weight->mWeight;
						b32 found = false;
						for(u32 j = 0; j < MAX_JOINTS_PER_VERTEX; ++j) {
							if(weights[j] == 0.0f) {
								ids[j] = cur->index;
								weights[j] = weight;
								found = true;
								break;
							}
						}
						kh_assert(found);
					}
				}

			} else {
				if(top_stack > 1) {
					cur = res.stack_nodes + (--top_stack - 1);			
				} 
				else {
					break;
				}
			}
		}
	}

	switch(format) {
		case VertexFormat_PosNormalUVSkinned :
		case VertexFormat_PosNormalUV : {
			u8 *dst = (u8 *)contents->memory;
			for(u32 i = 0; i < mesh->mNumVertices; ++i) {

				aiVector3D pos    = mesh->mVertices[i];
				aiVector3D normal = mesh->mNormals[i];
				aiVector3D uv0    = mesh->mTextureCoords[0][i];

				Vertex_PNU *dst_vertex = (Vertex_PNU *)dst;
				dst_vertex->pos    = kh_vec3(pos.x, pos.y, pos.z);
				dst_vertex->normal = kh_vec3(normal.x, normal.y, normal.z);
				dst_vertex->uv0    = kh_vec2(uv0.x, uv0.y);

				dst += vertex_size;
			}

		} break;
		case VertexFormat_PosNormalTangentUVSkinned :
		case VertexFormat_PosNormalTangentUV : {

			u8 *dst = (u8 *)contents->memory;
			for(u32 i = 0; i < mesh->mNumVertices; ++i) {
				aiVector3D pos     = mesh->mVertices[i];
				aiVector3D normal  = mesh->mNormals[i];
				aiVector3D tangent = mesh->mTangents[i];
				aiVector3D uv0     = mesh->mTextureCoords[0][i];

				Vertex_PNUT *dst_vertex = (Vertex_PNUT *)dst;
				dst_vertex->pos     = kh_vec3(pos.x, pos.y, pos.z);
				dst_vertex->normal  = kh_vec3(normal.x, normal.y, normal.z);
				dst_vertex->uv0     = kh_vec2(uv0.x, uv0.y);
				dst_vertex->tangent = kh_vec3(tangent.x, tangent.y, tangent.z);

				dst += vertex_size;
			}
		} break;
		case VertexFormat_PosNormalTangentBitangentUVSkinned : 
		case VertexFormat_PosNormalTangentBitangentUV : {
			u8 *dst = (u8 *)contents->memory;
			for(u32 i = 0; i < mesh->mNumVertices; ++i) {
				aiVector3D pos       = mesh->mVertices[i];
				aiVector3D normal    = mesh->mNormals[i];
				aiVector3D tangent   = mesh->mTangents[i];
				aiVector3D bitangent = mesh->mBitangents[i];
				aiVector3D uv0       = mesh->mTextureCoords[0][i];

				Vertex_PNUTB *dst_vertex = (Vertex_PNUTB *)dst;
				dst_vertex->pos       = kh_vec3(pos.x, pos.y, pos.z);
				dst_vertex->normal    = kh_vec3(normal.x, normal.y, normal.z);
				dst_vertex->uv0       = kh_vec2(uv0.x, uv0.y);
				dst_vertex->tangent   = kh_vec3(tangent.x, tangent.y, tangent.z);
				dst_vertex->bitangent = kh_vec3(bitangent.x, bitangent.y, bitangent.z);

				dst += vertex_size;
			}
		} break;
		default : {
			kh_assert(!"invalid vertex format");
		} break;
	}

	u8 *dst = (u8 *)contents->memory + vertices_size;
	for(u32 i = 0; i < mesh->mNumFaces; ++i) {
		aiFace *tri = mesh->mFaces + i;
		kh_assert(tri->mNumIndices == 3);

		u32 *dst_indices = (u32 *)dst;

		dst_indices[0] = tri->mIndices[0];
		dst_indices[1] = tri->mIndices[1];
		dst_indices[2] = tri->mIndices[2];

		dst += sizeof(u32) * 3;

	}
	return(res);
}

KH_INTERN void
load_skeleton_hierarchy(AssetContents *contents, StackAllocator *allocator, const aiScene* scene, AssimpFileInfo *file) {
	contents->size = file->joint_count * sizeof(Joint);
	contents->memory = kh_push(allocator, contents->size);
	contents->type.key = AssetType_skeleton;
	contents->type.skeleton.joint_count = file->joint_count;

	Joint *joints = (Joint *)contents->memory;
	aiNode *root = scene->mRootNode;
	kh_assert(root);
	u32 index = 0;
	i32 top_stack = 0;
	file->stack_nodes[top_stack++] = {0, -1, root};
	for(AssimpStackNode *cur = file->stack_nodes + (top_stack - 1);;) {
		kh_assert(top_stack < MAX_NODE_IN_STACK);
		if(cur->children < cur->node->mNumChildren) {
			AssimpStackNode *parent = cur;
			cur =  file->stack_nodes + (top_stack++);
			aiNode *child = parent->node->mChildren[parent->children++];

			cur->node = child; 
			cur->children = 0;
			cur->index = index++;
			kh_assert(cur->index < (i32)file->joint_count);
			Joint *joint = joints + cur->index;
			joint->parent_id = parent->index;
		} else {
			if(top_stack > 1) {
				cur = file->stack_nodes + (--top_stack - 1);			
			} 
			else {
				break;
			}
		}
	}
}

KH_INTERN void
load_skin_for_trimesh(AssetContents *contents, StackAllocator *allocator, const aiScene *scene, AssimpFileInfo *file) {
	u32 transform_size = (file->joint_count + 1) * sizeof(mat4);
	u32 pose_size = file->joint_count * sizeof(Transform_SQT);
	contents->size = transform_size + pose_size;
	contents->memory = kh_push(allocator, contents->size);
	contents->type.key = AssetType_meshskin;


	mat4 *inverse_bind_poses = (mat4 *)contents->memory;
	Transform_SQT *local_poses = (Transform_SQT *)((u8 *)contents->memory + transform_size);

	aiNode *root = scene->mRootNode;
	kh_assert(root);
	inverse_bind_poses[file->joint_count] = kh_transpose_mat4(*(mat4 *)&root->mTransformation);

	// TODO(flo): support multiple meshes
	kh_assert(scene->mNumMeshes == 1);

	aiMesh *mesh = scene->mMeshes[0];
	kh_assert(mesh);

	u32 index_count = 0;
	i32 top_stack = 0;
	file->stack_nodes[top_stack++] = {0, -1, root};
	for(AssimpStackNode *cur = file->stack_nodes + (top_stack - 1);;) {
		kh_assert(top_stack < MAX_NODE_IN_STACK);
		if(cur->children < cur->node->mNumChildren) {
			AssimpStackNode *parent = cur;

			cur =  file->stack_nodes + (top_stack++);
			aiNode *child = parent->node->mChildren[parent->children++];
			cur->node = child;
			cur->children = 0;
			cur->index = index_count++;
			kh_assert(cur->index < (i32)file->joint_count);

			std::string str(child->mName.data);
			auto child_search = file->joints_map.find(str);

			mat4 *inverse_bind_pose = inverse_bind_poses + cur->index; 

			if(child_search != file->joints_map.end()) {
				aiBone *bone = mesh->mBones[child_search->second];
				*inverse_bind_pose = kh_transpose_mat4(*(mat4 *)(&bone->mOffsetMatrix));
			} else {
				*inverse_bind_pose = kh_identity_mat4();
			}
			aiVector3D pos;
			aiQuaternion rot;
			aiVector3D scale;

			child->mTransformation.Decompose(scale, rot, pos);
			Transform_SQT *local_pose = local_poses + cur->index;
			local_pose->pos = kh_vec3(pos.x, pos.y, pos.z);
			local_pose->rot = kh_quat(rot.x, rot.y, rot.z, rot.w);
			local_pose->scale = kh_vec3(scale.x, scale.y, scale.z);

		} else {
			if(top_stack > 1) {
				cur = file->stack_nodes + (--top_stack - 1);			
			} 
			else {
				break;
			}
		}
	}
}

KH_INTERN void
load_animation_clip(AssetContents *contents, StackAllocator *allocator, const aiScene *scene, AssimpFileInfo *file) {
	// TODO(flo): we need to specify what animation we want to load
	aiAnimation *anim = scene->mAnimations[0];
	kh_assert(anim);

	aiNode *root = scene->mRootNode;
	kh_assert(root);

	u32 joint_count = file->joint_count;
	u32 sample_count = anim->mChannels[0]->mNumPositionKeys;
	for(u32 i = 0; i < anim->mNumChannels; ++i) {
		aiNodeAnim *node = anim->mChannels[i];
		b32 num_pos_key = (node->mNumPositionKeys == sample_count);
		b32 num_rot_key = (node->mNumRotationKeys == sample_count);
		// b32 num_sca_key = (node->mNumScalingKeys == sample_count);
		b32 num_sca_key = true;

		kh_assert(num_pos_key && num_rot_key && num_sca_key);
	}

	f64 sample_time = anim->mChannels[0]->mPositionKeys[1].mTime - anim->mChannels[0]->mPositionKeys[0].mTime; 
	u32 ticks_per_second = 30.0f;
	if(anim->mTicksPerSecond != 0.0f) ticks_per_second = anim->mTicksPerSecond;

	contents->type.animation.sample_count = sample_count;
	contents->type.animation.duration = anim->mDuration;
	contents->type.animation.frame_rate = sample_time / ticks_per_second;
	contents->type.key = AssetType_animation;

	contents->size = (sample_count * joint_count) * sizeof(Transform_SQT);
	contents->memory = kh_push(allocator, contents->size);

	Transform_SQT *samples = (Transform_SQT *)contents->memory;

	std::vector<std::string> names;
	u32 stack_index = 0;
	i32 top_stack = 0;
	file->stack_nodes[top_stack++] = {0, -1, root};
	for(AssimpStackNode *cur = file->stack_nodes + (top_stack - 1);;) {
		kh_assert(top_stack < MAX_NODE_IN_STACK);
		if(cur->children < cur->node->mNumChildren) {
			AssimpStackNode *parent = cur;
			cur =  file->stack_nodes + (top_stack++);
			aiNode *child = parent->node->mChildren[parent->children++];
			cur->node = child; 
			cur->children = 0;
			cur->index = stack_index++;
			kh_assert(cur->index < (i32)joint_count);
			std::string name(child->mName.data);
			names.push_back(name);
		} else {
			if(top_stack > 1) {
				cur = file->stack_nodes + (--top_stack - 1);			
			} 
			else {
				break;
			}
		}
	}

	std::unordered_map<std::string, u32> anim_map;
	for(u32 i = 0; i < anim->mNumChannels; ++i) {
		aiNodeAnim *channel = anim->mChannels[i];
		std::string str(channel->mNodeName.data);
		kh_assert(anim_map.find(str) == anim_map.end());
		std::pair<std::string, u32> pair;
		pair.first = str;
		pair.second = i;
		anim_map.insert(pair);
	}
	kh_assert(anim_map.size() == anim->mNumChannels);
	u32 test = 0;
	for(u32 i = 0; i < joint_count; ++i) {

		u32 pos_count = sample_count;
		u32 rot_count = sample_count;
		u32 sca_count = sample_count;


		auto search = anim_map.find(names[i]);
		aiNodeAnim *channel = 0;
		if(search != anim_map.end()) {
			channel = anim->mChannels[search->second];	
			pos_count = channel->mNumPositionKeys;
			rot_count = channel->mNumRotationKeys;
			sca_count = channel->mNumScalingKeys;
		}
		for(u32 j = 0; j < sample_count; ++j) {
			u32 index = (j * joint_count) + i;
			test = index;
			kh_assert(index < sample_count * joint_count);

			Transform_SQT *sqt = samples + index; 
			sqt->pos = kh_vec3(0,0,0);
			sqt->rot = quat_identity();
			sqt->scale = kh_vec3(1,1,1);
			if(channel) {
				if(j < pos_count) {
					aiVectorKey *pos_key = channel->mPositionKeys + j;
					sqt->pos = kh_vec3(pos_key->mValue.x, pos_key->mValue.y, pos_key->mValue.z);
				}
				if(j < rot_count) {
					aiQuatKey *rot_key = channel->mRotationKeys + j;
					sqt->rot = kh_quat(rot_key->mValue.x, rot_key->mValue.y, rot_key->mValue.z, rot_key->mValue.w);

				}
				if(j < sca_count) {
					aiVectorKey *sca_key = channel->mScalingKeys + j;
					sqt->scale = kh_vec3(sca_key->mValue.x, sca_key->mValue.y, sca_key->mValue.z);
				}

			}
		}
	}
}