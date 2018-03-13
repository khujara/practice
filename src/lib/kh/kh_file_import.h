#ifndef KH_FILE_IMPORT_H
#define KH_FILE_IMPORT_H

/* TODO(flo): FILE IMPORT :
	TEXTURES : JPG, PSD, SPINE, TGA
	3D MODELS : COLLADA, OBJ, FBX, OPENGEX
	AUDIO : WAV, OGG


	FINISH TTF AND PNG LOADERS!

*/
	
struct texture_2d_contents
{
	u32 width;
	u32 height;
	u32 bytes_per_pixel;
	void *memory;

	void *contents;
};

struct font_contents
{

};


/*
---------------------------
	NOTE: PNG
---------------------------
*/

#define PNG_SIGNATURE(a, b, c, d) ((a << 24) | (b << 16) | (c << 8) | (d << 0))
#define CHECK_PNG_SIGNATURE_32_0 0x89504e47
#define CHECK_PNG_SIGNATURE_32_1 0x0d0a1a0a

#define PNG_TYPE(a, b, c, d)  (((a) << 24) | ((b) << 16) | ((c) << 8) | ((d) << 0))

#pragma pack(push, 1)
struct png_signature
{
	u8 e[8];
};

struct png_chunk
{
	u32 data_length;
	u32 type;
};

struct png_ihdr_chunk
{
	u32 width;
	u32 height;
	u8 bit_depth;
	u8 color_type;
	u8 compression;
	u8 filter;
	u8 interlace;
};

struct png_chunk_end
{
	u32 crc;
};
#pragma pack(pop)

KH_INTERN b32
test_png(void *contents)
{
	png_signature *sig = (png_signature *)contents;
	u32 sig_0 = PNG_SIGNATURE(sig->e[0], sig->e[1], sig->e[2], sig->e[3]);
	u32 sig_1 = PNG_SIGNATURE(sig->e[4], sig->e[5], sig->e[6], sig->e[7]);

	b32 res = ((sig_0 == CHECK_PNG_SIGNATURE_32_0) && (sig_1 == CHECK_PNG_SIGNATURE_32_1));
	return(res);
}

KH_INTERN texture_2d_contents
load_png_file(void *contents)
{
	texture_2d_contents res = {};

	res.contents = contents;

	u8 *at = (u8 *)contents;
	png_signature *sig = (png_signature *)at;
	u32 sig_0 = PNG_SIGNATURE(sig->e[0], sig->e[1], sig->e[2], sig->e[3]);
	u32 sig_1 = PNG_SIGNATURE(sig->e[4], sig->e[5], sig->e[6], sig->e[7]);
	at += sizeof(png_signature);
	/* TODO(flo): chunk types in order :
		IHDR, cHRM, gAMA, sBIT, PLTE , bKGD, hIST, tRNS, oFFs, pHYs, sCAL,
		IDAT, tIME, tEXt, zTXt, fRAc, glFg, glFt, glFx, IEND
	*/
	if(sig_0 == CHECK_PNG_SIGNATURE_32_0 && sig_1 == CHECK_PNG_SIGNATURE_32_1)
	{
		u32 pixels_data_count = 0;
		u8 *pixels = 0;
		u32 offset_count = 0;
		b32 parsing = true;
		u32 bit_depth = 0;
		while(parsing)
		{
			png_chunk *chunk = (png_chunk *)at;
			if(chunk)
			{
				at += sizeof(png_chunk);
				u32 data_length = kh_exchange_significant_bits(chunk->data_length);
				u32 type = kh_exchange_significant_bits(chunk->type);
				switch(type)
				{
					case PNG_TYPE('I','H','D','R') :
					{
						kh_assert(data_length == 13);
						png_ihdr_chunk *ihdr_chunk = (png_ihdr_chunk *)at;
						bit_depth = ihdr_chunk->bit_depth;
						u32 width = kh_exchange_significant_bits(ihdr_chunk->width);
						u32 height = kh_exchange_significant_bits(ihdr_chunk->height);
						res.width = width;
						res.height = height;
						at += sizeof(png_ihdr_chunk);
					} break;
					case PNG_TYPE('I', 'D', 'A', 'T') :
					{
						pixels = (u8 *)realloc(pixels, data_length + pixels_data_count);
						u8 *dst = pixels + pixels_data_count;
						for(u32 ind = 0; ind < data_length; ++ind)
						{
							*dst++ = at[ind];
						}
						pixels_data_count += data_length;
						at += data_length;
					} break;
					case PNG_TYPE('I', 'E', 'N', 'D') :
					{
						parsing = false;
						at += data_length;
					} break;
					case PNG_TYPE('P', 'L', 'T', 'E') :
					{
						at += data_length;
					} break;
					default :
					{
						at += data_length;
					} break;
				}
				at += sizeof(png_chunk_end);
			}
			else
			{
				parsing = false;
			}
		}
		// res.data_length = pixels_data_count;
		// TODO(flo): filtering
		u32 bytes_per_line = (res.width * bit_depth + 7) / 8;
		u32 img_n = 1024;
		u32 raw = bytes_per_line * res.height * img_n + res.height;
		// TODO(flo): inflate algorithm
		res.memory = pixels;
		// free(Pixels);
	}
	else
	{
		kh_assert(!"PNG FILE NOT IDENTIFIED");
	}
	return(res);
}

KH_INTERN void
save_to_png_file()
{

}

/*
---------------------------
	NOTE: BMP
---------------------------
*/


#define BMP_FILE_SIGNATURE 0x4D42

#pragma pack(push, 1)
struct BMPFileHeader
{
	u16 file_type;
	u32 file_size;
	u16 reserved_1;
	u16 reserved_2;
	u32 bitmap_offset;
};

struct BMPv3Header
{
	u32 size;
	i32 width;
	i32 height;
	u16 planes;
	u16 bits_per_pixel;

	u32 compression;
	u32 size_of_bitmap;
	i32 hor_resolution;
	i32 vert_resolution;
	u32 colors_used;
	u32 colors_imp;
};

struct BMPv3BitfieldMask
{
	u32 red_mask;
	u32 green_mask;
	u32 blue_mask;
};
#pragma pack(pop)

KH_INTERN b32
test_bmp(void *contents)
{
	BMPFileHeader *file_head = (BMPFileHeader *)contents;
	b32 res = (file_head->file_type == BMP_FILE_SIGNATURE);
	return(res);
}

KH_INTERN texture_2d_contents
load_bmp_file(void *contents)
{
	texture_2d_contents res = {};

	res.contents = contents;

	BMPFileHeader *file_head = (BMPFileHeader *)contents;
	// TODO(flo): how to support other version than v3 windows NT?
	BMPv3Header *header = (BMPv3Header *)((u8 *)contents + sizeof(BMPFileHeader));
	BMPv3BitfieldMask *bitfield_masks = (BMPv3BitfieldMask *)((u8 *)header + sizeof(BMPv3Header));

	u32 *pixels = (u32 *)((u8 *)contents + file_head->bitmap_offset);

	res.memory = pixels;
	res.width = header->width;
	res.height = header->height;
	res.bytes_per_pixel = header->bits_per_pixel / 8;
	kh_assert(res.height >= 0);

	u32 red_mask = bitfield_masks->red_mask;
	u32 green_mask = bitfield_masks->green_mask;
	u32 blue_mask = bitfield_masks->blue_mask;
	u32 alpha_mask = ~(red_mask | green_mask | blue_mask);

	bit_scan red_scan = kh_find_least_significant_set_bit_U32(red_mask);
	bit_scan green_scan = kh_find_least_significant_set_bit_U32(green_mask);
	bit_scan blue_scan = kh_find_least_significant_set_bit_U32(blue_mask);
	bit_scan alpha_scan = kh_find_least_significant_set_bit_U32(alpha_mask);

	kh_assert(red_scan.found);
	kh_assert(green_scan.found);
	kh_assert(blue_scan.found);
	kh_assert(alpha_scan.found);

	i32 red_shift_right = (i32)red_scan.index;
	i32 green_shift_right = (i32)green_scan.index;
	i32 blue_shift_right = (i32)blue_scan.index;
	i32 alpha_shift_right = (i32)alpha_scan.index;

	u32 *src = pixels;

	for(u32 y = 0; y < res.height; ++y)
	{
		for(u32 x = 0; x < res.width; ++x)
		{
			u32 color = *src;

			v4 texel = {(f32)((color & red_mask) >> red_shift_right),
				(f32)((color & green_mask) >> green_shift_right),
				(f32)((color & blue_mask) >> blue_shift_right),
				(f32)((color & alpha_mask) >> alpha_shift_right)};
			// NOTE(flo): first pass gamma correction (this is a power of 2 instead of ^2.33 or ^2.2)
			f32 inverse_255 = 1.0f / 255.0f;
			texel.r = kh_sqr_f32(inverse_255 * texel.r);
			texel.g = kh_sqr_f32(inverse_255 * texel.g);
			texel.b = kh_sqr_f32(inverse_255 * texel.b);
			texel.a = inverse_255 * texel.a;

			texel.rgb *= texel.a;

			// NOTE(flo): last pass gamma correction
	    f32 float_255 = 255.0f;
	    texel.r = float_255*kh_sqrt_f32(texel.r);
	    texel.g = float_255*kh_sqrt_f32(texel.g);
	    texel.b = float_255*kh_sqrt_f32(texel.b);
	    texel.a = float_255*texel.a;

			*src++ = (((u32)(texel.a + 0.5f) << 24) |
			((u32)(texel.r + 0.5f) << 16) |
			((u32)(texel.g + 0.5f) << 8) |
			((u32)(texel.b + 0.5f) << 0));
		}
	}
	return(res);
}


KH_INTERN void
test_export_to_bmp(char *filename, umm size, i32 w, i32 h, void *ptr, LinearAllocator *memstack)
{
	file_handle dst_f = win32_open_file(filename, memstack, FileAccess_write, FileCreation_create_or_open);

	const u32 bmp_size = sizeof(BMPFileHeader) + sizeof(BMPv3Header) + sizeof(BMPv3BitfieldMask);
	umm total_size = size + bmp_size;

	BMPFileHeader out_header = {};
	out_header.file_type = BMP_FILE_SIGNATURE;
	out_header.file_size = (u32)size + 122; //size + 138 ?
	out_header.bitmap_offset = 122; //138?

	BMPv3Header out_v3_header = {};
	out_v3_header.size = 108; //124
	out_v3_header.width = w;
	out_v3_header.height = h;
	out_v3_header.planes = 1;
	out_v3_header.bits_per_pixel = 3 * 8;
	out_v3_header.compression = 3;
	out_v3_header.size_of_bitmap = (u32)size;
	out_v3_header.hor_resolution = 2835;
	// out_v3_header.hor_resolution = 2835;

	BMPv3BitfieldMask out_mask;
	out_mask.red_mask = 	0x00FF0000;
	out_mask.green_mask = 	0x0000FF00;
	out_mask.blue_mask = 	0x000000FF;

	u8 pad[122 - bmp_size];

	for(u32 i = 0; i < sizeof(pad); ++i)
	{
		pad[i] = 0;
	}

	win32_write_bytes_to_file(&dst_f, 0, sizeof(BMPFileHeader), &out_header);
	win32_write_bytes_to_file(&dst_f, sizeof(BMPFileHeader), sizeof(BMPv3Header), &out_v3_header);
	win32_write_bytes_to_file(&dst_f, sizeof(BMPFileHeader) + sizeof(BMPv3Header), 
	                          sizeof(BMPv3BitfieldMask), &out_mask);
	win32_write_bytes_to_file(&dst_f, bmp_size, sizeof(pad), &pad);
	win32_write_bytes_to_file(&dst_f, out_header.bitmap_offset, size, ptr);
}


/*
---------------------------
	NOTE: TTF
---------------------------
*/

#if _WIN32
#include <windows.h>
#endif

// TODO(flo): do we want ti pack set of codepoints into one bitmap ? use rect_pack from stb

// "c:/Windows/Fonts/arial.ttf"
// TODO(flo): load font
KH_INTERN void
load_font(void *contents)
{
}

/*
---------------------------
	NOTE: OBJ
---------------------------
*/

struct obj_index
{
	u32 pos_index;
	u32 uv_index;
	u32 normal_index;
};

struct obj_model
{
	u32 obj_indices_count;
	u32 pos_count;
	u32 uv_count;
	u32 normal_count;

	obj_index *indices;
	v3 *positions;
	v2 *uvs;
	v3 *normals;
};

struct indexed_model
{
	u32 count;
	u32 indices_count;

	u8 *elements;
	u32 *indices;

	u32 interleave;
};

struct triangle_mesh_contents
{
	u32 triangle_count;
	indexed_model model;
};

struct indexed_node
{
	u32 pos_ind;
	u32 uv_ind;
	u32 norm_ind;

	u32 out_ind; 

	indexed_node *next_in_hash;
};

#define VERTEX_LIMIT_PER_MESH_OBJECT 65535
#define MAX_INDEXED_HASH (1 << 14)

static indexed_model
to_indexed_model(obj_model *obj, LinearAllocator *tmp, LinearAllocator *mem, u32 interleave)
{
	indexed_model res = {};

	indexed_node **hash = (indexed_node **)kh_push(tmp, (1 << 14) * sizeof(indexed_node *));

	u32 pos_size = VERTEX_LIMIT_PER_MESH_OBJECT * sizeof(v3);
	u32 uv_size = VERTEX_LIMIT_PER_MESH_OBJECT * sizeof(v2);
	u32 normal_size = VERTEX_LIMIT_PER_MESH_OBJECT * sizeof(v3);
	u32 el_size = pos_size + uv_size + normal_size;

	res.elements = (u8 *)kh_push(mem, el_size);

	res.indices = kh_push_array(mem, VERTEX_LIMIT_PER_MESH_OBJECT, u32);
	res.interleave = interleave;

	u32 new_index = 0;

	// TODO(flo): we need some kind of hashing on our obj_index structure
	for(u32 i = 0; i < obj->obj_indices_count; ++i)
	{
		obj_index *in_index = obj->indices + i;

		u32 in_pos_index = in_index->pos_index;
		u32 in_uv_index = in_index->uv_index;
		u32 in_norm_index = in_index->normal_index;

		u32 hash_id = (in_pos_index << 8) + (in_uv_index << 4) + (in_norm_index << 0);
		u32 hash_slot = hash_id & (MAX_INDEXED_HASH - 1);

		indexed_node **el = hash + hash_slot;

		indexed_node *find = 0;
		for(indexed_node *search = *el; search; search = search->next_in_hash)
		{
			if(search->pos_ind == in_pos_index && search->uv_ind == in_uv_index && search->norm_ind == in_norm_index)
			{
				find = search;
				break;
			}
		}

		if(find)
		{
			u32 out_ind = find->out_ind;
			res.indices[res.indices_count++] = out_ind;
		}
		else
		{
			u32 count = res.count++;
			u32 index = new_index++;

			if(interleave > 0)
			{
				u32 offset = count * (sizeof(v3) + sizeof(v2) + sizeof(v3));
				v3 *pos = (v3 *)(res.elements + offset);
				*pos = obj->positions[in_pos_index];

				v2 *uv = (v2 *)((u8 *)pos + sizeof(v3));
				*uv = obj->uvs[in_uv_index];

				v3 *normal = (v3 *)((u8 *)uv + sizeof(v2));
				*normal = obj->normals[in_norm_index];
			}
			else
			{
				v3 *pos = (v3 *)(res.elements + (count * sizeof(v3)));
				*pos = obj->positions[in_pos_index];

				v2 *uv = (v2 *)(res.elements + pos_size + (count * sizeof(v2)));
				*uv = obj->uvs[in_uv_index];

				v3 *normal = (v3 *)(res.elements + pos_size + uv_size + (count * sizeof(v3)));
				*normal = obj->normals[in_norm_index];
			}

			// res.positions[count] = obj->positions[in_pos_index];
			// res.uvs[count] = obj->uvs[in_uv_index];
			// res.normals[count] = obj->normals[in_norm_index];
			res.indices[res.indices_count++] = index;

			find = kh_push_struct(tmp, indexed_node);
			find->pos_ind = in_pos_index;
			find->uv_ind = in_uv_index;
			find->norm_ind = in_norm_index;
			find->out_ind = index;
			find->next_in_hash = *el;
			*el = find;
		}
	}
	return(res);
}

inline void
get_vector_from_tokens(string_tokenizer *tokenizer, f32 *src, u32 vec_len)
{
	u32 i;
	for(i = 0; i < vec_len; ++i)
	{
		f32 val;
		token number = get_token_and_next(tokenizer);
		if(token_fit(number, Token_minus))
		{
			token dec = get_token_and_next(tokenizer);
			kh_assert(token_fit(dec, Token_decimal))
			val = token_to_f32(dec);
			val = -val;
		}
		else
		{
			kh_assert(token_fit(number, Token_decimal))
			val = token_to_f32(number);
		}

		src[i] = val;
	}
}

inline void
get_indices_from_tokens(string_tokenizer *tokenizer, obj_model *dst)
{
	u32 i;
	for(i = 0; i < 3; ++i)
	{
		obj_index *index = dst->indices + dst->obj_indices_count++;

		token ind_vert = get_token_and_next(tokenizer);
		kh_assert(token_fit(ind_vert, Token_numeric));
		index->pos_index = token_to_u32(ind_vert) - 1;

		token slash_tok = get_token_and_next(tokenizer);
		kh_assert(token_fit(slash_tok, Token_slash));

		token ind_tex_coord = get_token_and_next(tokenizer);
		kh_assert(token_fit(ind_tex_coord, Token_numeric));
		index->uv_index = token_to_u32(ind_tex_coord) - 1;

		slash_tok = get_token_and_next(tokenizer);
		kh_assert(token_fit(slash_tok, Token_slash));

		token ind_normal = get_token_and_next(tokenizer);
		kh_assert(token_fit(ind_normal, Token_numeric));
		index->normal_index = token_to_u32(ind_normal) - 1;
	}
}


KH_INTERN triangle_mesh_contents
load_obj(LinearAllocator *memstack, void *contents, u32 interleaved = 0)
{
	LinearAllocator tmp = {};

	triangle_mesh_contents res = {};

	obj_model obj = {};

	obj.positions = kh_push_array(&tmp, VERTEX_LIMIT_PER_MESH_OBJECT, v3);
	obj.uvs = kh_push_array(&tmp, VERTEX_LIMIT_PER_MESH_OBJECT, v2);
	obj.normals = kh_push_array(&tmp, VERTEX_LIMIT_PER_MESH_OBJECT, v3);
	obj.indices = kh_push_array(&tmp, VERTEX_LIMIT_PER_MESH_OBJECT, obj_index);

	string_tokenizer tokenizer = {};
	tokenizer.pos = (char *)contents;
	b32 is_parsing = true;
	while(is_parsing)
	{
		token cur_token = get_token_and_next(&tokenizer);
		switch(cur_token.type)
		{
			case Token_end_of_file :
			{
				is_parsing = false;
			} break;
			case Token_word :
			{
				if(word_fit(cur_token, "v"))
				{
					f32 pos[3];
					get_vector_from_tokens(&tokenizer, pos, array_count(pos));
					v3 vec = kh_vec3(pos[0], pos[1], pos[2]);
					obj.positions[obj.pos_count++] = vec;
				}
				else if(word_fit(cur_token, "vt"))
				{
					f32 uv[2];
					get_vector_from_tokens(&tokenizer, uv, array_count(uv));
					v2 vec = kh_vec2(uv[0], uv[1]);
					obj.uvs[obj.uv_count++] = vec;
				}
				else if(word_fit(cur_token, "vn"))
				{
					f32 normal[3];
					get_vector_from_tokens(&tokenizer, normal, array_count(normal));
					v3 vec = kh_vec3(normal[0], normal[1], normal[2]);
					obj.normals[obj.normal_count++] = vec;
				}
				else if(word_fit(cur_token, "f"))
				{
					get_indices_from_tokens(&tokenizer, &obj);
					res.triangle_count++;
				}
			} break;
			default :
			{

			} break;
		}
	}

	res.model = to_indexed_model(&obj, &tmp, memstack, interleaved);
	kh_clear(&tmp);
	return(res);
}

/*
---------------------------
	NOTE: GLSL Shader
---------------------------
*/


#endif //KH_FILE_IMPORT_H