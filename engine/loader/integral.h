/*	$Csoft$	*/
/*	Public domain	*/

#include "begin_code.h"

__BEGIN_DECLS
__inline__ Uint8 read_uint8(struct netbuf *);
__inline__ void	 write_uint8(struct netbuf *, Uint8);
__inline__ void	 pwrite_uint8(struct netbuf *, Uint8, off_t);
#define		 read_sint8(buf) ((Sint8)read_uint8(buf))
#define		 write_sint8(buf, v) write_uint8(buf, (Uint8)(v))
#define		 pwrite_sint8(buf, v, o) pwrite_uint8(buf, (Uint8)(v), (o))

__inline__ Uint16 read_uint16(struct netbuf *);
__inline__ void	  write_uint16(struct netbuf *, Uint16);
__inline__ void	  pwrite_uint16(struct netbuf *, Uint16, off_t);
#define		  read_sint16(buf) ((Sint16)read_uint16(buf))
#define		  write_sint16(buf, v) write_uint16(buf, (Uint16)(v))
#define		  pwrite_sint16(buf, v, o) pwrite_uint16(buf, (Uint16)(v), (o))

__inline__ Uint32 read_uint32(struct netbuf *);
__inline__ void	  write_uint32(struct netbuf *, Uint32);
__inline__ void	  pwrite_uint32(struct netbuf *, Uint32, off_t);
#define		  read_sint32(buf) ((Sint32)read_uint32(buf))
#define		  write_sint32(buf, v) write_uint32(buf, (Uint32)(v))
#define		  pwrite_sint32(buf, v, o) pwrite_uint32(buf, (Uint32)(v), (o))
__END_DECLS

#include "close_code.h"
