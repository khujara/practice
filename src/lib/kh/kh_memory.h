#ifndef KH_MEMORY_H
#define KH_MEMORY_H

/* TODO(flo):
	Types of memory blocks :
		- pre allocated size stack (what we have right now)
		- dynamic grow stack (same as above with chain list storing for the blocks)
		- fixed size allocation (just like megatexture stuff use free list for that)
		- general purpose allocation when we want "sub freeing" (the stacks above only allow lifo frees) relocatable heap
		- sub arena (a parent that can free his "children" arenas and that can virtual free when all of their "children" have no memory)
*/


#define kh_push(arena, size, ...) kh_push_size_(arena, size, ## __VA_ARGS__)
#define kh_push_struct(arena, type, ...) (type *)kh_push_size_(arena, sizeof(type), ## __VA_ARGS__)
#define kh_push_array(arena, count, type, ...) (type *)kh_push_size_(arena, sizeof(type)*count, ## __VA_ARGS__)
// #define kh_pop(arena, size, ...) kh_pop_(arena, size, ## __VA_ARGS__)
// #define kh_pop_struct(s) kh_free_memory(sizeof(&s), s)
// #define kh_pop_array(count, ptr) kh_free_memory(count*sizeof((ptr)[0]), ptr)
#define kh_copy(arena, size, source, ...) kh_copy_(size, source, kh_push_(arena, size, ## __VA_ARGS__))

#define OFFSET_TO_PREV_ALIGN(val, align) (val & (align - 1))
#define OFFSET_TO_NEXT_ALIGN(val, align) (align - (val & (align - 1)))
#define DEFAULT_MEMORY_ALIGNMENT 4
#define MINIMUM_ALLOC_SIZE 1024*1024

enum MemoryAllocationFlags
{
	MemoryAllocation_clear_to_zero = 0x1,
	MemoryAllocation_bound_cheking_left = 0x2,
	MemoryAllocation_bound_cheking_right = 0x4,
};

struct MemoryAllocationParams
{
	u32 flags;
	u32 align;
};

struct MemoryBlock {
	umm used;
	umm size;
	u8 *base;
	MemoryBlock *prev;
};

struct LinearArena {
	MemoryBlock *cur_block;
	u32 tmp_count;
};

struct TransientLinear {
	LinearArena *arena;
	MemoryBlock *block;
	umm start;
};

inline MemoryAllocationParams
default_params()
{
	MemoryAllocationParams res = {};
	res.align = DEFAULT_MEMORY_ALIGNMENT;
	return(res);
}

inline MemoryAllocationParams
align(u32 align)
{
	MemoryAllocationParams res = {};
	res.align = align;
	return(res);
}

inline umm
kh_get_aligned_size(umm init_size, u32 align) {
	kh_assert(is_pow2(align));
	umm res = KH_ALIGN_POW2(init_size, (umm)align);
	return(res);
}

inline MemoryBlock *
kh_get_memory_block(MemoryBlock *block, umm size)
{
	MemoryBlock *res = block;
	// TODO(flo): if block see if there is room available in prev arenas?
	if (!block || ((block->used + size) > block->size)) {
		umm final_size = kh_max_umm(size, (umm)MINIMUM_ALLOC_SIZE);
		MemoryBlock *new_block = g_platform.virtual_alloc(final_size, 0);
		new_block->prev = block;
		res = new_block;
	}
	return(res);
}

static MemoryBlock *
kh_free_memory(MemoryBlock *block) {
	MemoryBlock *res = block->prev;
	g_platform.virtual_free(block);
	return(res);
}

// TODO(flo): need optimization!
inline void
kh_zero_clear(umm size, void *ptr) {
    u8 *byte = (u8 *)ptr;
    while(size--) {
        *byte++ = 0;
    }
}

inline void *
kh_copy_(umm size, void *src_ptr, void *dst_ptr)
{
	u8 *src = (u8 *)src_ptr;
	u8 *dst = (u8 *)dst_ptr;
	while(size--)
	{
		*dst++ = *src++;
	}
	return(dst_ptr);
}

static void
kh_reserve_size(LinearArena *arena, umm init_size) {
	if(init_size) {
		umm size = kh_get_aligned_size(init_size, DEFAULT_MEMORY_ALIGNMENT);
		MemoryBlock *block = kh_get_memory_block(arena->cur_block, size);
		arena->cur_block = block;
		block->used = 0;
	}
}

static void *
kh_push_size_(LinearArena *arena, umm init_size) {
	umm size = kh_get_aligned_size(init_size, DEFAULT_MEMORY_ALIGNMENT);
	MemoryBlock *block = kh_get_memory_block(arena->cur_block, size);
	arena->cur_block = block;
	kh_assert(block->used + size <= block->size);
	void *res = block->base + block->used;
	block->used += size;
	kh_assert(size >= init_size);
	return(res);
}

static void
kh_clear(LinearArena *arena) {
	while (arena->cur_block) {
		arena->cur_block = kh_free_memory(arena->cur_block);
	}
}

inline TransientLinear
kh_begin_transient(LinearArena *arena) {
	TransientLinear res;

	if (!arena->cur_block) {
		kh_push_size_(arena, 0);
	}

	res.arena = arena;
	res.block = arena->cur_block;
	res.start = arena->cur_block ? arena->cur_block->used : 0;

	arena->tmp_count++;

	return(res);
}

inline void
kh_end_transient(const TransientLinear *tmp, b32 clear = false) {
	LinearArena *arena = tmp->arena;
	while (arena->cur_block != tmp->block) {
		arena->cur_block = kh_free_memory(arena->cur_block);
	}
	if (arena->cur_block)
	{
		kh_assert(arena->cur_block->used >= tmp->start);
		if(clear) {
			MemoryBlock *block = arena->cur_block;
			kh_assert(tmp->start <= block->used);
			umm size = block->used - tmp->start;
			kh_zero_clear(size, block->base + tmp->start);
		}
		arena->cur_block->used = tmp->start;
		kh_assert(arena->tmp_count > 0);
	}
	arena->tmp_count--;
}

#define boot_strap_push_struct(type, member) (type *)boot_strap_push_size_(sizeof(type), KH_OFFSETOF(type, member))
inline void *
boot_strap_push_size_(umm struct_size, umm offset_to_arena) {
	LinearArena arena = {};
	void *str = kh_push(&arena, struct_size); 
	*(LinearArena *)((u8 *)str + offset_to_arena) = arena;
	return(str);
}

#endif // KH_MEMORY_H
