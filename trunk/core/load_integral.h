/*	Public domain	*/

/*
 * Serialization of integers.
 */

#include <agar/core/byteswap.h>

#include "begin_code.h"
__BEGIN_DECLS
#define	AG_ReadSint8(ds)	 ((Sint8)AG_ReadUint8(ds))
#define	AG_WriteSint8(ds,v)	 AG_WriteUint8((ds),(Uint8)(v))
#define	AG_WriteSint8At(ds,v,o)	 AG_WriteUint8At((ds),(Uint8)(v),(o))
#define	AG_ReadSint16(ds)	 ((Sint16)AG_ReadUint16(ds))
#define	AG_WriteSint16(ds,v)	 AG_WriteUint16((ds),(Uint16)(v))
#define	AG_WriteSint16At(ds,v,o) AG_WriteUint16At((ds),(Uint16)(v),(o))
#define	AG_ReadSint32(ds)	 ((Sint32)AG_ReadUint32(ds))
#define	AG_WriteSint32(ds,v)	 AG_WriteUint32((ds),(Uint32)(v))
#define	AG_WriteSint32At(ds,v,o) AG_WriteUint32At((ds),(Uint32)(v),(o))
#ifdef HAVE_64BIT
# define AG_ReadSint64(ds)	  ((Sint64)AG_ReadUint64(ds))
# define AG_WriteSint64(ds,v)	  AG_WriteUint64((ds),(Uint64)(v))
# define AG_WriteSint64At(ds,v,o) AG_WriteUint64At((ds),(Uint64)(v),(o))
#endif

static __inline__ Uint8
AG_ReadUint8(AG_DataSource *ds)
{
	Uint8 i;
	if (AG_Read(ds, &i, sizeof(i), 1) != 0) { AG_FatalError(NULL); }
	return (i);
}
static __inline__ void
AG_WriteUint8(AG_DataSource *ds, Uint8 i)
{
	if (AG_Write(ds, &i, sizeof(i), 1) != 0) { AG_FatalError(NULL); }
}
static __inline__ void
AG_WriteUint8At(AG_DataSource *ds, Uint8 i, off_t pos)
{
	if (AG_WriteAt(ds, &i, sizeof(i), 1, pos) != 0) { AG_FatalError(NULL); }
}
static __inline__ Uint16
AG_ReadUint16(AG_DataSource *ds)
{
	Uint16 i;
	if (AG_Read(ds, &i, sizeof(i), 1) != 0) { AG_FatalError(NULL); }

	return (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE16(i) :
	                                             AG_SwapLE16(i);
}
static __inline__ void
AG_WriteUint16(AG_DataSource *ds, Uint16 u16)
{
	Uint16 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE16(u16) :
	                                                 AG_SwapLE16(u16);
	if (AG_Write(ds, &i, sizeof(i), 1) != 0) { AG_FatalError(NULL); }
}
static __inline__ void
AG_WriteUint16At(AG_DataSource *ds, Uint16 u16, off_t pos)
{
	Uint16 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE16(u16) :
	                                                 AG_SwapLE16(u16);
	if (AG_WriteAt(ds, &i, sizeof(i), 1, pos) != 0) { AG_FatalError(NULL); }
}
static __inline__ Uint32
AG_ReadUint32(AG_DataSource *ds)
{
	Uint32 i;
	if (AG_Read(ds, &i, sizeof(i), 1) != 0) { AG_FatalError(NULL); }
	return ((ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(i) :
	                                              AG_SwapLE32(i));
}
static __inline__ void
AG_WriteUint32(AG_DataSource *ds, Uint32 u32)
{
	Uint32 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(u32) :
	                                                 AG_SwapLE32(u32);
	if (AG_Write(ds, &i, sizeof(i), 1) != 0) { AG_FatalError(NULL); }
}
static __inline__ void
AG_WriteUint32At(AG_DataSource *ds, Uint32 u32, off_t pos)
{
	Uint32 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(u32) :
	                                                 AG_SwapLE32(u32);
	if (AG_WriteAt(ds, &i, sizeof(i), 1, pos) != 0) { AG_FatalError(NULL); }
}

#ifdef HAVE_64BIT
static __inline__ Uint64
AG_ReadUint64(AG_DataSource *ds)
{
	Uint64 i;
	if (AG_Read(ds, &i, sizeof(i), 1) != 0) { AG_FatalError(NULL); }
	return ((ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE64(i) :
	                                              AG_SwapLE64(i));
}
static __inline__ void
AG_WriteUint64(AG_DataSource *ds, Uint64 u64)
{
	Uint64 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE64(u64) :
	                                                 AG_SwapLE64(u64);
	if (AG_Write(ds, &i, sizeof(i), 1) != 0) { AG_FatalError(NULL); }
}
static __inline__ void
AG_WriteUint64At(AG_DataSource *ds, Uint64 u64, off_t pos)
{
	Uint64 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE64(u64) :
	                                                 AG_SwapLE64(u64);
	if (AG_WriteAt(ds, &i, sizeof(i), 1, pos) != 0) { AG_FatalError(NULL); }
}
#endif /* HAVE_64BIT */

__END_DECLS
#include "close_code.h"
