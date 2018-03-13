#include "stdlib.h"
#include "stdio.h"
#include "SDL.h"

struct SDLSchedHandle {
	SDL_sem *semaphore;
};

KH_INTERN SDLSchedHandle
sdl_create_thread_handle(u32 thread_count) {
	SDLSchedHandle res;
	u32 count = 0;
	res.semaphore = SDL_CreateSemaphore(thread_count);
	return(res);
}

KH_INTERN void
sdl_add_work_to_queue(WorkQueue *work_queue, job_entry_point *callback, void *user_data) {
	u32 new_next_task_to_write = (work_queue->next_task_to_write + 1) & SCHED_TASK_MASK;
	kh_assert(new_next_task_to_write != work_queue->next_task_to_read);
	WorkQueueTask *task = work_queue->tasks + work_queue->next_task_to_write;
	task->callback = callback;
	task->user_data = user_data;
	++work_queue->end;
    SDL_CompilerBarrier();
	work_queue->next_task_to_write = new_next_task_to_write;

	SDLSchedHandle *hdl = (SDLSchedHandle *)work_queue->platform_handle;
	SDL_SemPost(hdl->semaphore);
}

KH_INTERN b32
sdl_do_next_queue_work(WorkQueue *work_queue) {
	b32 no_task_remaining = false;
	u32 orig_next_task_to_read = work_queue->next_task_to_read;
	u32 new_next_task_to_read = (orig_next_task_to_read + 1) & SCHED_TASK_MASK;
	if(orig_next_task_to_read != work_queue->next_task_to_write) {
		u32 ind = interlocked_compare_exchange_u32(&work_queue->next_task_to_read, new_next_task_to_read, 
			orig_next_task_to_read);
		if(ind == orig_next_task_to_read) {
			WorkQueueTask task = work_queue->tasks[ind];
			task.callback(work_queue, task.user_data);
			interlocked_increment_u32(&work_queue->current);
		}
	} else {
		no_task_remaining = true;
	}
	return(no_task_remaining);
}

KH_INTERN void
sdl_complete_all_queue_works(WorkQueue *work_queue) {
	while(work_queue->current != work_queue->end) {
		sdl_do_next_queue_work(work_queue);
	}
	work_queue->current = 0;
	work_queue->end = 0;
}

int 
sdl_thread_proc(void *param) {
	WorkQueue *work_queue = (WorkQueue *)param;
	for(;;) {
		if(sdl_do_next_queue_work(work_queue)) {
			SDLSchedHandle *hdl = (SDLSchedHandle *)work_queue->platform_handle;
			SDL_SemWait(hdl->semaphore);	
		}
	}
}

KH_INTERN void
sdl_create_sched_queue(WorkQueue *work_queue, SDLSchedHandle *handle, u32 thread_count) {
	work_queue->current            = 0;
	work_queue->end                = 0;
	work_queue->next_task_to_write = 0;
	work_queue->next_task_to_read  = 0;
	work_queue->platform_handle    = handle;
	for(u32 t_i = 0; t_i < thread_count; ++t_i)	{
        SDL_Thread *thread_h = SDL_CreateThread(sdl_thread_proc, 0, work_queue);
        SDL_DetachThread(thread_h);
	}	
}

KH_INTERN FileHandle
sdl_open_file(char *file_name, u32 file_access, FileCreation creation) {
	FileHandle res = {};
	char *param = (file_access & FileAccess_write) ? "wb" : "rb";
	FILE *f = fopen(file_name, param);
	res.error = f ? 0 : 1;
	res.platform_file_handle = (void *)f;
	return(res);
}

KH_INTERN void
sdl_close_file(FileHandle *hdl) {
	FILE *f = (FILE *)hdl->platform_file_handle; 
	fclose(f);
}

inline u32
sdl_get_file_size(FileHandle *hdl) {
	FILE *f = (FILE *)hdl->platform_file_handle; 
	fseek(f, 0, SEEK_END);
	u32 res = ftell(f);
	return(res);
}

KH_INTERN void
sdl_read_bytes_of_file(FileHandle *file_hdl, u64 offset, u64 size, void *dst) {
	FILE *f = (FILE *)file_hdl->platform_file_handle;
	fseek(f, offset, SEEK_SET);
	fread(dst, size, 1, f);
}

KH_INTERN u64
sdl_write_bytes_to_file(FileHandle *file_hdl, u64 offset, u64 size, void *buffer) {
	FILE *f = (FILE *)file_hdl->platform_file_handle;
	fseek(f, offset, SEEK_SET);
	fwrite(buffer, size, 1, f);
	f32 res = offset + size;
	return(res);
}

struct SDLAllocator {
	MemoryBlock arena;
	u32 pad[8];
};

KH_INTERN MemoryBlock *
sdl_virtual_alloc(umm size, u64 flags) {
	kh_assert(sizeof(SDLAllocator) == 64);
	umm total_size = size + sizeof(SDLAllocator);
	umm base_offset = sizeof(SDLAllocator);

	SDLAllocator *alloc = (SDLAllocator *)calloc(1, total_size);

	alloc->arena.base = (u8 *)alloc + base_offset;
	alloc->arena.size = size;

	MemoryBlock *res = &alloc->arena;
	return(res);
}

static void
sdl_virtual_free(MemoryBlock *alloc) {
	free(alloc);	
}


static u8 *
sdl_heap_alloc(umm size) {
	u8 *res = (u8 *)malloc(size);
	return(res);
}

static void
sdl_heap_free(u8 *ptr) {
	free(ptr);
}

static u8 *
sdl_heap_realloc(u8 *ptr, umm size) {
	u8 *res = (u8 *)realloc(ptr, size);
	return(res);
}

KH_INLINE void
sdl_set_platform(Platform *platform) {
	platform->add_work_to_queue = sdl_add_work_to_queue;
	platform->complete_all_queue_works = sdl_complete_all_queue_works;
	platform->open_file = sdl_open_file;
	platform->close_file = sdl_close_file;
	platform->get_file_size = sdl_get_file_size;
	platform->read_bytes_of_file = sdl_read_bytes_of_file;
	platform->write_bytes_to_file = sdl_write_bytes_to_file;
	platform->virtual_alloc = sdl_virtual_alloc;
	platform->virtual_free = sdl_virtual_free;
	platform->heap_alloc = sdl_heap_alloc;
	platform->heap_free = sdl_heap_free;
	platform->heap_realloc = sdl_heap_realloc;
}
