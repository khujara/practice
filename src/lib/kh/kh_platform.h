#ifndef KH_PLATFORM_H

#include "kh_types.h"

#define GENERATE_STRINGS_ARR(STR) #STR,

// ---------------------------
// NOTE(flo): MEMORY
// --------------------------

typedef struct SharedAllocator *platform_virtual_alloc(umm size, u64 flags);
typedef void platform_virtual_free(struct SharedAllocator *ptr);
// typedef void *platform_heap_alloc(umm size);
// typedef void platform_heap_free(void *ptr);

// ---------------------------
// NOTE(flo): FILE I/O
// --------------------------

// TODO(flo): remove this once we have implemented our own string builder function (prinft equivalent)
#include <stdio.h>
/* TODO(flo): asm file i/o
	http://www.tutorialspoint.com/assembly_programming/assembly_file_management.htm
	win32 docs :
	https://msdn.microsoft.com/en-us/library/windows/desktop/aa365467(v=vs.85).aspx
	linux docs :
	http://linux.die.net/man/2/open

*/

#define MAX_PATH_BUFFER_SIZE 2048

typedef struct ProgramPath
{
	char path[MAX_PATH_BUFFER_SIZE];
	char *program_name;
	usize path_len;
} ProgramPath;

typedef struct FileHandle
{
	b32 error;
	void *platform_file_handle;
} FileHandle;

typedef struct FileGroupOfType
{
	u32 file_count;
	void *platform_file_group;
} file_group;

enum FileType
{
	FileType_assets,
	FileType_fonts,
	FileType_output_materials,
	FileType_output_nodegraph,
	FileType_output_shader,

	FileType_count,
};

enum FileAccessFlags
{
	FileAccess_read = 0x1,
	FileAccess_write = 0x2,
};

enum FileCreation
{
	FileCreation_only_open,
	FileCreation_only_create,
	FileCreation_create_or_open,
	FileCreation_override,
};

#define file_error(file_hdl) file_hdl->error

typedef FileGroupOfType platform_get_all_files_of_type(FileType type, struct StackAllocator *memstack);
typedef void platform_close_file_group(struct FileGroupOfType *file_group);
typedef FileHandle platform_open_next_file_of_type(struct FileGroupOfType *file_group, struct StackAllocator *memstack);
typedef FileHandle platform_open_file(char *file_name, struct StackAllocator *memstack, u32 file_access, FileCreation creation);
typedef void platform_close_file(struct FileHandle *hdl);
typedef void platform_read_bytes_of_file(struct FileHandle *hdl, u64 offset, u64 size, void *dst);
typedef void platform_write_bytes_to_file(struct FileHandle *hdl, u64 offset, u64 size, void *buffer);
typedef u32 platform_get_file_size(struct FileHandle *hdl);

// ---------------------------
// NOTE(flo): INPUTS
// --------------------------

enum MouseButton
{
	MouseButton_left,
	MouseButton_right,
	MouseButton_middle,
	MouseButton_b4,
	MouseButton_b5,

	MouseButton_count,
};

typedef struct ButtonState
{
	u32 down_count;
	b32 down;
	b32 went_down;
} ButtonState;

typedef struct Input
{
	ButtonState mouse_buttons[MouseButton_count];
	f32 mouse_x, mouse_y;
	f32 delta_mouse_x, delta_mouse_y;
	i32 dt_wheel;
} Input;

inline b32
was_pressed(ButtonState b_state)
{
	b32 res = ((b_state.down_count > 1) || ((b_state.down_count == 1) && (b_state.down)));
	return(res);
}

// ---------------------------
// NOTE(flo): MULTITHREADING
// --------------------------

#define JOB_ENTRY_POINT(name) void name(struct WorkQueue *queue, void *user_data)
typedef JOB_ENTRY_POINT(job_entry_point);
typedef void platform_add_work_to_queue(struct WorkQueue *queue, job_entry_point *callback, void *user_data);
typedef void platform_complete_all_queue_works(struct WorkQueue *queue);

// ---------------------------
// NOTE(flo): SHARED
// --------------------------

typedef struct Platform
{
	platform_get_all_files_of_type *get_all_files_of_type;
	platform_close_file_group *close_file_group;
	platform_open_next_file_of_type *open_next_file_of_type;
	platform_open_file *open_file;
	platform_close_file *close_file;
	platform_read_bytes_of_file *read_bytes_of_file;
	platform_write_bytes_to_file *write_bytes_to_file;
	platform_get_file_size *get_file_size;

	platform_virtual_alloc *virtual_alloc;
	platform_virtual_free *virtual_free;
	// platform_heap_alloc *heap_alloc;
	// platform_heap_free *heap_free;

	platform_add_work_to_queue *add_work_to_queue;
	platform_complete_all_queue_works *complete_all_queue_works;
} Platform;

typedef struct ProgramMemory
{
	struct ProgramState *p_state;
	struct FrameState *f_state;

	struct WorkQueue *high_queue;
	struct WorkQueue *low_queue;

	Platform platform;

} ProgramMemory;

typedef void kh_update(ProgramMemory *memory, Input *input, struct RenderManager *render, struct Assets *assets, f32 dt);
extern Platform g_platform;

#define KH_UINT_TO_PTR(x) ((void *)(uintptr(x)))
#define KH_PTR_TO_UINT(x) ((uintptr)(x))
#define KH_SWAP(a, b, type, tmp) type *(tmp) = (a); (a) = (b); (b) = (tmp);
// #define KH_ALIGNOF(t) ((char*)(&((struct {char c; t _h;}*)0)->_h) - (char*)0)
// #define KH_ALIGNPTR(x, mask) (kh_uint_to_ptr((kh_ptr_to_uint((u8*)(x) + (mask-1)) & ~(mask-1))))
// #define KH_TEST(type, member, parent) &(((type *)0)->member) - (parent **)0
#define KH_OFFSETOF(type, member) (umm)&(((type*)0)->member)
#define KH_ALIGN_POW2(val, align) ((val + (align - 1)) & ~(align - 1))
#define KH_ALIGN4(val) ((val + 3) & ~3)
#define KH_ALIGN8(val) ((val + 7) & ~7)
#define KH_ALIGN16(val) ((val + 15) & ~15)

#define KH_FLU(max) for(u32 i = 0; i < max; ++i)
#define KH_FLUS(index, max) for(u32 index = 0; index < max; ++index)
#define KH_FLU1(max) for(u32 i = 0; i < max; ++i)
#define KH_FLUS1(index, max) for(u32 index = 1; index < max; ++index)
// #define GET_EMUM_FROM_STRING_(STR) Sample_##STR
// #define GET_ENUM_FROM_STRING(STR) GET_EMUM_FROM_STRING_(STR)
// #define	PLATFORM_FUNCTION_(plat_prefix, func_name, ...) plat_prefix##func_name(## __VA_ARGS__)
// #define PLATFORM_FUNCTION(plat_prefix, func_name, ...) PLATFORM_FUNCTION_(plat_prefix, func_name, ## __VA_ARGS__)


//typedef char *kh_va_list;
//#define KH_ADDRESSOF(v) (&(v))
//#define KH_INTSIZEOF(v) ((sizeof(v) + sizeof(int) - 1) & ~(sizeof(int) - 1))
//#define KH_VA_START(ap, v) ((void)(ap = (kh_va_list)KH_ADDRESSOF(v) + KH_INTSIZEOF(v)))
//#define KH_VA_ARG(ap, t) (*(t *)((ap += KH_INTSIZEOF(t)) - KH_INTSIZEOF(t)))
//#define KH_VA_END(ap) ((void)ap = (kh_va_list)0))

// TODO(flo): remove this
#define MAX_STRING_BUFFER_SIZE 4096

#define STB_SPRINTF_IMPLEMENTATION
#include "ext/stb_sprintf.h"

#define KH_MATH_IMPLEMENTATION
#include "kh_math.h"
#include "kh_asset_format.h"
#include "kh_memory.h"
#include "kh_strings.h"
#include "kh_data_structures.h"
#include "kh_multithread.h"
#include "kh_camera.h"
#include "kh_tokenizer.h"

typedef struct ProgramState
{
	StackAllocator stack;
}ProgramState;

typedef struct FrameState
{
	struct Assets *asset_arr;
	StackAllocator stack;
}FrameState;

// TODO(flo): remove this
enum MaterialType {
	Material_rendertarget,
	Material_phong,
	Material_normalmap,
	Material_shadowmap,
	Material_skybox,
	Material_notexture,
	Material_zprepass,
	Material_count,
};

#define KH_PLATFORM_H
#endif // KH_PLATFORM_H