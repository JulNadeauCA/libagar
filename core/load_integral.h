/*	Public domain	*/

#include "begin_code.h"
__BEGIN_DECLS
#define	AG_ReadSint8(buf) ((Sint8)AG_ReadUint8(buf))
#define	AG_WriteSint8(buf, v) AG_WriteUint8(buf, (Uint8)(v))
#define	AG_PwriteSint8(buf, v, o) AG_PwriteUint8(buf, (Uint8)(v), (o))
#define	AG_ReadSint16(buf) ((Sint16)AG_ReadUint16(buf))
#define	AG_WriteSint16(buf, v) AG_WriteUint16(buf, (Uint16)(v))
#define	AG_PwriteSint16(buf, v, o) AG_PwriteUint16(buf, (Uint16)(v), (o))
#define	AG_ReadSint32(buf) ((Sint32)AG_ReadUint32(buf))
#define	AG_WriteSint32(buf, v) AG_WriteUint32(buf, (Uint32)(v))
#define	AG_PwriteSint32(buf, v, o) AG_PwriteUint32(buf, (Uint32)(v), (o))
#define	AG_ReadSint64(buf) ((Sint64)AG_ReadUint64(buf))
#define	AG_WriteSint64(buf, v) AG_WriteUint64(buf, (Uint64)(v))
#define	AG_PwriteSint64(buf, v, o) AG_PwriteUint64(buf, (Uint64)(v), (o))

static __inline__ Uint8
AG_ReadUint8(AG_Netbuf *buf)
{
	Uint8 i;
	AG_NetbufRead(&i, sizeof(i), 1, buf);
	return (i);
}
static __inline__ void
AG_WriteUint8(AG_Netbuf *buf, Uint8 i)
{
	AG_NetbufWrite(&i, sizeof(i), 1, buf);
}
static __inline__ void
AG_PwriteUint8(AG_Netbuf *buf, Uint8 i, off_t offs)
{
	AG_NetbufPwrite(&i, sizeof(i), 1, offs, buf);
}
static __inline__ Uint16
AG_ReadUint16(AG_Netbuf *buf)
{
	Uint16 i;
	AG_NetbufRead(&i, sizeof(i), 1, buf);
	return ((buf->byte_order == AG_NETBUF_BIG_ENDIAN) ?
	    SDL_SwapBE16(i) : SDL_SwapLE16(i));
	
}
static __inline__ void
AG_WriteUint16(AG_Netbuf *buf, Uint16 u16)
{
	Uint16 i = (buf->byte_order == AG_NETBUF_BIG_ENDIAN) ?
	    SDL_SwapBE16(u16) : SDL_SwapLE16(u16);
	AG_NetbufWrite(&i, sizeof(i), 1, buf);
}
static __inline__ void
AG_PwriteUint16(AG_Netbuf *buf, Uint16 u16, off_t offs)
{
	Uint16 i = (buf->byte_order == AG_NETBUF_BIG_ENDIAN) ?
	    SDL_SwapBE16(u16) : SDL_SwapLE16(u16);
	AG_NetbufPwrite(&i, sizeof(i), 1, offs, buf);
}
static __inline__ Uint32
AG_ReadUint32(AG_Netbuf *buf)
{
	Uint32 i;
	AG_NetbufRead(&i, sizeof(i), 1, buf);
	return ((buf->byte_order == AG_NETBUF_BIG_ENDIAN) ?
	    SDL_SwapBE32(i) : SDL_SwapLE32(i));
}
static __inline__ void
AG_WriteUint32(AG_Netbuf *buf, Uint32 u32)
{
	Uint32 i = (buf->byte_order == AG_NETBUF_BIG_ENDIAN) ?
	    SDL_SwapBE32(u32) : SDL_SwapLE32(u32);
	AG_NetbufWrite(&i, sizeof(i), 1, buf);
}
static __inline__ void
AG_PwriteUint32(AG_Netbuf *buf, Uint32 u32, off_t offs)
{
	Uint32 i = (buf->byte_order == AG_NETBUF_BIG_ENDIAN) ?
	    SDL_SwapBE32(u32) : SDL_SwapLE32(u32);
	AG_NetbufPwrite(&i, sizeof(i), 1, offs, buf);
}

#ifdef SDL_HAS_64BIT_TYPE
static __inline__ Uint64
AG_ReadUint64(AG_Netbuf *buf)
{
	Uint64 i;
	AG_NetbufRead(&i, sizeof(i), 1, buf);
	return ((buf->byte_order == AG_NETBUF_BIG_ENDIAN) ?
	    SDL_SwapBE64(i) : SDL_SwapLE64(i));
}
static __inline__ void
AG_WriteUint64(AG_Netbuf *buf, Uint64 u64)
{
	Uint64 i = (buf->byte_order == AG_NETBUF_BIG_ENDIAN) ?
	    SDL_SwapBE64(u64) : SDL_SwapLE64(u64);
	AG_NetbufWrite(&i, sizeof(i), 1, buf);
}
static __inline__ void
AG_PwriteUint64(AG_Netbuf *buf, Uint64 u64, off_t offs)
{
	Uint64 i = (buf->byte_order == AG_NETBUF_BIG_ENDIAN) ?
	    SDL_SwapBE64(u64) : SDL_SwapLE64(u64);
	AG_NetbufPwrite(&i, sizeof(i), 1, offs, buf);
}
#endif /* SDL_HAS_64BIT_TYPE */

__END_DECLS
#include "close_code.h"
