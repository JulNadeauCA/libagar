/*	$Csoft: integral.h,v 1.1 2003/06/19 01:53:38 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

__BEGIN_DECLS
__inline__ Uint8 AG_ReadUint8(AG_Netbuf *);
__inline__ void	 AG_WriteUint8(AG_Netbuf *, Uint8);
__inline__ void	 AG_PwriteUint8(AG_Netbuf *, Uint8, off_t);
#define		 AG_ReadSint8(buf) ((Sint8)AG_ReadUint8(buf))
#define		 AG_WriteSint8(buf, v) AG_WriteUint8(buf, (Uint8)(v))
#define		 AG_PwriteSint8(buf, v, o) AG_PwriteUint8(buf, (Uint8)(v), (o))

__inline__ Uint16 AG_ReadUint16(AG_Netbuf *);
__inline__ void	  AG_WriteUint16(AG_Netbuf *, Uint16);
__inline__ void	  AG_PwriteUint16(AG_Netbuf *, Uint16, off_t);
#define		  AG_ReadSint16(buf) ((Sint16)AG_ReadUint16(buf))
#define		  AG_WriteSint16(buf, v) AG_WriteUint16(buf, (Uint16)(v))
#define		  AG_PwriteSint16(buf, v, o) AG_PwriteUint16(buf, (Uint16)(v), (o))

__inline__ Uint32 AG_ReadUint32(AG_Netbuf *);
__inline__ void	  AG_WriteUint32(AG_Netbuf *, Uint32);
__inline__ void	  AG_PwriteUint32(AG_Netbuf *, Uint32, off_t);
#define		  AG_ReadSint32(buf) ((Sint32)AG_ReadUint32(buf))
#define		  AG_WriteSint32(buf, v) AG_WriteUint32(buf, (Uint32)(v))
#define		  AG_PwriteSint32(buf, v, o) AG_PwriteUint32(buf, (Uint32)(v), (o))
__END_DECLS

#include "close_code.h"
