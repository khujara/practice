#ifndef KH_MULTITHREAD_H

// TODO(flo): implement clang support
#define SCHED_TASK_SIZE 1024
#define SCHED_TASK_MASK (SCHED_TASK_SIZE - 1)

typedef struct WorkQueueTask {
	job_entry_point *callback;
	void *user_data;
}WorkQueue_job;

typedef struct WorkQueue {
	u32 volatile current;
	u32 volatile end;

	u32 volatile next_task_to_write;
	u32 volatile next_task_to_read;  

	void *platform_handle;

	WorkQueueTask tasks[SCHED_TASK_SIZE];
}WorkQueue;

typedef struct TaskWithMemory {
	b32 unuse;
	LinearArena arena;
	TransientLinear tmp;
}TaskWithMemory;

typedef struct TicketMutexLock {
	u64 volatile ticket;
	u64 volatile now_serving;
}TicketMutexLockk;

#if defined(_AMD64_)
#define PROCESSOR_MEMORY_BARRIER __fastorefence()
#elif defined(_IA64_)
#define PROCESSOR_MEMORY_BARRIER __mf()
#endif //_AMD64_

#if defined(_MSC_VER)
#include <intrin.h>

#define COMPILER_READ_BARRIER _ReadBarrier()
#define COMPILER_WRITE_BARRIER _WriteBarrier()

inline u32
interlocked_compare_exchange_u32(u32 volatile *val, u32 new_val, u32 expected) {
	u32 res = _InterlockedCompareExchange((long volatile *)val, new_val, expected);
	return(res);
}

inline u64
interlocked_compare_exchange_u64(u64 volatile *val, u64 new_val, u64 expected) {
	u64 res = _InterlockedCompareExchange64((__int64 volatile *)val, new_val, expected);
	return(res);
}

inline u8
interlocked_compare_exchange_m128(__m128 volatile *val, __m128 new_val, __m128 *expected) {
	u64 low = ((u64 *)val)[0];
	u64 high = ((u64 *)val)[1];
	u8 res = _InterlockedCompareExchange128((__int64 volatile *)val, high, low, (__int64 *)expected);
}


inline u32
interlocked_add_u32(u32 volatile *val, u32 add) {
	u32 res = (_InterlockedExchangeAdd((long volatile *)val, add));
}

inline u64
interlocked_exchange_u64(u64 volatile *val, u64 new_val) {
	u64 res = _InterlockedExchange64((__int64 volatile *)val, new_val);
	return(res);
}
inline u64
interlocked_add_u64(u64 volatile *val, u64 add) {
	u64 res = (_InterlockedExchangeAdd64((__int64 volatile *)val, add));
	return(res);
}

inline u32 interlocked_increment_u32(u32 volatile *add) {
	u32 res = _InterlockedIncrement((long volatile *)add);
	return(res);
}

inline u64 interlocked_increment_u32(u64 volatile *add) {
	u64 res = _InterlockedIncrement64((__int64 volatile *)add);
	return(res);
}

inline u32 get_thread_id() {
	u8 *thread_storage = (u8 *)__readgsqword(0x30);
	u32 thread_id = *(u32 *)(thread_storage + 0x48);
	return(thread_id);
}

#elif COMPILE_LLVM
#define WAIT_FOR_READ asm volatile("" ::: "memory")
#define WAIT_FOR_WRITE asm volatile("" ::: "memory");

inline u32
interlocked_compare_exchange_u32(u32 volatile *val, u32 new_val, u32 expected) {
	u32 res = __sync_val_compare_and_swap(val, expected, new_val);
	return(res);
}
#else
#error "COMPILER NOT SUPPORTED"
#endif //MSVC_VER

KH_INTERN TaskWithMemory *
begin_sched_task_with_memory(TaskWithMemory *task_arr, u32 count) {
	TaskWithMemory *res = 0;
	for(u32 t_i = 0; t_i < count; ++t_i) {
		TaskWithMemory *task = task_arr + t_i;
		if(task->unuse)	{
			res = task;
			res->unuse = false;
			res->tmp = kh_begin_transient(&res->arena);
			break;
		}
	}

	return(res);
}

KH_INTERN void
end_sched_task_with_memory(TaskWithMemory *task) {
	kh_end_transient(&task->tmp);
	COMPILER_WRITE_BARRIER;
	task->unuse = true;
}

inline void
begin_mutex_lock(TicketMutexLockk *mutex) {
	u64 ticket = interlocked_add_u64(&mutex->ticket, 1);
	while(ticket != mutex->now_serving) { _mm_pause(); }
}

inline void 
end_mutext_lock(TicketMutexLockk *mutex) {
	interlocked_add_u64(&mutex->now_serving, 1);
}



#define KH_MULTITHREAD_H
#endif //KH_MULTITHREAD_H
