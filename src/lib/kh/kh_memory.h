#ifndef KH_MEMORY_H
#define KH_MEMORY_H

/* TODO(flo):
	Types of memory blocks :
		- pre allocated size stack (what we have right now)
		- dynamic grow stack (same as above with chain list storing for the blocks)
		- fixed size allocation (just like megatexture stuff use free list for that)
		- general purpose allocation when we want "sub freeing" (the stacks above only allow lifo frees) relocatable heap
		- bound checking
		- sub arena (a parent that can free his "children" arenas and that can virtual free when all of their "children" have no memory)
*/


#define kh_push(stack, size, ...) kh_push_size_(stack, size, ## __VA_ARGS__)
#define kh_push_struct(stack, type, ...) (type *)kh_push_size_(stack, sizeof(type), ## __VA_ARGS__)
#define kh_push_array(stack, count, type, ...) (type *)kh_push_size_(stack, sizeof(type)*count, ## __VA_ARGS__)
// #define kh_pop(stack, size, ...) kh_pop_(stack, size, ## __VA_ARGS__)
// #define kh_pop_struct(s) kh_free_memory(sizeof(&s), s)
// #define kh_pop_array(count, ptr) kh_free_memory(count*sizeof((ptr)[0]), ptr)
#define kh_copy(stack, size, source, ...) kh_copy_(size, source, kh_push_(stack, size, ## __VA_ARGS__))

#define OFFSET_TO_PREV_ALIGN(val, align) (val & (align - 1))
#define OFFSET_TO_NEXT_ALIGN(val, align) (align - (val & (align - 1)))
#define DEFAULT_MEMORY_ALIGNMENT 4
#define MINIMUM_ALLOC_SIZE 1024*1024
#define MEMORY_PAGE_SIZE 4096

enum MemoryAllocationFlags
{
	MemoryAllocation_clear_to_zero = 0x1,
	MemoryAllocation_bound_cheking_left = 0x2,
	MemoryAllocation_bound_cheking_right = 0x4,
	MemoryAllocation_not_growable = 0x8,
};

struct MemoryAllocationParams
{
	u32 flags;
	u32 align;
};

struct SharedAllocator
{
	umm used;
	umm size;
	u8 *base;
	SharedAllocator *prev;
};

// @TODO(flo): this is not a stack allocator but more a linear dynamic allocator
// @TODO(flo): find another way to do bound checking!
struct StackAllocator
{
	SharedAllocator *cur; 
	u32 tmp_count;
};

struct DoubleBufferAllocator
{
	u8 *base;

	umm start;
	umm end;
	u32 left_tmp;
	u32 right_tmp;
};

struct PoolAllocator
{
	SharedAllocator *cur;
	umm data_size;
	umm next_free;
	u32 align;
};

// TODO(flo): implement this
struct GeneralPurposeAllocator
{
	u8 *base;
	umm size;
	umm used;
};

struct TransientStack
{
	StackAllocator *stack;
	SharedAllocator *alloc;
	umm used;
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

inline MemoryAllocationParams
memory_not_growable()
{
	MemoryAllocationParams res = {};
	res.align = DEFAULT_MEMORY_ALIGNMENT;
	res.flags |= MemoryAllocation_not_growable;
}

inline MemoryAllocationParams
memory_not_growable(u32 align)
{
	MemoryAllocationParams res = {};
	res.align = align;
	res.flags |= MemoryAllocation_not_growable;
}

inline umm
kh_get_aligned_offset(SharedAllocator *alloc, umm align)
{
	umm res = 0;
	umm ptr = (umm)alloc->base + alloc->used;
	umm mask = align - 1;
	if(ptr & mask)
	{
		res = align - (ptr & mask);
	}
	return(res);
}

inline umm
kh_get_aligned_offset(umm ptr, umm align)
{
	umm res = 0;
	umm mask = align - 1;
	if(ptr & mask)
	{
		res = align - (ptr & mask);
	}
	return(res);
}

inline umm
kh_get_aligned_size(SharedAllocator *alloc, umm init_size, u32 align)
{
	umm res = init_size;
	umm offset = kh_get_aligned_offset(alloc, align);
	res += offset;
	return(res);
}

inline umm
kh_get_aligned_size(umm ptr, umm init_size, u32 align)
{
	umm res = init_size;
	umm offset = kh_get_aligned_offset(ptr, align);
	res += offset;
	return(res);
}

// TODO(flo): need optimization!
inline void
kh_zero_clear(umm size, void *ptr)
{
    u8 *byte = (u8 *)ptr;
    while(size--)
    {
        *byte++ = 0;
    }
}

inline void *
kh_push_size_(StackAllocator *memstack, umm init_size, MemoryAllocationParams params = default_params())
{
	void *res = 0;

	umm size = 0;

	SharedAllocator *shared = memstack->cur;
	if(shared)
	{
		size = kh_get_aligned_size(shared, init_size, params.align);
	}

	if(!shared || ((shared->used + size > shared->size) && !(params.flags & MemoryAllocation_not_growable)))
	{
		size = init_size;
		umm final_size = kh_max_umm(size, (umm)MINIMUM_ALLOC_SIZE);
		SharedAllocator *alloc = g_platform.virtual_alloc(final_size, params.flags);
		alloc->prev = memstack->cur;
		memstack->cur = alloc;
		shared = memstack->cur;
	}

	kh_assert(shared->used + size <= shared->size);
	umm align_offset = kh_get_aligned_offset(shared, params.align);
	res = shared->base + shared->used + align_offset;
	shared->used += size;

	kh_assert(size >= init_size);

	return(res);
}

/* TODO(flo): 
	if we have something in the free list -> pick the first free element in the free list and remove it from the
		linked list, add the object to the pool.
	else if we have nothing in the free list -> it means we should have no fragementations, so just check that the
		"farthest pointer" from the pool is less than the size of the pool, add object to the pool and increment
		the "farthest pointer".
*/
inline void *
kh_pool_create(PoolAllocator *pool, umm init_data_size, u32 init_count = 0, MemoryAllocationParams params = default_params())
{
	void *res = 0;

	umm data_size = KH_ALIGN_POW2(init_data_size, (umm)params.align);

	kh_assert(data_size >= sizeof(void *));

	umm offset = 0;
	SharedAllocator *shared = pool->cur;
	if(shared)
	{
		kh_assert(data_size == pool->data_size);
	}

	if(!shared || (shared->used + data_size > shared->size))
	{
		umm count = MINIMUM_ALLOC_SIZE / data_size;
		umm final_size = kh_max_umm(data_size * init_count, data_size * count);
		SharedAllocator *alloc = g_platform.virtual_alloc(final_size, params.flags);
		alloc->prev = pool->cur;
		pool->cur = alloc;
		pool->data_size = data_size;
		pool->align = params.align;
		shared = pool->cur;
	}

	umm next_free = 0;
	if(pool->next_free)
	{
		res = shared->base + pool->next_free;
	}
	else
	{
		res = shared->base + shared->used;
	}
	pool->next_free = *(umm *)res;

	shared->used += data_size;

	kh_assert(pool->next_free < shared->size);
	kh_assert(pool->next_free % data_size == 0);

	kh_assert(shared->used + data_size <= shared->size);

	kh_assert(data_size >= init_data_size);

	return(res);

}

inline void *
kh_pack(GeneralPurposeAllocator *alloc, u32 init_size, MemoryAllocationParams params = default_params())
{
	umm size = kh_get_aligned_size((umm)(alloc->base + alloc->used), init_size, params.align);
	kh_assert((alloc->used + size) <= alloc->size);

	umm offset = kh_get_aligned_offset((umm)(alloc->base + alloc->used), params.align);
	void *res = alloc->base + alloc->used + offset;
	alloc->used += size;

	kh_assert(size >= init_size);
	return(res);
}

inline char *
kh_push_string(StackAllocator *memstack, char *src)
{
	u32 size = 1;
	for(char *at = src; *at; ++at)
	{
		++size;
	}

	char *dst = (char *)kh_push(memstack, size);
	for(u32 c_i = 0; c_i < size; ++c_i)
	{
		dst[c_i] = src[c_i];
	}

	return(dst);
}

inline void
kh_remove(PoolAllocator *pool, void *to_remove, u32 size)
{
	umm data_size = KH_ALIGN_POW2(size, pool->align);
	kh_assert(data_size == pool->data_size);

	SharedAllocator *shared = pool->cur;

	*(umm *)to_remove = pool->next_free;
	pool->next_free = (umm)to_remove - (umm)shared->base;

	kh_assert(pool->next_free < shared->size);
	kh_assert(pool->next_free % pool->data_size == 0);
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

inline void
kh_free(StackAllocator *memstack)
{
	SharedAllocator *to_free = memstack->cur;
	memstack->cur = to_free->prev;
	g_platform.virtual_free(to_free);
}

inline void
kh_clear(StackAllocator *memstack)
{
	while(memstack->cur)
	{
		kh_free(memstack);
	}
}

inline TransientStack
kh_begin_transient(StackAllocator *memstack)
{
	TransientStack res;

	if(!memstack->cur) {
		kh_push_size_(memstack, 0);
	}

	res.stack = memstack;
	res.alloc = memstack->cur;
	res.used = memstack->cur ? memstack->cur->used : 0;

	memstack->tmp_count++;

	return(res);
}

inline void
kh_end_transient(const TransientStack *tmp)
{
	StackAllocator *memstack = tmp->stack;
	while(memstack->cur != tmp->alloc)
	{
		kh_free(memstack);
	}
	if(memstack->cur)
	{
		kh_assert(memstack->cur->used >= tmp->used);
		memstack->cur->used = tmp->used;
		kh_assert(memstack->tmp_count > 0);
	}
	memstack->tmp_count--;
}

// #ifdef __cplusplus
// }
// #endif //__cplusplus

#endif // KH_MEMORY_H
