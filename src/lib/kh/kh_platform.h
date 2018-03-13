#ifndef KH_PLATFORM_H

#include "kh_types.h"

#define GENERATE_STRINGS_ARR(STR) #STR,

// ---------------------------
// NOTE(flo): MEMORY
// --------------------------

typedef struct MemoryBlock *platform_virtual_alloc(umm size, u64 flags);
typedef void platform_virtual_free(struct MemoryBlock *ptr);
typedef u8 *PlatformHeapAlloc(umm size);
typedef void PlatformHeapFree(u8 *ptr);
typedef u8 *PlatformHeapReAlloc(u8 *ptr, umm size);

#define kh_malloc(size) g_platform.heap_alloc(size)
#define kh_free(ptr) g_platform.heap_free(ptr)
#define kh_realloc(ptr, size) g_platform.heap_realloc(ptr, size)
#define kh_new(type) (type *)g_platform.heap_alloc(sizeof(type))

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

typedef struct ProgramPath {
	char path[MAX_PATH_BUFFER_SIZE];
	char *program_name;
	usize path_len;
} ProgramPath;

typedef struct FileHandle {
	b32 error;
	void *platform_file_handle;
} FileHandle;

typedef struct FileGroupOfType {
	u32 file_count;
	void *platform_file_group;
} file_group;

enum FileType {
	FileType_assets,
	FileType_count,
};

enum FileAccessFlags {
	FileAccess_read = 0x1,
	FileAccess_write = 0x2,
};

enum FileCreation {
	FileCreation_only_open,
	FileCreation_only_create,
	FileCreation_create_or_open,
	FileCreation_override,
};

struct FileContents {
	u32 size;
	void *memory;
};

#define file_error(file_hdl) file_hdl->error

typedef FileHandle platform_open_file(char *file_name, u32 file_access, FileCreation creation);
typedef void platform_close_file(struct FileHandle *hdl);
typedef void platform_read_bytes_of_file(struct FileHandle *hdl, u64 offset, u64 size, void *dst);
typedef u64 platform_write_bytes_to_file(struct FileHandle *hdl, u64 offset, u64 size, void *buffer);
typedef u32 platform_get_file_size(struct FileHandle *hdl);

// ---------------------------
// NOTE(flo): INPUTS
// --------------------------


enum ButtonFlags {
	Button_down,
	Button_pressed,
	Button_up,
};

enum MouseButton {
	MouseButton_left,
	MouseButton_right,
	MouseButton_middle,
	MouseButton_b4,
	MouseButton_b5,

	MouseButton_count,
};

typedef struct ButtonState {
	u32 down_count;
	b32 down;
} ButtonState;

enum ModifierButton {
	Modifier_shift,
	Modifier_ctrl,
	Modifier_alt,
	Modifier_count,
};

enum Button {
	Button_move_up,
	Button_move_down,
	Button_move_left,
	Button_move_right,
	Button_action_up,
	Button_action_down,
	Button_action_left,
	Button_action_right,
	Button_count,
};

typedef struct Input {
	ButtonState mouse_buttons[MouseButton_count];
	f32 mouse_x, mouse_y;
	f32 mouse_rx, mouse_ry;
	f32 delta_mouse_x, delta_mouse_y;
	i32 dt_wheel;
	ButtonState buttons[Button_count];
	b32 modifier_down[Modifier_count];

	ButtonState debug_buttons[256];
} Input;

inline b32
is_down(ButtonState b_state) {
	b32 res = (b_state.down);
	return(res);
}

inline b32
was_pressed(ButtonState b_state) {
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


#ifdef KH_IN_DEVELOPMENT
	typedef void LoadTex2dFile(struct AssetContents *contents, struct LinearArena *arena, 
	                           void *file_contents, u32 file_size);
	typedef void LoadFontFile(struct AssetContents *contents, struct LinearArena *arena, 
	                          void *file_contents, u32 file_size, 
	                          u32 first_glyph, u32 last_glyph, f32 pixels_height, u32 texture_size);
	typedef void LoadTriMeshFile(struct AssetContents *contents, struct LinearArena *arena, 
	                             void *file_contents, u32 file_size, enum VertexFormat format);
	typedef void LoadSkeletonFile(struct AssetContents *contents, struct LinearArena *arena, 
	                              void *file_contents, u32 file_size);
	typedef void LoadSkinFile(struct AssetContents *contents, struct LinearArena *arena, 
	                          void *file_contents, u32 file_size);
	typedef void LoadAnimationFile(struct AssetContents *contents, struct LinearArena *arena, 
	                               void *file_contents, u32 file_size);
	typedef void InitAssetLoader(struct Platform *platform);
	typedef void LoadAnimationForSkinFiles(struct AssetContents *contents, struct LinearArena *arena,
	                                       void *file_contents, u32 file_size, 
	                                       void *anim_file_contents, u32 anim_file_size);
#endif


typedef struct Platform {
	platform_open_file *open_file;
	platform_close_file *close_file;
	platform_read_bytes_of_file *read_bytes_of_file;
	platform_write_bytes_to_file *write_bytes_to_file;
	platform_get_file_size *get_file_size;

	platform_virtual_alloc *virtual_alloc;
	platform_virtual_free *virtual_free;
	PlatformHeapAlloc *heap_alloc;
	PlatformHeapFree *heap_free;
	PlatformHeapReAlloc *heap_realloc;

	platform_add_work_to_queue *add_work_to_queue;
	platform_complete_all_queue_works *complete_all_queue_works;

#ifdef KH_IN_DEVELOPMENT
	InitAssetLoader *init_asset_import;
	LoadTex2dFile *load_tex2d_directly;
	LoadFontFile *load_font_directly;
	LoadTriMeshFile *load_trimesh_directly;
	LoadSkeletonFile *load_skeleton_directly;
	LoadSkinFile *load_skin_directly;
	LoadAnimationFile *load_animation_directly;
	LoadAnimationForSkinFiles *load_animation_for_skin_directly;
#endif
} Platform;

typedef struct ProgramMemory {
	struct DebugState *debug_state;
	
	struct ProgramState *program_state;
	struct FrameState *frame_state;

	struct WorkQueue *high_queue;
	struct WorkQueue *low_queue;

	Platform platform;
} ProgramMemory;

typedef void kh_update(ProgramMemory *memory, Input *input, struct RenderManager *render, struct Assets *assets, f32 dt);

extern Platform g_platform;

#define array_count(arr) (sizeof(arr) / sizeof(arr)[0])
#define arr_len(arr) array_count(arr)
#define KH_UINT_TO_PTR(x) ((void *)(uintptr(x)))
#define KH_PTR_TO_UINT(x) ((uintptr)(x))
#define kh_swap_ptr(type, a, b, tmp) type *(tmp) = (a); (a) = (b); (b) = (tmp);
#define kh_swap_val(type, a, b, tmp) type (tmp) = (a); (a) = (b); (b) = (tmp);
// #define KH_ALIGNOF(t) ((char*)(&((struct {char c; t _h;}*)0)->_h) - (char*)0)
// #define KH_ALIGNPTR(x, mask) (kh_uint_to_ptr((kh_ptr_to_uint((u8*)(x) + (mask-1)) & ~(mask-1))))
// #define KH_TEST(type, member, parent) &(((type *)0)->member) - (parent **)0

#define KH_OFFSETOF(type, member) (umm)&(((type*)0)->member)
#define KH_SIZEOF(type, member) sizeof(((type *)0)->member)
#define KH_ARRAYCOUNT(type, member) array_count(((type *)0)->member)
#define KH_ARRAYSIZE(type, member) KH_ARRAYCOUNT(type, member) * KH_SIZEOF(type, member)
#define KH_ALIGN_POW2(val, align) ((val + (align - 1)) & ~(align - 1))
#define KH_ALIGN4(val) ((val + 3) & ~3)
#define KH_ALIGN8(val) ((val + 7) & ~7)
#define KH_ALIGN16(val) ((val + 15) & ~15)

KH_INLINE b32
is_pow2(u32 val) {
	b32 res = ((val & (val - 1)) == 0);
	return(res);
}

#define kh_lu0(name, max) for(u32 name = 0; name < max; ++name)
#define kh_ls0(name, max) for(i32 name = 0; name < max; ++name)
#define kh_lu1(name, max) for(u32 name = 1; name < max; ++name)
#define kh_ls1(name, max) for(i32 name = 1; name < max; ++name)
#define kh_lu(name, min, max) for(u32 name = min; name < max; ++name)
#define kh_ls(name, min, max) for(i32 name = min; name < max; ++name)
#define kh_lnext(type, name, first) for(type *name = first; name; name = name->next)
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

#define GENERATE_STR(NAME) #NAME,
#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_HASHSTR(NAME) hash_key_from_djb2(#NAME),

#if 1
#define kh_assert(expression) if(!(expression)) { *(int *)0 = 0;}
#define NOT_IMPLEMENTED kh_assert(!"not implemented")
#define KH_CHECK_EQ(a, b) kh_assert(a == b)
#define KH_CHECK_LT(a, b) kh_assert(a < b)
#define KH_CHECK_LE(a, b) kh_assert(a <= b)
#define KH_CHECK_GT(a, b) kh_assert(a > b)
#define KH_CHECK_GE(a, b) kh_assert(a >= b)
#else
// TODO(flo): user information
#define kh_assert(...)
#define KH_CHECK_EQ(...)
#define KH_CHECK_LT(...)
#define KH_CHECK_LE(...)
#define KH_CHECK_GT(...)
#define KH_CHECK_GE(...)
#define NOT_IMPLEMENTED
#endif

#define INVALID_DEFAULT_CASE default : { kh_assert(!"invalid type"); }break
#define INVALID_U32_OFFSET 0xFFFFFFFF

// TODO(flo): remove this
#define MAX_STRING_BUFFER_SIZE 4096


#define STB_SPRINTF_IMPLEMENTATION
#include "ext/stb_sprintf.h"
#define kh_printf stbsp_sprintf

#define KH_MATH_IMPLEMENTATION
#include "kh_math.h"
#include "kh_rand.h"
#include "kh_color.h"
#include "kh_memory.h"
#include "kh_strings.h"
#include "kh_hash.h"
#include "kh_sort.h"
#include "kh_multithread.h"
#include "kh_camera.h"
#include "kh_tokenizer.h"
#include "kh_asset_format.h"
#include "kh_asset_file.h"
#include "kh/kh_animation.h"


typedef struct ProgramState {
	LinearArena arena;
}ProgramState;

typedef struct FrameState {
	LinearArena arena;
}FrameState;

#define KH_PLATFORM_H
#endif // KH_PLATFORM_H