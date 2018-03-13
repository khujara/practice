struct Win32WndDim {
	i32 w;
	i32 h;
};

struct Win32SchedHandle {
	HANDLE semaphore;
};

struct Win32FileGroupOfType {
	HANDLE hdl;
	WIN32_FIND_DATAW data;
};

struct Win32Allocator {
	MemoryBlock arena;
	Win32Allocator *prev;
	Win32Allocator *next;
	u32 pad[4];
};

KH_INTERN void
win32_toggle_full_screen(HWND window, WINDOWPLACEMENT *window_pos) {
  // NOTE(Flo): Raymond Chen's on msdn oldnewthing blog

	DWORD style = GetWindowLong(window, GWL_STYLE);
	if (style & WS_OVERLAPPEDWINDOW) {
		MONITORINFO monitor_info = { sizeof(MONITORINFO) };
		if (GetWindowPlacement(window, window_pos) && 
		    GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &monitor_info)) {
			SetWindowLong(window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(window, HWND_TOP, monitor_info.rcMonitor.left, monitor_info.rcMonitor.top, 
			             monitor_info.rcMonitor.right - monitor_info.rcMonitor.left, 
			             monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	} else {
		SetWindowLong(window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(window, window_pos);
		SetWindowPos(window, NULL, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
			SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
}

KH_INTERN void
win32_execute_sys_cmd(char *path, char *cmd, char *cmd_line) {
	STARTUPINFO startup_info = {};
	startup_info.cb = sizeof(startup_info);
	startup_info.dwFlags = STARTF_FORCEOFFFEEDBACK;
	// StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
  // StartupInfo.wShowWindow = SW_HIDE;

	PROCESS_INFORMATION process_info = {};
	if(CreateProcess(cmd, cmd_line, 0, 0, FALSE, 0, 0, path, &startup_info, &process_info))	{
		// TODO(flo): handle the optimistic case
	} else {
		DWORD error_code = GetLastError();
	}
}

inline f32
win32_get_seconds_elapsed(LARGE_INTEGER start, LARGE_INTEGER end, f64 inverse_perf_freq) {
	f32 Result = ((f32)(end.QuadPart - start.QuadPart) * (f32)inverse_perf_freq);
	return(Result);
}

inline ProgramPath
win32_get_program_path() {
	ProgramPath res = {};

	GetModuleFileNameA(NULL, res.path, sizeof(res.path));
	res.program_name = res.path;
	for(char *scan = res.path; *scan; ++scan) {
		if(*scan == '\\') {
			res.program_name = scan + 1;
		}
	}

	res.path_len = res.program_name - res.path;

	return(res);
}

KH_GLOBAL Win32Allocator g_alloc_sentinel = {};
KH_GLOBAL TicketMutexLockk g_alloc_mutex = {};
KH_GLOBAL u32 g_alloc_count = {};
KH_GLOBAL u32 g_free_count = {};

static void
win32_init_allocator_sentinel() {
	g_alloc_sentinel.next = &g_alloc_sentinel;
	g_alloc_sentinel.prev = &g_alloc_sentinel;
}

#define MEMORY_CHECK_OVERFLOW 0
#define MEMORY_CHECK_UNDERFLOW 0

// TODO(flo): bound checking
static MemoryBlock *
win32_virtual_alloc(umm size, u64 flags) {
	if(!g_alloc_sentinel.next) {
		kh_assert(!g_alloc_sentinel.prev);
		win32_init_allocator_sentinel();
	}

	// TODO(flo): two choices : we need to push the Win32Allocator at the end of the ptr given by the virtual allocation or if we push it at the beginning it must be aligned with our cache line alignment of our allocation (2nd option here)
	kh_assert(sizeof(Win32Allocator) == 64);

	umm total_size = size + sizeof(Win32Allocator);
	umm base_offset = sizeof(Win32Allocator);

#if MEMORY_CHECK_UNDERFLOW
	umm page_size = 4096;
	total_size = size + 2*page_size;
	base_offset = 2*page_size;
	umm protect_offset = page_size;	
#elif MEMORY_CHECK_OVERFLOW
	umm page_size = 4096;
	umm size_rounded_up = KH_ALIGN_POW2(size, page_size);
	total_size = size_rounded_up + 2*page_size;
	base_offset = page_size + size_rounded_up - size;
	umm protect_offset = page_size + size_rounded_up;
#endif

	Win32Allocator *alloc = (Win32Allocator *)VirtualAlloc(0, total_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

	alloc->arena.base = (u8 *)alloc + base_offset;
	alloc->arena.size = size;
	//NOTE(flo): virtual alloc must be initialised to zero;
	kh_assert(alloc->arena.prev == 0);
	kh_assert(alloc->arena.used == 0);

#if MEMORY_CHECK_UNDERFLOW || MEMORY_CHECK_OVERFLOW
	DWORD old_prot = 0;
	BOOL prot = VirtualProtect((u8 *)alloc + protect_offset, page_size, PAGE_NOACCESS, &old_prot);
	kh_assert(prot);
#endif

	Win32Allocator *sentinel = &g_alloc_sentinel;
	alloc->next = sentinel;

	begin_mutex_lock(&g_alloc_mutex);
	alloc->prev = sentinel->prev;
	alloc->prev->next = alloc;
	alloc->next->prev = alloc;
	end_mutext_lock(&g_alloc_mutex);

	MemoryBlock *res = &alloc->arena;

	g_alloc_count++;
	return(res);
}

static void
win32_virtual_free(MemoryBlock *alloc) {
	Win32Allocator *win32_alloc = ((Win32Allocator *)alloc);
	begin_mutex_lock(&g_alloc_mutex);
	win32_alloc->prev->next = win32_alloc->next;
	win32_alloc->next->prev = win32_alloc->prev;
	end_mutext_lock(&g_alloc_mutex);
	BOOL res = VirtualFree(alloc, 0, MEM_RELEASE);
	kh_assert(res);
	g_free_count++;
}

static u8 *
win32_heap_alloc(umm size) {
	HANDLE hdl = GetProcessHeap();	
	u8 *res = (u8 *)HeapAlloc(hdl, 0, size);
	return(res);
}

static void
win32_heap_free(u8 *ptr) {
	HANDLE hdl = GetProcessHeap();	
	BOOL freed = HeapFree(hdl, 0, ptr);
	kh_assert(freed);
}

static u8 *
win32_heap_realloc(u8 *ptr, umm size) {
	HANDLE hdl = GetProcessHeap();	
	u8 *res = (u8 *)HeapReAlloc(hdl, 0, ptr, size);
	return(res);
}

inline Win32WndDim
win32_get_wnd_dim(HWND window) {
	Win32WndDim res;
	RECT client_rect;
	GetClientRect(window, &client_rect);
	res.w = client_rect.right - client_rect.left;
	res.h = client_rect.bottom - client_rect.top;
	return(res);
}

KH_INTERN Win32SchedHandle
win32_create_thread_handle(u32 thread_count) {
	Win32SchedHandle res;
	u32 count = 0;
	res.semaphore = CreateSemaphoreEx(0, count, thread_count, 0, 0, SEMAPHORE_ALL_ACCESS);
	return(res);
}

KH_INTERN void
win32_add_work_to_queue(WorkQueue *work_queue, job_entry_point *callback, void *user_data) {
	u32 new_next_task_to_write = (work_queue->next_task_to_write + 1) & SCHED_TASK_MASK;
	kh_assert(new_next_task_to_write != work_queue->next_task_to_read);
	WorkQueueTask *task = work_queue->tasks + work_queue->next_task_to_write;
	task->callback = callback;
	task->user_data = user_data;
	++work_queue->end;
	work_queue->next_task_to_write = new_next_task_to_write;

	Win32SchedHandle *hdl = (Win32SchedHandle *)work_queue->platform_handle;
	ReleaseSemaphore(hdl->semaphore, 1, 0); 
}

KH_INTERN b32
win32_do_next_queue_work(WorkQueue *work_queue) {
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
win32_complete_all_queue_works(WorkQueue *work_queue) {
	while(work_queue->current != work_queue->end) {
		win32_do_next_queue_work(work_queue);
	}
	work_queue->current = 0;
	work_queue->end = 0;
}

DWORD WINAPI
win32_thread_proc(LPVOID lp_param) {
	WorkQueue *work_queue = (WorkQueue *)lp_param;

	kh_assert(get_thread_id() == GetCurrentThreadId());
	for(;;)	{
		if(win32_do_next_queue_work(work_queue)) {
			Win32SchedHandle *hdl = (Win32SchedHandle *)work_queue->platform_handle;
			WaitForSingleObjectEx(hdl->semaphore, INFINITE, false);
		}
	}
}

KH_INTERN void
win32_create_sched_queue(WorkQueue *work_queue, Win32SchedHandle *handle, u32 thread_count) {
	work_queue->current            = 0;
	work_queue->end                = 0;
	work_queue->next_task_to_write = 0;
	work_queue->next_task_to_read  = 0;
	work_queue->platform_handle    = handle;

	for(u32 t_i = 0; t_i < thread_count; ++t_i)	{
		DWORD thread_id;
		HANDLE thread_h = CreateThread(0, 0, win32_thread_proc, work_queue, 0, &thread_id);
		CloseHandle(thread_h);
	}
}

KH_INTERN void
win32_file_error(FileHandle *file_hdl, char *message) { //, void *params) {
	// win32_file_error *error = (win32_file_error *)params;
	OutputDebugStringA("win32 file error : ");
	OutputDebugStringA(message);
	OutputDebugStringA("\n");
	file_hdl->error = true;
}

KH_INTERN FileGroupOfType
win32_get_all_files_of_type(FileType f_type) {
	FileGroupOfType res = {};
	Win32FileGroupOfType *win32_file_group = 
		(Win32FileGroupOfType *)VirtualAlloc(0, sizeof(Win32FileGroupOfType), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	res.platform_file_group = win32_file_group;

	wchar_t *str_type = L"*.*";
	switch(f_type) {
		case FileType_assets : {
			str_type = L"*.kh";
		} break;
		default : {
			kh_assert("invalid file type");
		} break;
	}
	// NOTE(flo): just for counting files
	WIN32_FIND_DATAW find_data;
	HANDLE file_hdl = FindFirstFileW(str_type, &find_data);
	while(file_hdl != INVALID_HANDLE_VALUE)	{
		++res.file_count;
		if(!FindNextFileW(str_type, &find_data)) break;
	}
	FindClose(file_hdl);

	win32_file_group->hdl = (void *)FindFirstFileW(str_type, &win32_file_group->data);
	return(res);
}

KH_INTERN void
win32_close_file_group(FileGroupOfType *file_group) {
	Win32FileGroupOfType *win32_file_group = (Win32FileGroupOfType *)file_group->platform_file_group;
	if(win32_file_group) {
		FindClose(win32_file_group->hdl);
		VirtualFree(win32_file_group, 0, MEM_RELEASE);
	}
}

KH_INTERN FileHandle
win32_open_next_file_of_type(FileGroupOfType *file_group) {
	Win32FileGroupOfType *win32_file_group = (Win32FileGroupOfType *)file_group->platform_file_group;
	FileHandle res = {};

	if(win32_file_group->hdl != INVALID_HANDLE_VALUE) {
		wchar_t *filename = win32_file_group->data.cFileName;
		HANDLE fhdl = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		res.platform_file_handle = (void *)fhdl;
		if(fhdl == INVALID_HANDLE_VALUE) {
			win32_file_error(&res, "unable to find the file");
		}
		if(!FindNextFileW(fhdl, &win32_file_group->data)) {
			FindClose(fhdl);
			fhdl = INVALID_HANDLE_VALUE;
		}
	}
	return(res);
}

KH_INTERN FileHandle
win32_open_file(char *file_name, u32 file_access, FileCreation creation) {
	FileHandle res = {};

	DWORD flags = (file_access & FileAccess_read) ? GENERIC_READ : 0;
	b32 test = (file_access & FileAccess_write);
	flags = (file_access & FileAccess_write) ? flags|GENERIC_WRITE : flags;

	DWORD creation_disp = OPEN_EXISTING;
	switch(creation) {
		case FileCreation_only_open : { creation_disp = OPEN_EXISTING; } break;
		case FileCreation_only_create : { creation_disp = CREATE_NEW; } break;
		case FileCreation_create_or_open : { creation_disp = OPEN_ALWAYS; } break;
		case FileCreation_override : { creation_disp = CREATE_ALWAYS; } break;
	}


	HANDLE fhdl = CreateFile(file_name, flags, FILE_SHARE_READ, 0, creation_disp, FILE_ATTRIBUTE_NORMAL, 0);
	kh_assert(sizeof(HANDLE) <= sizeof(res.platform_file_handle));
	res.platform_file_handle = (void *)fhdl;
	if(fhdl == INVALID_HANDLE_VALUE) {
		win32_file_error(&res, "unable to find the file");
	}
	return(res);
}

KH_INTERN void
win32_close_file(FileHandle *hdl) {
	HANDLE win32_hdl = (HANDLE)hdl->platform_file_handle;
	CloseHandle(win32_hdl);
}

inline u32
win32_get_file_size(FileHandle *hdl) {
	HANDLE win32_hdl = (HANDLE)hdl->platform_file_handle;
	DWORD s = GetFileSize(win32_hdl, NULL);
	u32 res = (u32)s;
	return(res);
}


KH_INTERN void
win32_read_bytes_of_file(FileHandle *file_hdl, u64 offset, u64 size, void *dst) {
	if(!file_error(file_hdl)) {
		OVERLAPPED overlapped = {};
		overlapped.Offset = (u32)(offset & 0xFFFFFFFF);
		overlapped.OffsetHigh = (u32)((offset >> 32) & 0xFFFFFFFF);
		HANDLE win32_hdl = (HANDLE)file_hdl->platform_file_handle;
		DWORD bytes_read;
		u32 size32 = kh_safe_truncate_u64_to_u32(size);
		if(ReadFile(win32_hdl, dst, size32, &bytes_read, &overlapped) && (size32 == bytes_read)) {
		} else {
			win32_file_error(file_hdl, "unable to read file");
		}
	}
}

KH_INTERN u64
win32_write_bytes_to_file(FileHandle *file_hdl, u64 offset, u64 size, void *buffer) {
	u64 res = 0;
	if(!file_error(file_hdl)) {
		OVERLAPPED overlapped = {};
		overlapped.Offset = (u32)(offset & 0xFFFFFFFF);
		overlapped.OffsetHigh = (u32)((offset >> 32) & 0xFFFFFFFF);
		HANDLE win32_hdl = (HANDLE)file_hdl->platform_file_handle;
		DWORD bytes_written;
		u32 size32 = kh_safe_truncate_u64_to_u32(size);
		if(WriteFile(win32_hdl, buffer, size32, &bytes_written, &overlapped) && (size32 == bytes_written)) {
			res = size32;
		} else {
			win32_file_error(file_hdl, "unable to write file");
		}
	}
	return(res);
}

struct Win32DebugTimer {
	f64 inv_freq;
	LARGE_INTEGER start;
	char txt[64];

	Win32DebugTimer(char *str) {
		OutputDebugStringA(str);

		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		inv_freq = 1.0f / (f64)freq.QuadPart;
		QueryPerformanceCounter(&start);

	}

~Win32DebugTimer() {
		LARGE_INTEGER end;
		QueryPerformanceCounter(&end);
		f32 us = win32_get_seconds_elapsed(start, end, inv_freq);
		stbsp_sprintf(txt, "%fms, %ffps \n", us * 1000.0f, 1.0f / us);
		OutputDebugStringA(txt);
	}
};

KH_INLINE void
win32_set_platform(Platform *platform) {
	platform->add_work_to_queue = win32_add_work_to_queue;
	platform->complete_all_queue_works = win32_complete_all_queue_works;
	platform->open_file = win32_open_file;
	platform->close_file = win32_close_file;
	platform->get_file_size = win32_get_file_size;
	platform->read_bytes_of_file = win32_read_bytes_of_file;
	platform->write_bytes_to_file = win32_write_bytes_to_file;
	platform->virtual_alloc = win32_virtual_alloc;
	platform->virtual_free = win32_virtual_free;
	platform->heap_alloc = win32_heap_alloc;
	platform->heap_free = win32_heap_free;
	platform->heap_realloc = win32_heap_realloc;
}
