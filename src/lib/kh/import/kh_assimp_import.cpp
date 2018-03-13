#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/matrix4x4.h>
#include <unordered_map>

#define MAX_NODE_IN_STACK 64
struct AssimpStackNode {
	u32 children;
	i32 index;
	struct aiNode *node;
};

KH_INTERN void
load_assimp_file(Assimp::Importer *importer, void *contents, u32 size) {
	u32 flags = aiProcess_Triangulate|aiProcess_GenNormals|aiProcess_LimitBoneWeights|aiProcess_JoinIdenticalVertices|aiProcess_MakeLeftHanded;
	// @NOTE(flo): seems dangerous
	// |aiProcess_OptimizeMeshes|aiProcess_OptimizeGraph;

	const aiScene* scene = importer->ReadFileFromMemory(contents, size, flags);
	bool no_err = (scene && scene->mFlags != AI_SCENE_FLAGS_INCOMPLETE && scene->mRootNode); 
	if(!no_err) {
		const char *err = importer->GetErrorString();
		kh_assert(!"assimp import failed");
	}
}

KH_INTERN void
close_assimp_file(Assimp::Importer *importer) {
	importer->FreeScene();
}

KH_INTERN void
load_trimesh_pnu(AssetContents *contents, const aiScene *scene, u32 mesh_count) {
	u32 vertex_size = contents->type.trimesh.vertex_size;
	u32 vertices_size = vertex_size * contents->type.trimesh.vert_c;
	u8 *dst = (u8 *)contents->memory;
	for(u32 i = 0; i < mesh_count; ++i) {
		aiMesh *mesh = scene->mMeshes[i];
		for(u32 j = 0; j < mesh->mNumVertices; ++j) {

			aiVector3D pos    = mesh->mVertices[j];
			aiVector3D normal = mesh->mNormals[j];
			aiVector3D uv0    = mesh->mTextureCoords[0][j];

			Vertex_PNU *dst_vertex = (Vertex_PNU *)dst;
			dst_vertex->pos    = kh_vec3(pos.x, pos.y, pos.z);
			dst_vertex->normal = kh_vec3(normal.x, normal.y, normal.z);
			dst_vertex->uv0    = kh_vec2(uv0.x, uv0.y);

			dst += vertex_size;
		}
	}
	dst = (u8 *)contents->memory + vertices_size;
	u32 vertex_offset = 0;
	for(u32 i = 0; i < mesh_count; ++i) {
		aiMesh *mesh = scene->mMeshes[i];
		for(u32 j = 0; j < mesh->mNumFaces; ++j) {
			aiFace *tri = mesh->mFaces + j;
			kh_assert(tri->mNumIndices == 3);

			u32 *dst_indices = (u32 *)dst;

			dst_indices[0] = tri->mIndices[0] + vertex_offset;
			dst_indices[1] = tri->mIndices[1] + vertex_offset;
			dst_indices[2] = tri->mIndices[2] + vertex_offset;

			dst += sizeof(u32) * 3;
		}
		vertex_offset += mesh->mNumVertices;
	}
}

KH_INTERN void
load_trimesh(AssetContents *contents, LinearArena *arena, const aiScene *scene, VertexFormat format) {
	kh_assert(scene->HasMeshes());
	LinearArena temp_arena = {};
	TransientLinear tmp = kh_begin_transient(&temp_arena);

	// TODO(flo): support multiple meshes
	u32 mesh_count = scene->mNumMeshes;

	u32 vertex_size = get_size_from_vertex_format(format);

	u32 vertices_count = 0;
	u32 tri_count = 0;
	for(u32 i = 0; i < mesh_count; ++i) {
		aiMesh *mesh = scene->mMeshes[i];
		vertices_count += mesh->mNumVertices;
		tri_count += mesh->mNumFaces;
	}

	u32 vertices_size = vertices_count * vertex_size;
	u32 indices_size = tri_count * 3 * sizeof(u32); 
	IndexMap index_map = {};

	AssetContents pnu_contents = {};
	pnu_contents.type.trimesh.vert_c = vertices_count;
	pnu_contents.type.trimesh.tri_c = tri_count;
	pnu_contents.type.trimesh.vertex_size = vertex_size;
	pnu_contents.type.trimesh.format = format;
	pnu_contents.type.trimesh.indices_offset = vertices_size; 
	pnu_contents.type.key = AssetType_trimesh;
	pnu_contents.size = vertices_size + indices_size;
	{
		if(format == VertexFormat_PNUS || format == VertexFormat_PNU) {
			pnu_contents.memory = kh_push(arena, pnu_contents.size);
			load_trimesh_pnu(&pnu_contents, scene, mesh_count);
			*contents = pnu_contents;
		} else {
			pnu_contents.type.trimesh.vertex_size = sizeof(Vertex_PNU);
			pnu_contents.type.trimesh.format = VertexFormat_PNU;
			pnu_contents.type.trimesh.indices_offset = sizeof(Vertex_PNU) * vertices_count;
			pnu_contents.size = sizeof(Vertex_PNU) * vertices_count + indices_size;
			pnu_contents.memory = kh_push(&temp_arena, pnu_contents.size);
			load_trimesh_pnu(&pnu_contents, scene, mesh_count);
			index_map.ind_list = (IndexList **)kh_push(&temp_arena, vertices_count * sizeof(IndexList *));  
			index_map.buffer_max_count = tri_count * 3 * sizeof(IndexList);
			index_map.buffer = (IndexList *)kh_push(&temp_arena, index_map.buffer_max_count);  
			AssetContents new_contents = add_mikktspace(&pnu_contents, &temp_arena, arena, format, &index_map);
			*contents = new_contents;
		}
	}

	if(has_skin(format)) {
		VertexSkinnedAttribute skin_attrib = get_skinned_attribute_offset(format);

		for(u32 i = 0; i < vertices_count; ++i) {
			u8 *vertex = (u8 *)contents->memory + (i * vertex_size);
			u32 *ids = (u32 *)(vertex + skin_attrib.offset);
			f32 *weights = (f32 *)(ids + MAX_JOINTS_PER_VERTEX);
			for(u32 j = 0; j < MAX_JOINTS_PER_VERTEX; ++j) {
				ids[j] = 0;
				weights[j] = 0.0f;
			}
		}
		u32 vertex_offset = 0;
		for(u32 mesh_i = 0; mesh_i < mesh_count; ++mesh_i) {
			u32 joint_count = 0;
			std::unordered_map<std::string, i32> joints_map;
			aiMesh *mesh = scene->mMeshes[mesh_i];
			for(u32 bone_i = 0; bone_i < mesh->mNumBones; ++bone_i) {
				aiBone *bone = mesh->mBones[bone_i];
				kh_assert(joints_map.find(bone->mName.data) == joints_map.end());
				std::string str(bone->mName.data);
				std::pair<std::string, i32> pair(str, bone_i);
				joints_map.insert(pair);
			}
			aiNode *root = scene->mRootNode;
			i32 top_stack = 0;
			AssimpStackNode stack_nodes[MAX_NODE_IN_STACK];
			stack_nodes[top_stack++] = {0, -1, root};
			for(AssimpStackNode *cur = stack_nodes + (top_stack - 1);;) {
				kh_assert(top_stack < MAX_NODE_IN_STACK);
				if(cur->children < cur->node->mNumChildren) {
					AssimpStackNode *parent = cur;
					cur =  stack_nodes + (top_stack++);

					aiNode *child = parent->node->mChildren[parent->children++];

					// Token name_token;
					// child = pass_weird_fbx_name(child, &name_token);

					cur->node = child; 
					cur->children = 0;
					cur->index = joint_count++;

					std::string str(child->mName.data);
					auto child_search = joints_map.find(str);
					if(child_search != joints_map.end()) {
						aiBone *bone = mesh->mBones[child_search->second];
						// TODO(flo): check if we have weird fbx name in the bone
						for(u32 j = 0; j < bone->mNumWeights; ++j) {
							aiVertexWeight *vert_weight = bone->mWeights + j;
							u32 vertex_id = vert_weight->mVertexId + vertex_offset;
							if(index_map.buffer_max_count > 0) {
								IndexList **first_ind = index_map.ind_list + vertex_id;
								for(IndexList *search = *first_ind; search; search = search->next) {
									u8 *vertex = (u8 *)contents->memory + (search->index * vertex_size);
									u32 *ids = (u32 *)(vertex + skin_attrib.offset);
									f32 *weights = (f32 *)(ids + MAX_JOINTS_PER_VERTEX);
									f32 weight = vert_weight->mWeight;
									b32 found = false;
									for(u32 k = 0; k < MAX_JOINTS_PER_VERTEX; ++k) {
										if(weights[k] == 0.0f) {
											ids[k] = cur->index;
											weights[k] = weight;
											found = true;
											break;
										}
									}
									kh_assert(found);
								}
							} else {
								u8 *vertex = (u8 *)contents->memory + (vertex_id * vertex_size);
								u32 *ids = (u32 *)(vertex + skin_attrib.offset);
								f32 *weights = (f32 *)(ids + MAX_JOINTS_PER_VERTEX);
								f32 weight = vert_weight->mWeight;
								b32 found = false;
								for(u32 k = 0; k < MAX_JOINTS_PER_VERTEX; ++k) {
									if(weights[k] == 0.0f) {
										ids[k] = cur->index;
										weights[k] = weight;
										found = true;
										break;
									}
								}
								kh_assert(found);
							}
						}
					}

				} else {
					if(top_stack > 1) {
						cur = stack_nodes + (--top_stack - 1);			
					} 
					else {
						break;
					}
				}
			}
			vertex_offset += mesh->mNumVertices;
		}
	}
	kh_end_transient(&tmp);
}

KH_INTERN void
load_skeleton_hierarchy(AssetContents *contents, LinearArena *arena, const aiScene* scene) {

	u32 joint_count = 0;
	aiNode *root = scene->mRootNode;
	i32 top_stack = 0;
	AssimpStackNode stack_nodes[MAX_NODE_IN_STACK];
	stack_nodes[top_stack++] = {0, -1, root};
	for(AssimpStackNode *cur = stack_nodes + (top_stack - 1);;) {
		kh_assert(top_stack < MAX_NODE_IN_STACK);
		if(cur->children < cur->node->mNumChildren) {
			AssimpStackNode *parent = cur;
			cur = stack_nodes + (top_stack++);
			aiNode *child = parent->node->mChildren[parent->children++];

			// Token name_token;
			// child = pass_weird_fbx_name(child, &name_token);

			cur->node = child; 
			cur->children = 0;
			cur->index = joint_count++;

		} else {
			if(top_stack > 1) {
				cur = stack_nodes + (--top_stack - 1);
			} else {
				break;
			}
		}
	}

	contents->size = joint_count * sizeof(Joint);
	contents->memory = kh_push(arena, contents->size);
	contents->type.key = AssetType_skeleton;
	contents->type.skeleton.joint_count = joint_count;

	Joint *joints = (Joint *)contents->memory;
	kh_assert(root);
	u32 index = 0;
	top_stack = 0;
	stack_nodes[top_stack++] = {0, -1, root};
	for(AssimpStackNode *cur = stack_nodes + (top_stack - 1);;) {
		kh_assert(top_stack < MAX_NODE_IN_STACK);
		if(cur->children < cur->node->mNumChildren) {
			AssimpStackNode *parent = cur;
			cur =  stack_nodes + (top_stack++);
			aiNode *child = parent->node->mChildren[parent->children++];

			// Token name_token;
			// child = pass_weird_fbx_name(child, &name_token);

			cur->node = child; 
			cur->children = 0;
			cur->index = index++;
			kh_assert(cur->index < (i32)joint_count);
			Joint *joint = joints + cur->index;

			kh_assert(child->mName.length < array_count(joint->debug_name));
			strings_copy(child->mName.length, child->mName.data, joint->debug_name);

			joint->parent_id = parent->index;
		} else {
			if(top_stack > 1) {
				cur = stack_nodes + (--top_stack - 1);			
			} 
			else {
				break;
			}
		}
	}
}

KH_INTERN void
load_skin_for_trimesh(AssetContents *contents, LinearArena *arena, const aiScene *scene) {

	aiNode *root = scene->mRootNode;
	u32 joint_count = 0;
	i32 top_stack = 0;
	AssimpStackNode stack_nodes[MAX_NODE_IN_STACK];
	stack_nodes[top_stack++] = {0, -1, root};
	for(AssimpStackNode *cur = stack_nodes + (top_stack - 1);;) {
		kh_assert(top_stack < MAX_NODE_IN_STACK);
		if(cur->children < cur->node->mNumChildren) {
			AssimpStackNode *parent = cur;
			cur = stack_nodes + (top_stack++);
			aiNode *child = parent->node->mChildren[parent->children++];

			// Token name_token;
			// child = pass_weird_fbx_name(child, &name_token);

			cur->node = child; 
			cur->children = 0;
			cur->index = joint_count++;
		} else {
			if(top_stack > 1) {
				cur = stack_nodes + (--top_stack - 1);
			} else {
				break;
			}
		}
	}

	u32 transform_size = (joint_count + 1) * sizeof(mat4);
	u32 pose_size = joint_count * sizeof(Transform_SQT);
	contents->size = transform_size + pose_size;
	contents->memory = kh_push(arena, contents->size);
	contents->type.key = AssetType_meshskin;

	mat4 *inverse_bind_poses = (mat4 *)contents->memory;
	Transform_SQT *local_poses = (Transform_SQT *)((u8 *)contents->memory + transform_size);

	kh_assert(root);
	inverse_bind_poses[joint_count] = kh_transpose_mat4(*(mat4 *)&root->mTransformation);

	u32 mesh_count = scene->mNumMeshes;
	std::unordered_map<std::string, std::pair<u32, i32>> joints_map;
	for(u32 i = 0; i < mesh_count; ++i) {
		aiMesh *mesh = scene->mMeshes[i];
		for(u32 j = 0; j < mesh->mNumBones; ++j) {
			aiBone *bone = mesh->mBones[j];
			std::string name(bone->mName.data);
			auto search = joints_map.find(name);
			if(search == joints_map.end()) {
				std::pair<std::string, std::pair<u32, i32>> pair;
				pair.first = name;
				std::pair<u32, u32> sec;
				sec.first = i;
				sec.second = j;
				pair.second = sec;
				joints_map.insert(pair);
			} else {

				aiMesh *test_mesh = scene->mMeshes[search->second.first];
				aiBone *test_bone = test_mesh->mBones[search->second.second];
				kh_assert(bone->mOffsetMatrix == test_bone->mOffsetMatrix);
			}
		}
	}


	u32 index_count = 0;
	top_stack = 0;
	stack_nodes[top_stack++] = {0, -1, root};
	for(AssimpStackNode *cur = stack_nodes + (top_stack - 1);;) {
		kh_assert(top_stack < MAX_NODE_IN_STACK);
		if(cur->children < cur->node->mNumChildren) {
			AssimpStackNode *parent = cur;

			cur =  stack_nodes + (top_stack++);
			aiNode *child = parent->node->mChildren[parent->children++];

			// Token name_token;
			// aiMatrix4x4t<double> transform;

			// AssimpFBXMatrix fbx_matrix;
			// child = set_fbx_matrix_transform(child, &name_token, &fbx_matrix);

			cur->node = child;
			cur->children = 0;
			cur->index = index_count++;

			kh_assert(cur->index < (i32)joint_count);

			std::string str(child->mName.data);
			auto child_search = joints_map.find(str);

			mat4 *inverse_bind_pose = inverse_bind_poses + cur->index; 

			if(child_search != joints_map.end()) {
				aiMesh *mesh = scene->mMeshes[child_search->second.first];
				aiBone *bone = mesh->mBones[child_search->second.second];
				*inverse_bind_pose = kh_transpose_mat4(*(mat4 *)(&bone->mOffsetMatrix));
			} else {
				*inverse_bind_pose = kh_identity_mat4();
			}
			// aiMatrix4x4 mat = get_result_fbx_matrix(&fbx_matrix) * child->mTransformation;
			aiMatrix4x4 mat = child->mTransformation;

			aiVector3D pos;
			aiQuaternion rot;
			aiVector3D scale;

			mat.Decompose(scale, rot, pos);
			Transform_SQT *local_pose = local_poses + cur->index;
			local_pose->pos = kh_vec3(pos.x, pos.y, pos.z);
			local_pose->rot = kh_quat(rot.x, rot.y, rot.z, rot.w);
			local_pose->scale = kh_vec3(scale.x, scale.y, scale.z);

		} else {
			if(top_stack > 1) {
				cur = stack_nodes + (--top_stack - 1);			
			} 
			else {
				break;
			}
		}
	}
}

// TODO(flo): load this only once and set the index map for the mesh
KH_INTERN void
load_animation_clip(AssetContents *contents, LinearArena *arena, const aiScene *scene) {

	// TODO(flo): we need to specify what animation we want to load
	aiAnimation *anim = scene->mAnimations[0];
	kh_assert(anim);

	aiNode *root = scene->mRootNode;
	kh_assert(root);
	std::vector<std::string> joints_arr;

	u32 joint_count = 0;
	i32 top_stack = 0;
	AssimpStackNode stack_nodes[MAX_NODE_IN_STACK];
	stack_nodes[top_stack++] = {0, -1, root};
	for(AssimpStackNode *cur = stack_nodes + (top_stack - 1);;) {
		kh_assert(top_stack < MAX_NODE_IN_STACK);
		if(cur->children < cur->node->mNumChildren) {
			AssimpStackNode *parent = cur;
			cur = stack_nodes + (top_stack++);
			aiNode *child = parent->node->mChildren[parent->children++];
			cur->node = child; 
			cur->children = 0;
			cur->index = joint_count++;

			std::string name(child->mName.data);
			joints_arr.push_back(name);
		} else {
			if(top_stack > 1) {
				cur = stack_nodes + (--top_stack - 1);
			} else {
				break;
			}
		}
	}

	// TODO(flo): do we have to make 'vertices/indices-like' arrays ?
	u32 sample_count = anim->mChannels[0]->mNumPositionKeys;
	for(u32 i = 0; i < anim->mNumChannels; ++i) {
		aiNodeAnim *node = anim->mChannels[i];
		u32 num_pos_key = node->mNumPositionKeys;
		if(num_pos_key > sample_count) {
			sample_count = num_pos_key;
		}
		u32 num_rot_key = node->mNumRotationKeys;
		if(num_rot_key > sample_count) {
			sample_count = num_rot_key;
		}
		u32 num_sca_key = node->mNumScalingKeys;
		if(num_sca_key > sample_count) {
			sample_count = num_rot_key;
		}
	}

	f64 sample_time = anim->mChannels[0]->mPositionKeys[1].mTime - anim->mChannels[0]->mPositionKeys[0].mTime; 
	u32 ticks_per_second = 30.0f;
	if(anim->mTicksPerSecond != 0.0f) ticks_per_second = anim->mTicksPerSecond;

	contents->type.animation.sample_count = sample_count;
	contents->type.animation.duration = anim->mDuration / ticks_per_second;
	contents->type.animation.frame_rate = sample_time / ticks_per_second;
	contents->type.animation.joint_count = joint_count;
	contents->type.key = AssetType_animation;

	contents->size = (sample_count * joint_count) * sizeof(Transform_SQT);
	contents->memory = kh_push(arena, contents->size);

	struct SampleCounter {
		u32 pos_count;
		u32 rot_count;
		u32 sca_count;
	};

	char *fbx_keyword = "AssimpFbx";
	std::unordered_map<std::string, std::vector<u32>> anim_map;

	std::unordered_map<std::string, SampleCounter> counter_map;

	for(u32 i = 0; i < anim->mNumChannels; ++i) {

		aiNodeAnim *channel = anim->mChannels[i];

		std::string str(channel->mNodeName.data, channel->mNodeName.length);
		auto search = anim_map.find(str);

		if(search == anim_map.end())
		{
			std::pair<std::string, std::vector<u32>> pair;
			pair.first = str;
			pair.second.push_back(i);
			anim_map.insert(pair);

			kh_assert(counter_map.find(str) == counter_map.end());

			std::pair<std::string, SampleCounter> counter_pair;
			counter_pair.first = str;
			SampleCounter sample_counter;
			sample_counter.pos_count = 0;
			sample_counter.rot_count = 0;
			sample_counter.sca_count = 0;

			if(channel->mNumPositionKeys > 1) {
				sample_counter.pos_count += channel->mNumPositionKeys;
			}
			if(channel->mNumRotationKeys > 1) {
				sample_counter.rot_count += channel->mNumRotationKeys;
			}
			if(channel->mNumScalingKeys > 1) {
				sample_counter.sca_count += channel->mNumScalingKeys;
			}

			counter_pair.second = sample_counter;
			counter_map.insert(counter_pair);

		} else {

			search->second.push_back(i);

			auto counter = counter_map.find(str);
			kh_assert(counter != counter_map.end());
			if(channel->mNumPositionKeys > 1) {
				counter->second.pos_count += channel->mNumPositionKeys;
			}
			if(channel->mNumRotationKeys > 1) {
				counter->second.rot_count += channel->mNumRotationKeys;
			}
			if(channel->mNumScalingKeys > 1) {
				counter->second.sca_count += channel->mNumScalingKeys;
			}
		}
	}
	kh_assert(anim_map.size() <= anim->mNumChannels);

	for(auto i : counter_map) {
		SampleCounter counter = i.second;

		kh_assert(counter.pos_count <= sample_count);
		kh_assert(counter.rot_count <= sample_count);
		kh_assert(counter.sca_count <= sample_count);
	}

	Transform_SQT *samples = (Transform_SQT *)contents->memory;

	for(u32 joint_i = 0; joint_i < joint_count; ++joint_i) {
		auto search = anim_map.find(joints_arr[joint_i]);
		for(u32 sample_i = 0; sample_i < sample_count; ++sample_i) {
			u32 index = (sample_i * joint_count) + joint_i;
			kh_assert(index < sample_count * joint_count);
			Transform_SQT *sqt = samples + index++; 

			sqt->pos = kh_vec3(0,0,0);
			sqt->rot = quat_identity();
			sqt->scale = kh_vec3(1,1,1);
			if(search != anim_map.end()) {
				for(u32 k = 0; k < search->second.size(); ++k) {
					aiNodeAnim *channel = anim->mChannels[search->second[k]];
					if(sample_i < channel->mNumPositionKeys && channel->mNumPositionKeys > 1) {
						aiVectorKey *pos_key = channel->mPositionKeys + sample_i;
						sqt->pos = kh_vec3(pos_key->mValue.x, pos_key->mValue.y, pos_key->mValue.z);
					}
					if(sample_i < channel->mNumRotationKeys && channel->mNumRotationKeys > 1) {
						aiQuatKey *rot_key = channel->mRotationKeys + sample_i;
						sqt->rot = kh_quat(rot_key->mValue.x, rot_key->mValue.y, rot_key->mValue.z, rot_key->mValue.w);
					}
					if(sample_i < channel->mNumScalingKeys && channel->mNumScalingKeys > 1) {
						aiVectorKey *sca_key = channel->mScalingKeys + sample_i;
						sqt->scale = kh_vec3(sca_key->mValue.x, sca_key->mValue.y, sca_key->mValue.z);
					}
				}
			}
		}
	}
}

KH_INTERN void
load_animation_for_skin(AssetContents *contents, LinearArena *arena, const aiScene *skin_scene, const aiScene *anim_scene) {

	u32 joint_count = 0;
	i32 top_stack = 0;
	AssimpStackNode stack_nodes[MAX_NODE_IN_STACK];
	stack_nodes[top_stack++] = {0, -1, skin_scene->mRootNode};
	for(AssimpStackNode *cur = stack_nodes + (top_stack - 1);;) {
		kh_assert(top_stack < MAX_NODE_IN_STACK);
		if(cur->children < cur->node->mNumChildren) {
			AssimpStackNode *parent = cur;
			cur = stack_nodes + (top_stack++);
			aiNode *child = parent->node->mChildren[parent->children++];

			// Token name_token;
			// child = pass_weird_fbx_name(child, &name_token);

			cur->node = child; 
			cur->children = 0;
			cur->index = joint_count++;

		} else {
			if(top_stack > 1) {
				cur = stack_nodes + (--top_stack - 1);
			} else {
				break;
			}
		}
	}

	contents->type.key = AssetType_animationskin;
	contents->size = joint_count * sizeof(u32);
	contents->memory = kh_push(arena, contents->size);
	
	i32 *index_map = (i32 *)contents->memory;

	std::unordered_map<std::string, u32> joints_map;
	u32 anim_joint_count = 0;
	top_stack = 0;
	stack_nodes[top_stack++] = {0, -1, anim_scene->mRootNode};
	for(AssimpStackNode *cur = stack_nodes + (top_stack - 1);;) {
		kh_assert(top_stack < MAX_NODE_IN_STACK);
		if(cur->children < cur->node->mNumChildren) {
			AssimpStackNode *parent = cur;
			cur = stack_nodes + (top_stack++);
			aiNode *child = parent->node->mChildren[parent->children++];
			// Token name_token;
			// child = pass_weird_fbx_name(child, &name_token);
			cur->node = child; 
			cur->children = 0;
			cur->index = anim_joint_count++;

			std::string name(child->mName.data);
			std::pair<std::string, u32> p;
			p.first = name;
			p.second = cur->index;
			kh_assert(joints_map.find(name) == joints_map.end());
			joints_map.insert(p);	
		} else {
			if(top_stack > 1) {
				cur = stack_nodes + (--top_stack - 1);
			} else {
				break;
			}
		}
	}

	top_stack = 0;
	u32 index = 0;
	stack_nodes[top_stack++] = {0, -1, skin_scene->mRootNode};
	for(AssimpStackNode *cur = stack_nodes + (top_stack - 1);;) {
		kh_assert(top_stack < MAX_NODE_IN_STACK);
		if(cur->children < cur->node->mNumChildren) {
			AssimpStackNode *parent = cur;
			cur = stack_nodes + (top_stack++);
			aiNode *child = parent->node->mChildren[parent->children++];

			// Token name_token;
			// child = pass_weird_fbx_name(child, &name_token);
			cur->node = child; 
			cur->children = 0;
			cur->index = index++;

			kh_assert(cur->index < (i32)joint_count);
			index_map[cur->index] = -1;

			std::string name(child->mName.data);
			auto search = joints_map.find(name);
			if(search != joints_map.end()) {
				u32 anim_joint_index = search->second;
				kh_assert(anim_joint_index < anim_joint_count);
				index_map[cur->index] = (i32)anim_joint_index;
			}

		} else {
			if(top_stack > 1) {
				cur = stack_nodes + (--top_stack - 1);
			} else {
				break;
			}
		}
	}
}