#ifndef KHJR_WIN32_H

#define FOREACH_SAMPLE(SAMPLE) \
	SAMPLE(basic_mesh) \
	SAMPLE(basic_scene) \

#define GENERATE_DLL_NAMES(NAME) "kh_" #NAME "_sample.dll",
#define GENERATE_DLL_ENUM(ENUM) Sample_##ENUM,

enum PracticeSample
{
	Sample_none,
	FOREACH_SAMPLE(GENERATE_DLL_ENUM)
};

struct win32_sample_library
{
	HMODULE lib;
	b32 is_loaded;
	kh_update *frame_update;
};

// TODO(flo): remove this?
#if 0
enum main_menu_state
{
	MainMenu_start,
	MainMenu_show,
};

typedef void mainmenu_reset(prog_memory *mem);
typedef void mainmenu_show(StackAllocator *memstack, prog_memory *memory, WorkQueue *queue, kh_render_manager *rd_buff, Assets *assets, f32 dt, main_menu_state state);
struct win32_main_menu_library
{
	HMODULE lib;
	b32 is_loaded;
	mainmenu_show *show_main_menu;
	mainmenu_reset *reset_main_menu;
};
#endif
typedef DWORD WINAPI xinput_get_state(DWORD dwUserIndex, XINPUT_STATE *pState);
typedef DWORD WINAPI xinput_set_state(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration);

struct win32_xinput_library
{
	HMODULE lib;
	xinput_get_state *get_state;
	xinput_set_state *set_state;
};

enum RendererType {
	RendererType_software,
	RendererType_opengl,
};

enum DisplayType {
	DisplayType_opengl,
};

struct win32_state
{
	RendererType renderer;
	DisplayType display;

	PracticeSample new_sample;
	PracticeSample cur_sample;

	b32 show_menu;
	WINDOWPLACEMENT window_pos;
};

#define LOAD_OGL_FUNC(name) name = (TYPE_##name *)wglGetProcAddress(#name); kh_assert(name)
#define DEFINE_OGL_FUNC(name) KH_GLOBAL TYPE_##name *name

DEFINE_OGL_FUNC(wglSwapIntervalEXT);
DEFINE_OGL_FUNC(wglChoosePixelFormatARB);
DEFINE_OGL_FUNC(wglCreateContextAttribsARB);
DEFINE_OGL_FUNC(glCreateShader);
DEFINE_OGL_FUNC(glShaderSource);
DEFINE_OGL_FUNC(glCompileShader);
DEFINE_OGL_FUNC(glGetShaderiv);
DEFINE_OGL_FUNC(glGetShaderInfoLog);
DEFINE_OGL_FUNC(glAttachShader);
DEFINE_OGL_FUNC(glBindBuffer);
DEFINE_OGL_FUNC(glBindFramebuffer);
DEFINE_OGL_FUNC(glCreateProgram);
DEFINE_OGL_FUNC(glUseProgram);
DEFINE_OGL_FUNC(glDeleteProgram);
DEFINE_OGL_FUNC(glLinkProgram);
DEFINE_OGL_FUNC(glGetProgramiv);
DEFINE_OGL_FUNC(glGetProgramInfoLog);
DEFINE_OGL_FUNC(glDeleteShader);
DEFINE_OGL_FUNC(glGenVertexArrays);
DEFINE_OGL_FUNC(glGenBuffers);
DEFINE_OGL_FUNC(glBindVertexArray);
DEFINE_OGL_FUNC(glBufferData);
DEFINE_OGL_FUNC(glNamedBufferData);
DEFINE_OGL_FUNC(glVertexAttribPointer);
DEFINE_OGL_FUNC(glEnableVertexAttribArray);
DEFINE_OGL_FUNC(glDeleteVertexArrays);
DEFINE_OGL_FUNC(glDeleteBuffers);
DEFINE_OGL_FUNC(glDetachShader);
DEFINE_OGL_FUNC(glDrawArraysIndirect);
DEFINE_OGL_FUNC(glGetUniformLocation);
DEFINE_OGL_FUNC(glUniformMatrix4fv);
DEFINE_OGL_FUNC(glCreateTextures);
DEFINE_OGL_FUNC(glTexStorage2D);
DEFINE_OGL_FUNC(glBufferStorage);
DEFINE_OGL_FUNC(glNamedBufferStorage);
DEFINE_OGL_FUNC(glCreateBuffers);
DEFINE_OGL_FUNC(glTextureParameteri);
DEFINE_OGL_FUNC(glTextureStorage2D);
DEFINE_OGL_FUNC(glTextureSubImage2D);
DEFINE_OGL_FUNC(glBindTextureUnit);
DEFINE_OGL_FUNC(glActiveTexture);
DEFINE_OGL_FUNC(glBindBufferBase);
DEFINE_OGL_FUNC(glCreateFramebuffers);
DEFINE_OGL_FUNC(glNamedFramebufferTexture);
DEFINE_OGL_FUNC(glDrawElementsInstancedBaseVertexBaseInstance);
DEFINE_OGL_FUNC(glDrawElementsIndirect);
DEFINE_OGL_FUNC(glMapBufferRange);
DEFINE_OGL_FUNC(glMapNamedBufferRange);
DEFINE_OGL_FUNC(glUnmapBuffer);
DEFINE_OGL_FUNC(glUnmapNamedBuffer);
DEFINE_OGL_FUNC(glGenerateTextureMipmap);
DEFINE_OGL_FUNC(glDrawBuffers);
DEFINE_OGL_FUNC(glNamedFramebufferDrawBuffers);
DEFINE_OGL_FUNC(glCheckFramebufferStatus);
DEFINE_OGL_FUNC(glCheckNamedFramebufferStatus);
DEFINE_OGL_FUNC(glCreateRenderbuffers);
DEFINE_OGL_FUNC(glGenRenderbuffers);
DEFINE_OGL_FUNC(glBindRenderbuffer);
DEFINE_OGL_FUNC(glFramebufferRenderbuffer);
DEFINE_OGL_FUNC(glNamedFramebufferRenderbuffer);
DEFINE_OGL_FUNC(glRenderbufferStorage);
DEFINE_OGL_FUNC(glNamedRenderbufferStorage);
DEFINE_OGL_FUNC(glBufferSubData);
DEFINE_OGL_FUNC(glNamedBufferSubData);
DEFINE_OGL_FUNC(glMultiDrawElementsIndirect);
DEFINE_OGL_FUNC(glVertexAttribIPointer);
DEFINE_OGL_FUNC(glVertexAttribDivisor);
DEFINE_OGL_FUNC(glGetStringi);
DEFINE_OGL_FUNC(glGetInternalformativ);
DEFINE_OGL_FUNC(glGetInternalformati64v);
DEFINE_OGL_FUNC(glTextureStorage3D);
DEFINE_OGL_FUNC(glTexStorage3D);
DEFINE_OGL_FUNC(glGetAttribLocation);
DEFINE_OGL_FUNC(glGetUniformBlockIndex);
DEFINE_OGL_FUNC(glBindAttribLocation);
DEFINE_OGL_FUNC(glUniformBlockBinding);
DEFINE_OGL_FUNC(glBindTextures);
DEFINE_OGL_FUNC(glDebugMessageCallback);
DEFINE_OGL_FUNC(glTexPageCommitmentARB);
DEFINE_OGL_FUNC(glTexturePageCommitmentEXT);
DEFINE_OGL_FUNC(glGetTextureHandleARB);
DEFINE_OGL_FUNC(glGetTextureSamplerHandleARB);
DEFINE_OGL_FUNC(glMakeTextureHandleResidentARB);
DEFINE_OGL_FUNC(glMakeTextureHandleNonResidentARB);
DEFINE_OGL_FUNC(glUniformHandleui64ARB);
DEFINE_OGL_FUNC(glBindVertexBuffer);
DEFINE_OGL_FUNC(glVertexAttribBinding);
DEFINE_OGL_FUNC(glVertexAttribFormat);
DEFINE_OGL_FUNC(glVertexAttribIFormat);
DEFINE_OGL_FUNC(glVertexAttribLFormat);
DEFINE_OGL_FUNC(glBindVertexBuffers);
DEFINE_OGL_FUNC(glVertexArrayAttribFormat);
DEFINE_OGL_FUNC(glVertexArrayVertexBuffer);
DEFINE_OGL_FUNC(glVertexArrayVertexBuffers);
DEFINE_OGL_FUNC(glVertexArrayAttribBinding);
DEFINE_OGL_FUNC(glVertexBindingDivisor);
DEFINE_OGL_FUNC(glVertexArrayBindingDivisor);
DEFINE_OGL_FUNC(glCreateVertexArrays);
DEFINE_OGL_FUNC(glTextureSubImage3D);
DEFINE_OGL_FUNC(glFramebufferTexture3D);
DEFINE_OGL_FUNC(glNamedFramebufferTextureLayer);
DEFINE_OGL_FUNC(glTextureView);
DEFINE_OGL_FUNC(glDeleteFramebuffers);


KH_INTERN void
win32_load_wgl_extensions()
{
	LOAD_OGL_FUNC(wglSwapIntervalEXT);
	LOAD_OGL_FUNC(wglChoosePixelFormatARB);
	LOAD_OGL_FUNC(wglCreateContextAttribsARB);
}

KH_INTERN void
win32_load_gl_extensions()
{
	LOAD_OGL_FUNC(glGetStringi);
	LOAD_OGL_FUNC(glCreateShader);
	LOAD_OGL_FUNC(glShaderSource);
	LOAD_OGL_FUNC(glCompileShader);
	LOAD_OGL_FUNC(glGetShaderiv);
	LOAD_OGL_FUNC(glGetShaderInfoLog);
	LOAD_OGL_FUNC(glBindBuffer);
	LOAD_OGL_FUNC(glBindFramebuffer);
	LOAD_OGL_FUNC(glCreateProgram);
	LOAD_OGL_FUNC(glAttachShader);
	LOAD_OGL_FUNC(glLinkProgram);
	LOAD_OGL_FUNC(glGetProgramiv);
	LOAD_OGL_FUNC(glGetProgramInfoLog);
	LOAD_OGL_FUNC(glDeleteShader);
	LOAD_OGL_FUNC(glGenVertexArrays);
	LOAD_OGL_FUNC(glGenBuffers);
	LOAD_OGL_FUNC(glBindVertexArray);
	LOAD_OGL_FUNC(glBufferData);
	LOAD_OGL_FUNC(glNamedBufferData);
	LOAD_OGL_FUNC(glVertexAttribPointer);
	LOAD_OGL_FUNC(glEnableVertexAttribArray);
	LOAD_OGL_FUNC(glUseProgram);
	LOAD_OGL_FUNC(glDeleteVertexArrays);
	LOAD_OGL_FUNC(glDeleteBuffers);
	LOAD_OGL_FUNC(glDeleteProgram);
	LOAD_OGL_FUNC(glDetachShader);
	LOAD_OGL_FUNC(glDrawArraysIndirect);
	LOAD_OGL_FUNC(glGetUniformLocation);
	LOAD_OGL_FUNC(glUniformMatrix4fv);
	LOAD_OGL_FUNC(glCreateTextures);
	LOAD_OGL_FUNC(glTexStorage2D);
	LOAD_OGL_FUNC(glBufferStorage);
	LOAD_OGL_FUNC(glNamedBufferStorage);
	LOAD_OGL_FUNC(glCreateBuffers);
	LOAD_OGL_FUNC(glTextureParameteri);
	LOAD_OGL_FUNC(glTextureStorage2D);
	LOAD_OGL_FUNC(glTextureSubImage2D);
	LOAD_OGL_FUNC(glBindTextureUnit);
	LOAD_OGL_FUNC(glActiveTexture);
	LOAD_OGL_FUNC(glBindBufferBase);
	LOAD_OGL_FUNC(glCreateFramebuffers);
	LOAD_OGL_FUNC(glNamedFramebufferTexture);
	LOAD_OGL_FUNC(glDrawElementsInstancedBaseVertexBaseInstance);
	LOAD_OGL_FUNC(glDrawElementsIndirect);
	LOAD_OGL_FUNC(glMapBufferRange);
	LOAD_OGL_FUNC(glMapNamedBufferRange);
	LOAD_OGL_FUNC(glUnmapBuffer);
	LOAD_OGL_FUNC(glUnmapNamedBuffer);
	LOAD_OGL_FUNC(glGenerateTextureMipmap);
	LOAD_OGL_FUNC(glDrawBuffers);
	LOAD_OGL_FUNC(glNamedFramebufferDrawBuffers);
	LOAD_OGL_FUNC(glCheckFramebufferStatus);
	LOAD_OGL_FUNC(glCheckNamedFramebufferStatus);
	LOAD_OGL_FUNC(glCreateRenderbuffers);
	LOAD_OGL_FUNC(glGenRenderbuffers);
	LOAD_OGL_FUNC(glBindRenderbuffer);
	LOAD_OGL_FUNC(glFramebufferRenderbuffer);
	LOAD_OGL_FUNC(glNamedFramebufferRenderbuffer);
	LOAD_OGL_FUNC(glRenderbufferStorage);
	LOAD_OGL_FUNC(glNamedRenderbufferStorage);
	LOAD_OGL_FUNC(glBufferSubData);
	LOAD_OGL_FUNC(glNamedBufferSubData);
	LOAD_OGL_FUNC(glMultiDrawElementsIndirect);
	LOAD_OGL_FUNC(glVertexAttribIPointer);
	LOAD_OGL_FUNC(glVertexAttribDivisor);
	LOAD_OGL_FUNC(glGetInternalformativ);
	LOAD_OGL_FUNC(glGetInternalformati64v);
	LOAD_OGL_FUNC(glTextureStorage3D);
	LOAD_OGL_FUNC(glTexStorage3D);
	LOAD_OGL_FUNC(glGetAttribLocation);
	LOAD_OGL_FUNC(glGetUniformLocation);
	LOAD_OGL_FUNC(glGetUniformBlockIndex);
	LOAD_OGL_FUNC(glBindAttribLocation);
	LOAD_OGL_FUNC(glUniformBlockBinding);
	LOAD_OGL_FUNC(glBindTextures);

	LOAD_OGL_FUNC(glDebugMessageCallback);

	// NOTE(flo): Bindless textures
	LOAD_OGL_FUNC(glGetTextureHandleARB);
	LOAD_OGL_FUNC(glGetTextureSamplerHandleARB);
	LOAD_OGL_FUNC(glMakeTextureHandleResidentARB);
	LOAD_OGL_FUNC(glMakeTextureHandleNonResidentARB);
	LOAD_OGL_FUNC(glUniformHandleui64ARB);

	// @NOTE(flo): Sparse Texture
	LOAD_OGL_FUNC(glTexPageCommitmentARB);
	LOAD_OGL_FUNC(glTexturePageCommitmentEXT);

	LOAD_OGL_FUNC(glBindVertexBuffer);
	LOAD_OGL_FUNC(glVertexAttribBinding);
	LOAD_OGL_FUNC(glVertexAttribFormat);
	LOAD_OGL_FUNC(glVertexAttribIFormat);
	LOAD_OGL_FUNC(glVertexAttribLFormat);
	LOAD_OGL_FUNC(glBindVertexBuffers);
	LOAD_OGL_FUNC(glVertexBindingDivisor);
	LOAD_OGL_FUNC(glVertexArrayAttribFormat);
	LOAD_OGL_FUNC(glVertexArrayVertexBuffer);
	LOAD_OGL_FUNC(glVertexArrayVertexBuffers);
	LOAD_OGL_FUNC(glVertexArrayAttribBinding);
	LOAD_OGL_FUNC(glVertexArrayBindingDivisor);
	LOAD_OGL_FUNC(glCreateVertexArrays);
	LOAD_OGL_FUNC(glTextureSubImage3D);
	LOAD_OGL_FUNC(glFramebufferTexture3D);
	LOAD_OGL_FUNC(glNamedFramebufferTextureLayer);
	LOAD_OGL_FUNC(glTextureView);
	LOAD_OGL_FUNC(glDeleteFramebuffers);


}
#undef LOAD_OGL_FUNC
#undef DEFINE_OGL_FUNC

#define KHJR_WIN32_H
#endif //KHJR_WIN32_H
