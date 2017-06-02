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

#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define ERROR_INVALID_VERSION_ARB               0x2095
#define ERROR_INVALID_PROFILE_ARB               0x2096


#define GL_TEXTURE_SWIZZLE_R              0x8E42
#define GL_TEXTURE_SWIZZLE_G              0x8E43
#define GL_TEXTURE_SWIZZLE_B              0x8E44
#define GL_TEXTURE_SWIZZLE_A              0x8E45
#define GL_TEXTURE_SWIZZLE_RGBA           0x8E46

#define GL_SHADING_LANGUAGE_VERSION       0x8B8C

#define GL_FRAGMENT_SHADER 				  0x8B30

#define GL_DRAW_INDIRECT_BUFFER           0x8F3F

#define GL_VERTEX_SHADER 				  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_STREAM_DRAW                    0x88E0
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_PIXEL_UNPACK_BUFFER            0x88EC
#define GL_UNIFORM_BUFFER                 0x8A11
#define GL_TEXTURE0                       0x84C0
#define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT 0x8A34
#define GL_MAP_READ_BIT                   0x0001
#define GL_MAP_WRITE_BIT                  0x0002
#define GL_MAP_PERSISTENT_BIT             0x0040
#define GL_MAP_COHERENT_BIT               0x0080
#define GL_DYNAMIC_STORAGE_BIT            0x0100
#define GL_MAX_COLOR_ATTACHMENTS          0x8CDF
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_COLOR_ATTACHMENT1              0x8CE1
#define GL_COLOR_ATTACHMENT2              0x8CE2
#define GL_COLOR_ATTACHMENT3              0x8CE3
#define GL_FRAMEBUFFER                    0x8D40
#define GL_READ_FRAMEBUFFER               0x8CA8
#define GL_DRAW_FRAMEBUFFER               0x8CA9
#define GL_READ_FRAMEBUFFER_BINDING       0x8CAA
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
#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY   0x9102
#define GL_TEXTURE_WRAP_R                 0x8072
#define GL_CLAMP_TO_EDGE                  0x812F

#define GL_DYNAMIC_READ                   0x88E9
#define GL_DYNAMIC_COPY                   0x88EA

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

// typedef void WINAPI gl_bind_framebuffer(GLenum target, GLuint framebuffer);
typedef char GLchar;
typedef int GLint;
typedef signed long long int GLsizeiptr;
typedef uint64_t GLuint64;
typedef int64_t GLint64;
typedef ptrdiff_t GLintptr;

typedef GLuint WINAPI TYPE_glCreateShader(GLenum shaderType);
typedef void WINAPI TYPE_glShaderSource(GLuint shader, GLsizei count, GLchar **string, GLint *length);
typedef void WINAPI TYPE_glCompileShader(GLuint shader);
typedef void WINAPI TYPE_glGetShaderiv(GLuint shader, GLenum pname, GLint *params); 
typedef void WINAPI TYPE_glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog); 
typedef void WINAPI TYPE_glAttachShader(GLuint program, GLuint shader); 
typedef void WINAPI TYPE_glDeleteShader(GLuint shader);
typedef void WINAPI TYPE_glBindBuffer(GLenum target, GLuint buffer); 
typedef void WINAPI TYPE_glBindFramebuffer(GLenum target, GLuint framebuffer); 
typedef void WINAPI TYPE_glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void WINAPI TYPE_glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint layer);
typedef GLuint WINAPI TYPE_glCreateProgram(void);
typedef void WINAPI TYPE_glLinkProgram(GLuint program);
typedef void WINAPI TYPE_glGetProgramiv(GLuint program, GLenum pname, GLint *params);
typedef void WINAPI TYPE_glGetProgramInfoLog(GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef void WINAPI TYPE_glGenVertexArrays(GLsizei n, GLuint *arrays);
typedef void WINAPI TYPE_glGenBuffers(GLsizei n, GLuint *buffers);
typedef void WINAPI TYPE_glGenFramebuffers(GLsizei n, GLuint *ids); 
typedef void WINAPI TYPE_glBindVertexArray(GLuint array); 
typedef void WINAPI TYPE_glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
typedef void WINAPI TYPE_glNamedBufferData(GLuint buffer, GLsizei size, const void *data, GLenum usage);
typedef void WINAPI TYPE_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
typedef void WINAPI TYPE_glEnableVertexAttribArray(GLuint index); 
typedef void WINAPI TYPE_glUseProgram(GLuint program);
typedef void WINAPI TYPE_glDeleteVertexArrays(GLsizei n, const GLuint *arrays);
typedef void WINAPI TYPE_glDeleteBuffers(GLsizei n, const GLuint *buffers);
typedef void WINAPI TYPE_glDeleteProgram(GLuint program);
typedef void WINAPI TYPE_glDetachShader(GLuint program, GLuint shader);
typedef void WINAPI TYPE_glDrawArraysIndirect(GLenum mode, const void *indirect);
typedef GLint WINAPI TYPE_glGetUniformLocation(GLuint program, const GLchar *name);
typedef void WINAPI TYPE_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void WINAPI TYPE_glCreateTextures(GLenum target, GLsizei n, GLuint *textures);
typedef void WINAPI TYPE_glTexStorage2D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef void WINAPI TYPE_glBufferStorage(GLenum target, GLsizeiptr size, const GLvoid* data, GLbitfield flags);
typedef void WINAPI TYPE_glNamedBufferStorage(GLuint buffer, GLsizei size, const void *data, GLbitfield flags);
typedef void WINAPI TYPE_glCreateBuffers(GLsizei n, GLuint *buffers);
typedef void WINAPI TYPE_glTextureParameteri(GLuint texture, GLenum pname, GLint param);
typedef void WINAPI TYPE_glTextureStorage2D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef void WINAPI TYPE_glTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
typedef void WINAPI TYPE_glBindTextureUnit(GLuint unit, GLuint texture);
typedef void WINAPI TYPE_glActiveTexture(GLenum texture);
typedef void WINAPI TYPE_glBindBufferBase(GLenum target, GLuint index, GLuint buffer);
typedef void WINAPI TYPE_glCreateFramebuffers(GLsizei n, GLuint *ids);
typedef void WINAPI TYPE_glNamedFramebufferTexture(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
typedef void WINAPI TYPE_glNamedFramebufferTextureLayer(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
typedef void WINAPI TYPE_glDrawElementsInstancedBaseVertexBaseInstance(GLenum mode, GLsizei count, GLenum type, GLvoid *indices, GLsizei primcount, GLint basevertex, GLuint baseinstance);
typedef void WINAPI TYPE_glDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect);
typedef void* WINAPI TYPE_glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef void* WINAPI TYPE_glMapNamedBufferRange(GLuint buffer, GLintptr offset, GLsizei length, GLbitfield access);
typedef GLboolean WINAPI TYPE_glUnmapBuffer(GLenum target);
typedef GLboolean WINAPI TYPE_glUnmapNamedBuffer(GLuint buffer);
typedef void WINAPI TYPE_glGenerateTextureMipmap(GLuint texture);
typedef void WINAPI TYPE_glDrawBuffers(GLsizei n, const GLenum *bufs);
typedef void WINAPI TYPE_glNamedFramebufferDrawBuffers(GLuint framebuffer, GLsizei n, const GLenum *bufs);
typedef GLenum WINAPI TYPE_glCheckFramebufferStatus(GLenum target);
typedef GLenum WINAPI TYPE_glCheckNamedFramebufferStatus(GLuint framebuffer, GLenum target);
typedef void WINAPI TYPE_glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data);
typedef void WINAPI TYPE_glNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizei size, const void *data);
typedef void WINAPI TYPE_glMultiDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride);
typedef void WINAPI TYPE_glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void WINAPI TYPE_glVertexAttribDivisor(GLuint index, GLuint divisor);
typedef GLubyte* WINAPI TYPE_glGetStringi(GLenum name, GLuint index);
typedef void WINAPI TYPE_glGetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params);
typedef void WINAPI TYPE_glGetInternalformati64v(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint64 *params);
typedef void WINAPI TYPE_glTextureStorage3D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
typedef void WINAPI TYPE_glTexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
typedef GLint WINAPI TYPE_glGetAttribLocation(GLuint program, const GLchar *name);
typedef GLint WINAPI TYPE_glGetUniformBlockIndex(GLuint program, const GLchar *name);
typedef void WINAPI TYPE_glBindAttribLocation(GLuint program, GLuint index, const GLchar *name);
typedef void WINAPI TYPE_glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
typedef void WINAPI TYPE_glCreateVertexArrays(GLsizei n, GLuint *arrays);

typedef void WINAPI TYPE_glBindTextures(GLuint first, GLsizei count, const GLuint *textures);

// @NOTE(flo): bindless textures
typedef void WINAPI TYPE_glCreateRenderbuffers(GLsizei n, GLuint *renderbuffers);
typedef void WINAPI TYPE_glGenRenderbuffers(GLsizei n, GLuint *renderbuffers);
typedef void WINAPI TYPE_glBindRenderbuffer(GLenum target, GLuint renderbuffer);
typedef void WINAPI TYPE_glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void WINAPI TYPE_glNamedFramebufferRenderbuffer(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void WINAPI TYPE_glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void WINAPI TYPE_glNamedRenderbufferStorage(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);

// @NOTE(flo): sparse textures
typedef void WINAPI TYPE_glTexturePageCommitmentEXT(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean resident);
typedef void WINAPI TYPE_glTexPageCommitmentARB(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean resident);

// @NOTE(flo): ARB Buffer Storage
// @NOTE(flo): ARB Map Buffer Range
// @NOTE(flo): ARB Multi Draw Indirect
// @NOTE(flo): ARB Shader Storage Buffer Object
// @NOTE(flo): ARB Uniform Buffer Object
// @NOTE(flo): ARB Sparse Texture
// @NOTE(flo): ARB Sync
// @NOTE(flo): ARB Texture View
// @NOTE(flo): ARB Shader Draw Parameters?
// @NOTE(flo): ARB Direct State Access
// @NOTE(flo): NV Command List

// @NOTE(flo): ARB Bindless texture
typedef GLuint64 WINAPI TYPE_glGetTextureHandleARB(GLuint texture);
typedef GLuint64 WINAPI TYPE_glGetTextureSamplerHandleARB(GLuint texture, GLuint sampler);
typedef void WINAPI TYPE_glMakeTextureHandleResidentARB(GLuint64 handle);
typedef void WINAPI TYPE_glMakeTextureHandleNonResidentARB(GLuint64 handle);
typedef void WINAPI TYPE_glUniformHandleui64ARB(GLint location, GLuint64 value);
// TODO(flo): other bindless texture at -> https://www.opengl.org/registry/specs/ARB/bindless_Texture.txt

typedef void WINAPI TYPE_glBindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLintptr stride);
typedef void WINAPI TYPE_glVertexAttribBinding(	GLuint attribindex, GLuint bindingindex);
typedef void WINAPI TYPE_glVertexAttribFormat(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
typedef void WINAPI TYPE_glVertexAttribIFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
typedef void WINAPI TYPE_glVertexAttribLFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
typedef void WINAPI TYPE_glBindVertexBuffers(GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
typedef void WINAPI TYPE_glVertexBindingDivisor(GLuint bindingindex, GLuint divisor);

typedef void WINAPI TYPE_glVertexArrayAttribFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
typedef void WINAPI TYPE_glVertexArrayVertexBuffer(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
typedef void WINAPI TYPE_glVertexArrayVertexBuffers(GLuint vaobj, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
typedef void WINAPI TYPE_glVertexArrayAttribBinding(GLuint vaobj, GLuint attribindex, GLuint bindingindex);
typedef void WINAPI TYPE_glVertexArrayBindingDivisor(GLuint vaobj, GLuint bindingindex, GLuint divisor);

// NOTE(flo): WGL EXTENSIONS
typedef BOOL WINAPI TYPE_wglSwapIntervalEXT(int interval);	
typedef BOOL WINAPI TYPE_wglChoosePixelFormatARB(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList,
    UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef HGLRC WINAPI TYPE_wglCreateContextAttribsARB(HDC hDC, HGLRC hShareContext, const int *attribList);

#define GL_DEBUG_CALLBACK(name) void WINAPI name(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam)
typedef GL_DEBUG_CALLBACK(GLDEBUGPROC);
typedef void WINAPI TYPE_glDebugMessageCallback(GLDEBUGPROC *callback, const void *userParam);

typedef void WINAPI TYPE_glTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);

typedef void WINAPI TYPE_glTextureView(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers);

typedef void WINAPI TYPE_glDeleteFramebuffers(GLsizei n, GLuint *framebuffers);

typedef void WINAPI TYPE_glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);

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
DEFINE_OGL_FUNC(glBindBufferRange);

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
	LOAD_OGL_FUNC(glBindBufferRange);
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

