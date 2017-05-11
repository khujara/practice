#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

KH_INTERN TriangleMeshContents
load_trimesh(StackAllocator *mem_stack, void *contents, u32 size, VertexFormat format) {
	TriangleMeshContents res = {};

	Assimp::Importer importer;

	u32 flags = aiProcess_Triangulate|aiProcess_GenSmoothNormals|
		aiProcess_JoinIdenticalVertices|aiProcess_MakeLeftHanded;

	if(format != VertexFormat_PosNormalUV) flags |= aiProcess_CalcTangentSpace;

	const aiScene *scene = importer.ReadFileFromMemory(contents, size, flags);

	bool no_err = (scene && scene->mFlags != AI_SCENE_FLAGS_INCOMPLETE && scene->mRootNode); 

	if(!no_err) {
		const char *err = importer.GetErrorString();
		kh_assert(err);
	}

	kh_assert(scene->HasMeshes());

	// TODO(flo): support animations
	kh_assert(!scene->HasAnimations());


	// TODO(flo): support multiple meshes
	kh_assert(scene->mNumMeshes == 1);


	// aiNode *root = scene->mRootNode;

	// aiNode *child = root->mChildren[0];

	// u32 mesh_index = child->mMeshes[0];

	// aiMesh *mesh = scene->mMeshes[mesh_index];

	aiMesh *mesh = scene->mMeshes[0];
	kh_assert(mesh);

	res.format         = format;
	res.tri_count      = mesh->mNumFaces;
	res.indices_count  = mesh->mNumFaces * 3;
	res.vertices_count = mesh->mNumVertices;


	switch(format) {
		case VertexFormat_PosNormalUV : {
			u32 vertex_size = sizeof(Vertex_PNU);

			res.vertices = (u8 *)kh_push(mem_stack, res.vertices_count * vertex_size);

			u8 *dst = res.vertices;
			for(u32 i = 0; i < res.vertices_count; ++i) {

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
		case VertexFormat_PosNormalTangentUV : {
			u32 vertex_size = sizeof(Vertex_PNUT);

			res.vertices = (u8 *)kh_push(mem_stack, res.vertices_count * vertex_size);

			u8 *dst = res.vertices;
			for(u32 i = 0; i < res.vertices_count; ++i) {
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
		case VertexFormat_PosNormalTangentBitangentUV : {
			u32 vertex_size = sizeof(Vertex_PNUTB);

			res.vertices = (u8 *)kh_push(mem_stack, res.vertices_count * vertex_size);

			u8 *dst = res.vertices;
			for(u32 i = 0; i < res.vertices_count; ++i) {
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
	}

	res.indices = (u8 *)kh_push(mem_stack, res.indices_count * sizeof(u32));

	u8 *dst = res.indices;
	for(u32 i = 0; i < res.tri_count; ++i) {
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