#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_MEDIUM          0x9147
#define GL_DEBUG_SEVERITY_LOW             0x9148
#define GL_DEBUG_TYPE_MARKER              0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP          0x8269
#define GL_DEBUG_TYPE_POP_GROUP           0x826A
#define GL_DEBUG_SEVERITY_NOTIFICATION    0x826B
#define GL_DEBUG_OUTPUT_SYNCHRONOUS       0x8242
#define GL_DEBUG_OUTPUT                   0x92E0

#define GL_MAJOR_VERSION                  0x821B
#define GL_MINOR_VERSION                  0x821C

#define ERROR_INVALID_VERSION_ARB               0x2095
#define ERROR_INVALID_PROFILE_ARB               0x2096

#define GL_VERTEX_ARRAY_BINDING           0x85B5

#define GL_FIRST_VERTEX_CONVENTION        0x8E4D
#define GL_LAST_VERTEX_CONVENTION         0x8E4E
#define GL_PROVOKING_VERTEX               0x8E4F

#define GL_R8                             0x8229
#define GL_RGBA32UI                       0x8D70
#define GL_RGB32UI                        0x8D71
#define GL_RGBA16UI                       0x8D76
#define GL_RGB16UI                        0x8D77
#define GL_RGBA8UI                        0x8D7C
#define GL_RGB8UI                         0x8D7D
#define GL_RGBA32I                        0x8D82
#define GL_RGB32I                         0x8D83
#define GL_RGBA16I                        0x8D88
#define GL_RGB16I                         0x8D89
#define GL_RGBA8I                         0x8D8E
#define GL_RGB8I                          0x8D8F
#define GL_RED_INTEGER                    0x8D94
#define GL_GREEN_INTEGER                  0x8D95
#define GL_BLUE_INTEGER                   0x8D96
#define GL_RGB_INTEGER                    0x8D98
#define GL_RGBA_INTEGER                   0x8D99
#define GL_BGR_INTEGER                    0x8D9A
#define GL_BGRA_INTEGER                   0x8D9B
#define GL_RG                             0x8227
#define GL_RG_INTEGER                     0x8228
#define GL_R8                             0x8229
#define GL_R16                            0x822A
#define GL_RG8                            0x822B
#define GL_RG16                           0x822C
#define GL_R16F                           0x822D
#define GL_R32F                           0x822E
#define GL_RG16F                          0x822F
#define GL_RG32F                          0x8230
#define GL_R8I                            0x8231
#define GL_R8UI                           0x8232
#define GL_R16I                           0x8233
#define GL_R16UI                          0x8234
#define GL_R32I                           0x8235
#define GL_R32UI                          0x8236
#define GL_RG8I                           0x8237
#define GL_RG8UI                          0x8238
#define GL_RG16I                          0x8239
#define GL_RG16UI                         0x823A
#define GL_RG32I                          0x823B
#define GL_RG32UI                         0x823C
#define GL_R8_SNORM                       0x8F94
#define GL_RG8_SNORM                      0x8F95
#define GL_RGB8_SNORM                     0x8F96
#define GL_RGBA8_SNORM                    0x8F97
#define GL_R16_SNORM                      0x8F98
#define GL_RG16_SNORM                     0x8F99
#define GL_RGB16_SNORM                    0x8F9A
#define GL_RGBA16_SNORM                   0x8F9B
#define GL_RGBA32F                        0x8814
#define GL_RGB32F                         0x8815
#define GL_RGBA16F                        0x881A
#define GL_RGB16F                         0x881B
#define GL_TEXTURE_SWIZZLE_R              0x8E42
#define GL_TEXTURE_SWIZZLE_G              0x8E43
#define GL_TEXTURE_SWIZZLE_B              0x8E44
#define GL_TEXTURE_SWIZZLE_A              0x8E45
#define GL_TEXTURE_SWIZZLE_RGBA           0x8E46
#define GL_TEXTURE_CUBE_MAP_SEAMLESS      0x884F

#define GL_SHADING_LANGUAGE_VERSION       0x8B8C

#define GL_GEOMETRY_SHADER                0x8DD9
#define GL_FRAGMENT_SHADER 				  0x8B30

#define GL_DRAW_INDIRECT_BUFFER           0x8F3F

#define GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT 0x90DF
#define GL_SHADER_STORAGE_BUFFER_START    0x90D4
#define GL_SHADER_STORAGE_BUFFER_SIZE     0x90D5
#define GL_UNIFORM_BLOCK_DATA_SIZE        0x8A40
#define GL_UNIFORM_BLOCK_INDEX            0x8A3A
#define GL_SHADER_STORAGE_BLOCK           0x92E6
#define GL_BUFFER_DATA_SIZE               0x9303
#define GL_BUFFER_BINDING                 0x9302
#define GL_BUFFER_DATA_SIZE               0x9303
#define GL_NUM_ACTIVE_VARIABLES           0x9304
#define GL_NAME_LENGTH                    0x92F9
#define GL_ARRAY_SIZE                     0x92FB
#define GL_OFFSET                         0x92FC
#define GL_BLOCK_INDEX                    0x92FD
#define GL_BUFFER_VARIABLE                0x92E5

#define GL_REFERENCED_BY_VERTEX_SHADER    0x9306
#define GL_REFERENCED_BY_TESS_CONTROL_SHADER 0x9307
#define GL_REFERENCED_BY_TESS_EVALUATION_SHADER 0x9308
#define GL_REFERENCED_BY_GEOMETRY_SHADER  0x9309
#define GL_REFERENCED_BY_FRAGMENT_SHADER  0x930A
#define GL_REFERENCED_BY_COMPUTE_SHADER   0x930B
#define GL_MULTISAMPLE                    0x809D

#define GL_ACTIVE_VARIABLES               0x9305
#define GL_VERTEX_SHADER 				  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_STREAM_DRAW                    0x88E0
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_PIXEL_PACK_BUFFER              0x88EB
#define GL_PIXEL_PACK_BUFFER_BINDING      0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING    0x88EF
#define GL_PIXEL_UNPACK_BUFFER            0x88EC
#define GL_UNIFORM_BUFFER                 0x8A11
#define GL_TEXTURE0                       0x84C0
#define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT 0x8A34
#define GL_MAP_READ_BIT                   0x0001
#define GL_MAP_WRITE_BIT                  0x0002
#define GL_MAP_PERSISTENT_BIT             0x0040
#define GL_MAP_COHERENT_BIT               0x0080
#define GL_DYNAMIC_STORAGE_BIT            0x0100
#define GL_CLIENT_STORAGE_BIT             0x0200
#define GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT 0x00004000
#define GL_BUFFER_IMMUTABLE_STORAGE       0x821F
#define GL_BUFFER_STORAGE_FLAGS           0x8220
#define GL_CLEAR_TEXTURE                  0x9365
#define GL_LOCATION_COMPONENT             0x934A
#define GL_TRANSFORM_FEEDBACK_BUFFER_INDEX 0x934B
#define GL_TRANSFORM_FEEDBACK_BUFFER_STRIDE 0x934C
#define GL_QUERY_BUFFER                   0x9192
#define GL_QUERY_BUFFER_BARRIER_BIT       0x00008000
#define GL_QUERY_BUFFER_BINDING           0x9193
#define GL_QUERY_RESULT_NO_WAIT           0x9194

#define GL_MAX_COLOR_ATTACHMENTS          0x8CDF
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_COLOR_ATTACHMENT1              0x8CE1
#define GL_COLOR_ATTACHMENT2              0x8CE2
#define GL_COLOR_ATTACHMENT3              0x8CE3
#define GL_FRAMEBUFFER                    0x8D40
#define GL_READ_FRAMEBUFFER               0x8CA8
#define GL_DRAW_FRAMEBUFFER               0x8CA9
#define GL_READ_FRAMEBUFFER_BINDING       0x8CAA
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_FRAMEBUFFER_SRGB               0x8DB9
#define GL_FRAMEBUFFER_UNDEFINED          0x8219
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER 0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED        0x8CDD
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE 0x8D56
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS 0x8DA8
#define GL_RENDERBUFFER                   0x8D41
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_BGRA                           0x80E1
#define GL_SHADER_STORAGE_BUFFER          0x90D2
#define GL_NUM_EXTENSIONS				  0x821D
#define GL_TEXTURE_3D                     0x806F
#define GL_TEXTURE_2D_ARRAY               0x8C1A
#define GL_RGB                            0x1907
#define GL_BGR                            0x80E0
#define GL_BGRA                           0x80E1
#define GL_RGBA32F                        0x8814
#define GL_RGBA8                          0x8058
#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_COMPONENT32              0x81A7
#define GL_DEPTH_COMPONENT32F             0x8CAC
#define GL_TEXTURE_2D_ARRAY               0x8C1A
#define GL_MAX_ARRAY_TEXTURE_LAYERS       0x88FF
#define GL_TEXTURE_2D_MULTISAMPLE         0x9100
#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY   0x9102
#define GL_TEXTURE_WRAP_R                 0x8072
#define GL_CLAMP_TO_EDGE                  0x812F

#define GL_DYNAMIC_READ                   0x88E9
#define GL_DYNAMIC_COPY                   0x88EA
#define GL_READ_ONLY                      0x88B8
#define GL_WRITE_ONLY                     0x88B9
#define GL_READ_WRITE                     0x88BA
#define GL_BUFFER_ACCESS                  0x88BB
#define GL_BUFFER_MAPPED                  0x88BC
#define GL_BUFFER_MAP_POINTER             0x88BD
#define GL_STREAM_DRAW                    0x88E0
#define GL_STREAM_READ                    0x88E1
#define GL_STREAM_COPY                    0x88E2
#define GL_STATIC_DRAW                    0x88E4
#define GL_STATIC_READ                    0x88E5
#define GL_STATIC_COPY                    0x88E6
#define GL_DYNAMIC_DRAW                   0x88E8

#define GL_SAMPLES_PASSED                 0x8914
#define GL_TEXTURE_CUBE_MAP               0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP       0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X    0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X    0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y    0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y    0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z    0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z    0x851A
#define GL_TEXTURE_CUBE_MAP_ARRAY         0x9009
#define GL_TEXTURE_BINDING_CUBE_MAP_ARRAY 0x900A
#define GL_PROXY_TEXTURE_CUBE_MAP_ARRAY   0x900B
#define GL_SAMPLER_CUBE_MAP_ARRAY         0x900C
#define GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW  0x900D
#define GL_INT_SAMPLER_CUBE_MAP_ARRAY     0x900E
#define GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY 0x900F

#define GL_TEXTURE_SPARSE_ARB             0x91A6
#define GL_VIRTUAL_PAGE_SIZE_INDEX_ARB    0x91A7
#define GL_NUM_SPARSE_LEVELS_ARB          0x91AA
#define GL_NUM_VIRTUAL_PAGE_SIZES_ARB     0x91A8
#define GL_VIRTUAL_PAGE_SIZE_X_ARB        0x9195
#define GL_VIRTUAL_PAGE_SIZE_Y_ARB        0x9196
#define GL_VIRTUAL_PAGE_SIZE_Z_ARB        0x9197
#define GL_MAX_SPARSE_TEXTURE_SIZE_ARB    0x9198
#define GL_MAX_SPARSE_3D_TEXTURE_SIZE_ARB 0x9199
#define GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS_ARB 0x919A
#define GL_SPARSE_TEXTURE_FULL_ARRAY_CUBE_MIPMAPS_ARB 0x91A9

#define GL_SYNC_CONDITION                 0x9113
#define GL_SYNC_STATUS                    0x9114
#define GL_SYNC_FLAGS                     0x9115
#define GL_SYNC_FENCE                     0x9116
#define GL_SYNC_GPU_COMMANDS_COMPLETE     0x9117
#define GL_UNSIGNALED                     0x9118
#define GL_SIGNALED                       0x9119
#define GL_ALREADY_SIGNALED               0x911A
#define GL_TIMEOUT_EXPIRED                0x911B
#define GL_CONDITION_SATISFIED            0x911C
#define GL_WAIT_FAILED                    0x911D
#define GL_TIMEOUT_IGNORED                0xFFFFFFFFFFFFFFFFull

#define DEFINE_OGL_FUNC(name) KH_GLOBAL TYPE_##name *name

#ifndef APIENTRY
#define APIENTRY
#endif

#if defined(WIN32)
#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126
#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define LOAD_OGL_FUNC(name) name = (TYPE_##name *)wglGetProcAddress(#name);

typedef BOOL APIENTRY TYPE_wglSwapIntervalEXT(int interval);	
typedef BOOL APIENTRY TYPE_wglChoosePixelFormatARB(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList,
    UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef HGLRC APIENTRY TYPE_wglCreateContextAttribsARB(HDC hDC, HGLRC hShareContext, const int *attribList);
DEFINE_OGL_FUNC(wglSwapIntervalEXT);
DEFINE_OGL_FUNC(wglChoosePixelFormatARB);
DEFINE_OGL_FUNC(wglCreateContextAttribsARB);
KH_INTERN void
win32_load_wgl_extensions()
{
	LOAD_OGL_FUNC(wglSwapIntervalEXT);
	LOAD_OGL_FUNC(wglChoosePixelFormatARB);
	LOAD_OGL_FUNC(wglCreateContextAttribsARB);
}
#else
#error "PLATFORM NOT_IMPLEMENTED"
#define LOAD_OGL_FUNC(...)
#endif

// typedef void APIENTRY gl_bind_framebuffer(GLenum target, GLuint framebuffer);
#include <stdint.h>
typedef char GLchar;
typedef int GLint;
typedef signed long long int GLsizeiptr;
typedef uint64_t GLuint64;
typedef int64_t GLint64;
typedef ptrdiff_t GLintptr;

typedef void APIENTRY TYPE_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void APIENTRY TYPE_glUniform4fv(GLint location, GLsizei count, const GLfloat *value);
typedef void APIENTRY TYPE_glUniform1i(GLint location, GLint v0);
typedef void APIENTRY TYPE_glUniform1f(GLint location, GLfloat v0);
typedef void APIENTRY TYPE_glUniform2fv(GLint location, GLsizei count, const GLfloat *value);
typedef void APIENTRY TYPE_glUniform3fv(GLint location, GLsizei count, const GLfloat *value);
typedef void APIENTRY TYPE_glUniform1iv(GLint location, GLsizei count, const GLint *value);
typedef void APIENTRY TYPE_glTexImage2DMultisample(GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void APIENTRY TYPE_glNamedRenderbufferStorageMultisample(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);

typedef GLuint APIENTRY TYPE_glCreateShader(GLenum shaderType);
typedef void APIENTRY TYPE_glShaderSource(GLuint shader, GLsizei count, GLchar **string, GLint *length);
typedef void APIENTRY TYPE_glCompileShader(GLuint shader);
typedef void APIENTRY TYPE_glGetShaderiv(GLuint shader, GLenum pname, GLint *params); 
typedef void APIENTRY TYPE_glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source); 
typedef void APIENTRY TYPE_glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog); 
typedef void APIENTRY TYPE_glAttachShader(GLuint program, GLuint shader); 
typedef void APIENTRY TYPE_glDeleteShader(GLuint shader);
typedef void APIENTRY TYPE_glBindBuffer(GLenum target, GLuint buffer); 
typedef void APIENTRY TYPE_glBindFramebuffer(GLenum target, GLuint framebuffer); 
typedef void APIENTRY TYPE_glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level);
typedef void APIENTRY TYPE_glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void APIENTRY TYPE_glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint layer);
typedef GLuint APIENTRY TYPE_glCreateProgram(void);
typedef void APIENTRY TYPE_glLinkProgram(GLuint program);
typedef void APIENTRY TYPE_glGetProgramiv(GLuint program, GLenum pname, GLint *params);
typedef void APIENTRY TYPE_glGetProgramInfoLog(GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef void APIENTRY TYPE_glGenVertexArrays(GLsizei n, GLuint *arrays);
typedef void APIENTRY TYPE_glGenBuffers(GLsizei n, GLuint *buffers);
typedef void APIENTRY TYPE_glGenFramebuffers(GLsizei n, GLuint *ids); 
typedef void APIENTRY TYPE_glBindVertexArray(GLuint array); 
typedef void APIENTRY TYPE_glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
typedef void APIENTRY TYPE_glNamedBufferData(GLuint buffer, GLsizei size, const void *data, GLenum usage);
typedef void APIENTRY TYPE_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
typedef void APIENTRY TYPE_glEnableVertexAttribArray(GLuint index); 
typedef void APIENTRY TYPE_glUseProgram(GLuint program);
typedef void APIENTRY TYPE_glDeleteVertexArrays(GLsizei n, const GLuint *arrays);
typedef void APIENTRY TYPE_glDeleteBuffers(GLsizei n, const GLuint *buffers);
typedef void APIENTRY TYPE_glDeleteProgram(GLuint program);
typedef void APIENTRY TYPE_glDetachShader(GLuint program, GLuint shader);
typedef void APIENTRY TYPE_glDrawArraysIndirect(GLenum mode, const void *indirect);
typedef GLint APIENTRY TYPE_glGetUniformLocation(GLuint program, const GLchar *name);
typedef void APIENTRY TYPE_glCreateTextures(GLenum target, GLsizei n, GLuint *textures);
typedef void APIENTRY TYPE_glTexStorage2D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef void APIENTRY TYPE_glBufferStorage(GLenum target, GLsizeiptr size, const GLvoid* data, GLbitfield flags);
typedef void APIENTRY TYPE_glNamedBufferStorage(GLuint buffer, GLsizei size, const void *data, GLbitfield flags);
typedef void APIENTRY TYPE_glCreateBuffers(GLsizei n, GLuint *buffers);
typedef void APIENTRY TYPE_glTextureParameteri(GLuint texture, GLenum pname, GLint param);
typedef void APIENTRY TYPE_glTextureStorage2D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef void APIENTRY TYPE_glTextureStorage2DMultisample(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void APIENTRY TYPE_glTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
typedef void APIENTRY TYPE_glBindTextureUnit(GLuint unit, GLuint texture);
typedef void APIENTRY TYPE_glActiveTexture(GLenum texture);
typedef void APIENTRY TYPE_glBindBufferBase(GLenum target, GLuint index, GLuint buffer);
typedef void APIENTRY TYPE_glGenFramebuffers(GLsizei n, GLuint *ids);
typedef void APIENTRY TYPE_glCreateFramebuffers(GLsizei n, GLuint *ids);
typedef void APIENTRY TYPE_glNamedFramebufferTexture(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
typedef void APIENTRY TYPE_glNamedFramebufferTextureLayer(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
typedef void APIENTRY TYPE_glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, GLvoid *indices, GLint basevertex);
typedef void APIENTRY TYPE_glDrawElementsInstancedBaseVertexBaseInstance(GLenum mode, GLsizei count, GLenum type, GLvoid *indices, GLsizei primcount, GLint basevertex, GLuint baseinstance);
typedef void APIENTRY TYPE_glDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect);
typedef void* APIENTRY TYPE_glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef void* APIENTRY TYPE_glMapNamedBufferRange(GLuint buffer, GLintptr offset, GLsizei length, GLbitfield access);
typedef GLboolean APIENTRY TYPE_glUnmapBuffer(GLenum target);
typedef GLboolean APIENTRY TYPE_glUnmapNamedBuffer(GLuint buffer);
typedef void APIENTRY TYPE_glGenerateTextureMipmap(GLuint texture);
typedef void APIENTRY TYPE_glDrawBuffers(GLsizei n, const GLenum *bufs);
typedef void APIENTRY TYPE_glNamedFramebufferDrawBuffers(GLuint framebuffer, GLsizei n, const GLenum *bufs);
typedef GLenum APIENTRY TYPE_glCheckFramebufferStatus(GLenum target);
typedef GLenum APIENTRY TYPE_glCheckNamedFramebufferStatus(GLuint framebuffer, GLenum target);
typedef void APIENTRY TYPE_glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data);
typedef void APIENTRY TYPE_glNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizei size, const void *data);
typedef void APIENTRY TYPE_glMultiDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride);
typedef void APIENTRY TYPE_glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void APIENTRY TYPE_glVertexAttribDivisor(GLuint index, GLuint divisor);
typedef GLubyte* APIENTRY TYPE_glGetStringi(GLenum name, GLuint index);
typedef void APIENTRY TYPE_glGetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params);
typedef void APIENTRY TYPE_glGetInternalformati64v(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint64 *params);
typedef void APIENTRY TYPE_glTextureStorage3D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
typedef void APIENTRY TYPE_glTexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
typedef GLint APIENTRY TYPE_glGetAttribLocation(GLuint program, const GLchar *name);
typedef GLint APIENTRY TYPE_glGetUniformBlockIndex(GLuint program, const GLchar *name);
typedef void APIENTRY TYPE_glBindAttribLocation(GLuint program, GLuint index, const GLchar *name);
typedef void APIENTRY TYPE_glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
typedef void APIENTRY TYPE_glCreateVertexArrays(GLsizei n, GLuint *arrays);

typedef void APIENTRY TYPE_glBindTextures(GLuint first, GLsizei count, const GLuint *textures);

// @NOTE(flo): bindless textures
typedef void APIENTRY TYPE_glCreateRenderbuffers(GLsizei n, GLuint *renderbuffers);
typedef void APIENTRY TYPE_glGenRenderbuffers(GLsizei n, GLuint *renderbuffers);
typedef void APIENTRY TYPE_glBindRenderbuffer(GLenum target, GLuint renderbuffer);
typedef void APIENTRY TYPE_glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void APIENTRY TYPE_glNamedFramebufferRenderbuffer(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void APIENTRY TYPE_glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void APIENTRY TYPE_glNamedRenderbufferStorage(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);

// @NOTE(flo): sparse textures
typedef void APIENTRY TYPE_glTexturePageCommitmentEXT(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean resident);
typedef void APIENTRY TYPE_glTexPageCommitmentARB(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean resident);

typedef void APIENTRY TYPE_glGetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
typedef GLuint APIENTRY TYPE_glGetProgramResourceIndex(GLuint program, GLenum programInterface, const char *name);
typedef void APIENTRY TYPE_glGetProgramResourceiv(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params);

typedef void APIENTRY TYPE_glBlitNamedFramebuffer(GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

typedef void APIENTRY TYPE_glProvokingVertex(GLenum provokeMode);
// @NOTE(flo): ARB Buffer Storage
// @NOTE(flo): ARB Map Buffer Range
// @NOTE(flo): ARB Multi Draw Indirect
// @NOTE(flo): ARB Shader Storage Buffer Object
// @NOTE(flo): ARB Uniform Buffer Object
// @NOTE(flo): ARB Sparse Texture
// @NOTE(flo): ARB Sync

typedef struct __GLsync *GLsync;
typedef GLenum APIENTRY TYPE_glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef void APIENTRY TYPE_glDeleteSync(GLsync sync); 
typedef GLsync APIENTRY TYPE_glFenceSync(GLenum condition, GLbitfield flags);
typedef void APIENTRY TYPE_glWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef void APIENTRY TYPE_glGetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);

// @NOTE(flo): ARB Texture View
// @NOTE(flo): ARB Shader Draw Parameters?
// @NOTE(flo): ARB Direct State Access
// @NOTE(flo): NV Command List

// @NOTE(flo): ARB Bindless texture
typedef GLuint64 APIENTRY TYPE_glGetTextureHandleARB(GLuint texture);
typedef GLuint64 APIENTRY TYPE_glGetTextureSamplerHandleARB(GLuint texture, GLuint sampler);
typedef void APIENTRY TYPE_glMakeTextureHandleResidentARB(GLuint64 handle);
typedef void APIENTRY TYPE_glMakeTextureHandleNonResidentARB(GLuint64 handle);
typedef void APIENTRY TYPE_glUniformHandleui64ARB(GLint location, GLuint64 value);
// TODO(flo): other bindless texture at -> https://www.opengl.org/registry/specs/ARB/bindless_Texture.txt

typedef void APIENTRY TYPE_glBindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLintptr stride);
typedef void APIENTRY TYPE_glVertexAttribBinding(	GLuint attribindex, GLuint bindingindex);
typedef void APIENTRY TYPE_glVertexAttribFormat(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
typedef void APIENTRY TYPE_glVertexAttribIFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
typedef void APIENTRY TYPE_glVertexAttribLFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
typedef void APIENTRY TYPE_glBindVertexBuffers(GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
typedef void APIENTRY TYPE_glVertexBindingDivisor(GLuint bindingindex, GLuint divisor);

typedef void APIENTRY TYPE_glVertexArrayAttribFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
typedef void APIENTRY TYPE_glVertexArrayVertexBuffer(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
typedef void APIENTRY TYPE_glVertexArrayVertexBuffers(GLuint vaobj, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
typedef void APIENTRY TYPE_glVertexArrayAttribBinding(GLuint vaobj, GLuint attribindex, GLuint bindingindex);
typedef void APIENTRY TYPE_glVertexArrayBindingDivisor(GLuint vaobj, GLuint bindingindex, GLuint divisor);

#define GL_DEBUG_CALLBACK(name) void APIENTRY name(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam)
typedef GL_DEBUG_CALLBACK(GLDEBUGPROC);
typedef void APIENTRY TYPE_glDebugMessageCallback(GLDEBUGPROC *callback, const void *userParam);

typedef void APIENTRY TYPE_glTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);

typedef void APIENTRY TYPE_glTextureView(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers);

typedef void APIENTRY TYPE_glDeleteFramebuffers(GLsizei n, GLuint *framebuffers);

typedef void APIENTRY TYPE_glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef void APIENTRY TYPE_glBindFragDataLocation(GLuint program, GLuint colorNumber, const char *name);

typedef void APIENTRY TYPE_glUniform1ui(GLint location, GLuint v0);


typedef void APIENTRY TYPE_glClearBufferiv(GLenum buffer,GLint drawbuffer, const GLint * value);
typedef void APIENTRY TYPE_glClearBufferuiv(GLenum buffer,GLint drawbuffer, const GLuint * value);
typedef void APIENTRY TYPE_glClearBufferfv(GLenum buffer,GLint drawbuffer, const GLfloat * value);
typedef void APIENTRY TYPE_glClearBufferfi(GLenum buffer,GLint drawbuffer, GLfloat depth, GLint stencil);
typedef void APIENTRY TYPE_glClearNamedFramebufferiv(GLuint framebuffer,GLenum buffer, GLint drawbuffer, const GLint *value);
typedef void APIENTRY TYPE_glClearNamedFramebufferuiv(GLuint framebuffer,GLenum buffer, GLint drawbuffer, const GLuint *value);
typedef void APIENTRY TYPE_glClearNamedFramebufferfv(GLuint framebuffer,GLenum buffer, GLint drawbuffer, const GLfloat *value);
typedef void APIENTRY TYPE_glClearNamedFramebufferfi(GLuint framebuffer,GLenum buffer, const GLfloat depth, GLint stencil);

DEFINE_OGL_FUNC(glCreateShader);
DEFINE_OGL_FUNC(glShaderSource);
DEFINE_OGL_FUNC(glCompileShader);
DEFINE_OGL_FUNC(glGetShaderiv);
DEFINE_OGL_FUNC(glGetShaderSource);
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
DEFINE_OGL_FUNC(glCreateTextures);
DEFINE_OGL_FUNC(glTexStorage2D);
DEFINE_OGL_FUNC(glBufferStorage);
DEFINE_OGL_FUNC(glNamedBufferStorage);
DEFINE_OGL_FUNC(glCreateBuffers);
DEFINE_OGL_FUNC(glTextureParameteri);
DEFINE_OGL_FUNC(glTextureStorage2D);
DEFINE_OGL_FUNC(glTextureStorage2DMultisample);
DEFINE_OGL_FUNC(glTextureSubImage2D);
DEFINE_OGL_FUNC(glBindTextureUnit);
DEFINE_OGL_FUNC(glActiveTexture);
DEFINE_OGL_FUNC(glBindBufferBase);
DEFINE_OGL_FUNC(glCreateFramebuffers);
DEFINE_OGL_FUNC(glGenFramebuffers);
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
DEFINE_OGL_FUNC(glFramebufferTexture);
DEFINE_OGL_FUNC(glFramebufferTexture2D);
DEFINE_OGL_FUNC(glFramebufferTexture3D);
DEFINE_OGL_FUNC(glNamedFramebufferTextureLayer);
DEFINE_OGL_FUNC(glTextureView);
DEFINE_OGL_FUNC(glDeleteFramebuffers);
DEFINE_OGL_FUNC(glBindBufferRange);
DEFINE_OGL_FUNC(glGetActiveUniformBlockiv);
DEFINE_OGL_FUNC(glGetProgramResourceIndex);
DEFINE_OGL_FUNC(glGetProgramResourceiv);
DEFINE_OGL_FUNC(glUniformMatrix4fv);
DEFINE_OGL_FUNC(glUniform4fv);
DEFINE_OGL_FUNC(glUniform1i);
DEFINE_OGL_FUNC(glUniform1f);
DEFINE_OGL_FUNC(glUniform2fv);
DEFINE_OGL_FUNC(glUniform3fv);
DEFINE_OGL_FUNC(glUniform1iv);
DEFINE_OGL_FUNC(glBlitNamedFramebuffer);
DEFINE_OGL_FUNC(glProvokingVertex);
DEFINE_OGL_FUNC(glNamedRenderbufferStorageMultisample);
DEFINE_OGL_FUNC(glTexImage2DMultisample);
DEFINE_OGL_FUNC(glBindFragDataLocation);
DEFINE_OGL_FUNC(glUniform1ui);
DEFINE_OGL_FUNC(glDrawElementsBaseVertex);
DEFINE_OGL_FUNC(glClearBufferiv);
DEFINE_OGL_FUNC(glClearBufferuiv);
DEFINE_OGL_FUNC(glClearBufferfv);
DEFINE_OGL_FUNC(glClearBufferfi);
DEFINE_OGL_FUNC(glClearNamedFramebufferiv);
DEFINE_OGL_FUNC(glClearNamedFramebufferuiv);
DEFINE_OGL_FUNC(glClearNamedFramebufferfv);
DEFINE_OGL_FUNC(glClearNamedFramebufferfi);
DEFINE_OGL_FUNC(glClientWaitSync);
DEFINE_OGL_FUNC(glDeleteSync);
DEFINE_OGL_FUNC(glFenceSync);
DEFINE_OGL_FUNC(glWaitSync);
DEFINE_OGL_FUNC(glGetSynciv);

KH_INTERN void
load_gl_extensions()
{
	LOAD_OGL_FUNC(glGetStringi);
	LOAD_OGL_FUNC(glCreateShader);
	LOAD_OGL_FUNC(glShaderSource);
	LOAD_OGL_FUNC(glCompileShader);
	LOAD_OGL_FUNC(glGetShaderiv);
	LOAD_OGL_FUNC(glGetShaderSource);
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
	LOAD_OGL_FUNC(glBindBufferRange);
	LOAD_OGL_FUNC(glCreateFramebuffers);
	LOAD_OGL_FUNC(glGenFramebuffers);
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
	LOAD_OGL_FUNC(glFramebufferTexture);
	LOAD_OGL_FUNC(glFramebufferTexture2D);
	LOAD_OGL_FUNC(glFramebufferTexture3D);
	LOAD_OGL_FUNC(glNamedFramebufferTextureLayer);
	LOAD_OGL_FUNC(glTextureView);
	LOAD_OGL_FUNC(glDeleteFramebuffers);
	LOAD_OGL_FUNC(glGetActiveUniformBlockiv);
	LOAD_OGL_FUNC(glGetProgramResourceIndex);
	LOAD_OGL_FUNC(glGetProgramResourceiv);
	LOAD_OGL_FUNC(glTextureStorage2DMultisample);

	LOAD_OGL_FUNC(glUniformMatrix4fv);
	LOAD_OGL_FUNC(glUniform4fv);
	LOAD_OGL_FUNC(glUniform1i);
	LOAD_OGL_FUNC(glUniform1f);
	LOAD_OGL_FUNC(glUniform2fv);
	LOAD_OGL_FUNC(glUniform3fv);
	LOAD_OGL_FUNC(glUniform1iv);
	LOAD_OGL_FUNC(glBlitNamedFramebuffer);
	LOAD_OGL_FUNC(glProvokingVertex);
	LOAD_OGL_FUNC(glNamedRenderbufferStorageMultisample);
	LOAD_OGL_FUNC(glTexImage2DMultisample);
	LOAD_OGL_FUNC(glBindFragDataLocation);
	LOAD_OGL_FUNC(glUniform1ui);
	LOAD_OGL_FUNC(glDrawElementsBaseVertex);

	LOAD_OGL_FUNC(glClearBufferiv);
	LOAD_OGL_FUNC(glClearBufferuiv);
	LOAD_OGL_FUNC(glClearBufferfv);
	LOAD_OGL_FUNC(glClearBufferfi);
	LOAD_OGL_FUNC(glClearNamedFramebufferiv);
	LOAD_OGL_FUNC(glClearNamedFramebufferuiv);
	LOAD_OGL_FUNC(glClearNamedFramebufferfv);
	LOAD_OGL_FUNC(glClearNamedFramebufferfi);
	LOAD_OGL_FUNC(glClientWaitSync);
	LOAD_OGL_FUNC(glDeleteSync);
	LOAD_OGL_FUNC(glFenceSync);
	LOAD_OGL_FUNC(glWaitSync);
	LOAD_OGL_FUNC(glGetSynciv);

}
#undef LOAD_OGL_FUNC
#undef DEFINE_OGL_FUNC

