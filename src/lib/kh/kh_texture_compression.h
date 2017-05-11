#ifndef KH_TEXTURE_COMPRESSION_H

#define DXT_NORMAL_MODE 0
#define DXT_DITHER_MODE 1
#define DXT_HIGHQUAL_MODE 2

// TODO(flo): implement this!
KH_INTERN void
compress_to_dxt(void *dst, void *src, b32 alpha, u32 mode)
{

}

KH_INTERN void
compress_to_etc(void* dst, void* src, b32 alpha, u32 mode)
{

}

KH_INTERN void
compress_to_astc(void *dst, void *src, b32 alpha, u32 mode)
{

}

KH_INTERN void
compress_to_pvrtc(void *dst, void *src, b32 alpha, u32 mode)
{

}

#define KH_TEXTURE_COMPRESSION_H
#endif
