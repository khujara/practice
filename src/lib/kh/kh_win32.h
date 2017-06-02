struct win32_wnd_dim
{
	i32 w;
	i32 h;
};

struct win32_sched_handle
{
	HANDLE semaphore;
};

struct win32_file_handle
{
	HANDLE hdl;
};

struct win32_file_group_of_type
{
	HANDLE hdl;
	WIN32_FIND_DATAW data;
};

struct win32_allocator
{
	SharedAllocator shared;
	win32_allocator *prev;
	win32_allocator *next;
	u64 pad[2];
};

KH_INTERN void
win32_toggle_full_screen(HWND window, WINDOWPLACEMENT *window_pos)
{
  // NOTE(Flo): Raymond Chen's on msdn oldnewthing blog

	DWORD style = GetWindowLong(window, GWL_STYLE);
	if (style & WS_OVERLAPPEDWINDOW)
	{
		MONITORINFO monitor_info = { sizeof(MONITORINFO) };
		if (GetWindowPlacement(window, window_pos) &&
			GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &monitor_info))
		{
			SetWindowLong(window, GWL_STYLE,
				style & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(window, HWND_TOP,
				monitor_info.rcMonitor.left, monitor_info.rcMonitor.top,
				monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
				monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
				SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else
	{
		SetWindowLong(window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(window, window_pos);
		SetWindowPos(window, NULL, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
			SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
}

KH_INTERN void
win32_execute_sys_cmd(char *path, char *cmd, char *cmd_line)
{
	STARTUPINFO startup_info = {};
	startup_info.cb = sizeof(startup_info);
	startup_info.dwFlags = STARTF_FORCEOFFFEEDBACK;
	// StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
  // StartupInfo.wShowWindow = SW_HIDE;

	PROCESS_INFORMATION process_info = {};
	if(CreateProcess(cmd, cmd_line, 0, 0, FALSE, 0, 0, path, &startup_info, &process_info))
	{
		// TODO(flo): handle the optimistic case
	}
	else
	{
		DWORD error_code = GetLastError();
	}
}

inline f32
win32_get_seconds_elapsed(LARGE_INTEGER start, LARGE_INTEGER end, f64 inverse_perf_freq)
{
	f32 Result = ((f32)(end.QuadPart - start.QuadPart) * (f32)inverse_perf_freq);
	return(Result);
}

inline ProgramPath
win32_get_program_path()
{
	ProgramPath res = {};

	GetModuleFileNameA(NULL, res.path, sizeof(res.path));
	res.program_name = res.path;
	for(char *scan = res.path; *scan; ++scan)
	{
		if(*scan == '\\')
		{
			res.program_name = scan + 1;
		}
	}

	res.path_len = res.program_name - res.path;

	return(res);
}

KH_GLOBAL win32_allocator g_alloc_sentinel = {};
KH_GLOBAL TicketMutexLockk g_alloc_mutex = {};
static void
win32_init_allocator_sentinel()
{
	g_alloc_sentinel.next = &g_alloc_sentinel;
	g_alloc_sentinel.prev = &g_alloc_sentinel;
}

static SharedAllocator *
win32_virtual_alloc(umm size, u64 flags)
{
	// TODO(flo): two choices : we need to push the win32_allocator at the end of the ptr given by the virtual allocation or if we push it at the beginning it must be aligned with our cache line alignment of our allocation (2nd option here)
	kh_assert(sizeof(win32_allocator) == 64);

	umm total_size = size + sizeof(win32_allocator);
	umm base_offset = sizeof(win32_allocator);

	win32_allocator *alloc = (win32_allocator *)VirtualAlloc(0, total_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

	alloc->shared.base = (u8 *)alloc + base_offset;
	alloc->shared.size = size;
	//NOTE(flo): virtual alloc must be initialised to zero;
	kh_assert(alloc->shared.prev == 0);
	kh_assert(alloc->shared.used == 0);

	win32_allocator *sentinel = &g_alloc_sentinel;
	alloc->next = sentinel;

	begin_mutex_lock(&g_alloc_mutex);
	alloc->prev = sentinel->prev;
	alloc->prev->next = alloc;
	alloc->next->prev = alloc;
	end_mutext_lock(&g_alloc_mutex);

	SharedAllocator *res = &alloc->shared;
	return(res);
}

static void
win32_virtual_free(SharedAllocator *alloc)
{
	win32_allocator *win32_alloc = ((win32_allocator *)alloc);
	begin_mutex_lock(&g_alloc_mutex);
	win32_alloc->prev->next = win32_alloc->next;
	win32_alloc->next->prev = win32_alloc->prev;
	end_mutext_lock(&g_alloc_mutex);
	BOOL res = VirtualFree(alloc, 0, MEM_RELEASE);
}

inline win32_wnd_dim
win32_get_wnd_dim(HWND window)
{
	win32_wnd_dim res;

	RECT client_rect;
	GetClientRect(window, &client_rect);
	res.w = client_rect.right - client_rect.left;
	res.h = client_rect.bottom - client_rect.top;
	return(res);
}

KH_INTERN win32_sched_handle
win32_create_thread_handle(u32 thread_count)
{
	win32_sched_handle res;
	u32 count = 0;
	res.semaphore = CreateSemaphoreEx(0, count, thread_count, 0, 0, SEMAPHORE_ALL_ACCESS);
	return(res);
}

KH_INTERN void
win32_add_work_to_queue(WorkQueue *queue, job_entry_point *callback, void *user_data)
{
	u32 new_next_task_to_write = (queue->next_task_to_write + 1) & SCHED_TASK_MASK;
	kh_assert(new_next_task_to_write != queue->next_task_to_read);
	WorkQueueTask *task = queue->tasks + queue->next_task_to_write;
	task->callback = callback;
	task->user_data = user_data;
	++queue->end;
	queue->next_task_to_write = new_next_task_to_write;

	win32_sched_handle *hdl = (win32_sched_handle *)queue->platform_handle;
	ReleaseSemaphore(hdl->semaphore, 1, 0); 
}

KH_INTERN b32
win32_do_next_queue_work(WorkQueue *queue)
{
	b32 no_task_remaining = false;

	u32 orig_next_task_to_read = queue->next_task_to_read;
	u32 new_next_task_to_read = (orig_next_task_to_read + 1) & SCHED_TASK_MASK;
	if(orig_next_task_to_read != queue->next_task_to_write)
	{
		u32 ind = interlocked_compare_exchange_u32(&queue->next_task_to_read, new_next_task_to_read, 
			orig_next_task_to_read);
		if(ind == orig_next_task_to_read)
		{
			WorkQueueTask task = queue->tasks[ind];
			task.callback(queue, task.user_data);
			interlocked_increment_u32(&queue->current);
		}
	}
	else
	{
		no_task_remaining = true;
	}

	return(no_task_remaining);
}

KH_INTERN void
win32_complete_all_queue_works(WorkQueue *queue)
{
	while(queue->current != queue->end)
	{
		win32_do_next_queue_work(queue);
	}

	queue->current = 0;
	queue->end = 0;
}

DWORD WINAPI
win32_thread_proc(LPVOID lp_param)
{
	WorkQueue *queue = (WorkQueue *)lp_param;

	kh_assert(get_thread_id() == GetCurrentThreadId());
	for(;;)
	{
		if(win32_do_next_queue_work(queue))
		{
			win32_sched_handle *hdl = (win32_sched_handle *)queue->platform_handle;
			WaitForSingleObjectEx(hdl->semaphore, INFINITE, false);
		}
	}
}

KH_INTERN void
win32_create_sched_queue(WorkQueue *queue, win32_sched_handle *handle, u32 thread_count)
{
	queue->current = 0;
	queue->end = 0;

	queue->next_task_to_write = 0;
	queue->next_task_to_read = 0;

	queue->platform_handle = handle;

	for(u32 t_i = 0; t_i < thread_count; ++t_i)
	{
		DWORD thread_id;
		HANDLE thread_h = CreateThread(0, 0, win32_thread_proc, queue, 0, &thread_id);
		CloseHandle(thread_h);
	}
}

KH_INTERN void
win32_file_error(FileHandle *file_hdl, char *message) //, void *params)
{
	// win32_file_error *error = (win32_file_error *)params;
	OutputDebugStringA("win32 file error : ");
	OutputDebugStringA(message);
	OutputDebugStringA("\n");
	file_hdl->error = true;
}

KH_INTERN FileGroupOfType
win32_get_all_files_of_type(FileType f_type, StackAllocator *memstack)
{
	FileGroupOfType res = {};

	win32_file_group_of_type *win32_file_group = kh_push_struct(memstack, win32_file_group_of_type);
	res.platform_file_group = win32_file_group;

	wchar_t *str_type = L"*.*";
	switch(f_type)
	{
		case FileType_assets : 
		{
			str_type = L"*.kh";
		} break;
		case FileType_fonts :
		{
			str_type = L"*.kf";
		} break;
		case FileType_output_materials :
		{
			str_type = L"*.km";
		} break;
		case FileType_output_nodegraph :
		{
			str_type = L"*.kpg";
		} break;
		default :
		{
			kh_assert("invalid file type");
		} break;
	}

	// NOTE(flo): just for counting files
	WIN32_FIND_DATAW find_data;
	HANDLE file_hdl = FindFirstFileW(str_type, &find_data);
	while(file_hdl != INVALID_HANDLE_VALUE)
	{
		++res.file_count;

		if(!FindNextFileW(str_type, &find_data))
		{
			break;
		}
	}
	FindClose(file_hdl);

	win32_file_group->hdl = FindFirstFileW(str_type, &win32_file_group->data);
	return(res);
}

KH_INTERN void
win32_close_file_group(FileGroupOfType *file_group)
{
	win32_file_group_of_type *win32_file_group = (win32_file_group_of_type *)file_group->platform_file_group;
	if(win32_file_group)
	{
		FindClose(win32_file_group->hdl);
		// TODO(flo): find a way to free this
		// win32_virtual_free(win32_file_group);
	}
}

KH_INTERN FileHandle
win32_open_next_file_of_type(FileGroupOfType *file_group, StackAllocator *memstack)
{
	win32_file_group_of_type *win32_file_group = (win32_file_group_of_type *)file_group->platform_file_group;
	FileHandle res = {};

	if(win32_file_group->hdl != INVALID_HANDLE_VALUE)
	{
		win32_file_handle *win32_file_hdl = kh_push_struct(memstack, win32_file_handle);
		res.platform_file_handle = win32_file_hdl;

		if(win32_file_hdl)
		{
			wchar_t *filename = win32_file_group->data.cFileName;
			win32_file_hdl->hdl = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
			if(win32_file_hdl->hdl == INVALID_HANDLE_VALUE)
			{
				win32_file_error(&res, "unable to find the file");
			}
		}

		if(!FindNextFileW(win32_file_group->hdl, &win32_file_group->data))
		{
			FindClose(win32_file_group->hdl);
			win32_file_group->hdl = INVALID_HANDLE_VALUE;
		}
	}

	return(res);
}

KH_INTERN FileHandle
win32_open_file(char *file_name, StackAllocator *memstack, u32 file_access, FileCreation creation)
{
	FileHandle res = {};

	win32_file_handle *win32_file_hdl = kh_push_struct(memstack, win32_file_handle);
	res.platform_file_handle = win32_file_hdl;

	if(win32_file_hdl)
	{

		DWORD flags = (file_access & FileAccess_read) ? GENERIC_READ : 0;
		b32 test = (file_access & FileAccess_write);
		flags = (file_access & FileAccess_write) ? flags|GENERIC_WRITE : flags;

		DWORD creation_disp = OPEN_EXISTING;
		switch(creation)
		{
			case FileCreation_only_open : { creation_disp = OPEN_EXISTING; } break;
			case FileCreation_only_create : { creation_disp = CREATE_NEW; } break;
			case FileCreation_create_or_open : { creation_disp = OPEN_ALWAYS; } break;
			case FileCreation_override : { creation_disp = CREATE_ALWAYS; } break;
		}

		win32_file_hdl->hdl = CreateFile(file_name, flags, FILE_SHARE_READ, 0, creation_disp, 0, 0);
		if(win32_file_hdl->hdl == INVALID_HANDLE_VALUE)
		{
			win32_file_error(&res, "unable to find the file");
		}
	}

	return(res);
}

KH_INTERN void
win32_close_file(FileHandle *hdl)
{

	win32_file_handle *win32_file_hdl = (win32_file_handle *)hdl->platform_file_handle;
	CloseHandle(win32_file_hdl->hdl);
	// TODO(flo): find a way to free this
	// win32_virtual_free(hdl->platform_file_handle);
}

inline u32
win32_get_file_size(FileHandle *hdl)
{
	win32_file_handle *win32_file_hdl = (win32_file_handle *)hdl->platform_file_handle;
	DWORD s = GetFileSize(win32_file_hdl->hdl, NULL);

	u32 res = (u32)s;
	return(res);
}


KH_INTERN void
win32_read_bytes_of_file(FileHandle *file_hdl, u64 offset, u64 size, void *dst)
{
	if(!file_error(file_hdl))
	{
		OVERLAPPED overlapped = {};
		overlapped.Offset = (u32)(offset & 0xFFFFFFFF);
		overlapped.OffsetHigh = (u32)((offset >> 32) & 0xFFFFFFFF);
		HANDLE win32_hdl = ((win32_file_handle *)file_hdl->platform_file_handle)->hdl;
		DWORD bytes_read;
		u32 size32 = kh_safe_truncate_u64_to_u32(size);
		if(ReadFile(win32_hdl, dst, size32, &bytes_read, &overlapped) && (size32 == bytes_read))
		{

		}
		else
		{
			win32_file_error(file_hdl, "unable to read file");
		}
	}
}

KH_INTERN void
win32_write_bytes_to_file(FileHandle *file_hdl, u64 offset, u64 size, void *buffer)
{
	if(!file_error(file_hdl))
	{
		OVERLAPPED overlapped = {};
		overlapped.Offset = (u32)(offset & 0xFFFFFFFF);
		overlapped.OffsetHigh = (u32)((offset >> 32) & 0xFFFFFFFF);
		HANDLE win32_hdl = ((win32_file_handle *)file_hdl->platform_file_handle)->hdl;
		DWORD bytes_written;
		u32 size32 = kh_safe_truncate_u64_to_u32(size);
		if(WriteFile(win32_hdl, buffer, size32, &bytes_written, &overlapped) && (size32 == bytes_written))
		{
	
		}
		else
		{
			win32_file_error(file_hdl, "unable to write file");
		}
	}
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