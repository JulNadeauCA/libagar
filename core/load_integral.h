/*	Public domain	*/
/*
 * Serialization of integers.
 */

#ifndef	_AGAR_CORE_LOAD_INTEGRAL_H_
#define	_AGAR_CORE_LOAD_INTEGRAL_H_
#include <agar/core/begin.h>

__BEGIN_DECLS
#define	AG_ReadSint8(ds)	 ((Sint8)AG_ReadUint8(ds))
#define	AG_ReadSint8v(ds,v)	 AG_ReadUint8v(ds, (Sint8 *)(v))
#define	AG_WriteSint8(ds,v)	 AG_WriteUint8((ds),(Uint8)(v))
#define	AG_WriteSint8v(ds,v)	 AG_WriteUint8v((ds),(Uint8 *)(v))
#define	AG_WriteSint8At(ds,v,o)	 AG_WriteUint8At((ds),(Uint8)(v),(o))

#define	AG_ReadSint16(ds)	 ((Sint16)AG_ReadUint16(ds))
#define	AG_ReadSint16v(ds,v)	 AG_ReadUint16v(ds, (Sint16 *)(v))
#define	AG_WriteSint16(ds,v)	 AG_WriteUint16((ds),(Uint16)(v))
#define	AG_WriteSint16v(ds,v)	 AG_WriteUint16v((ds),(Uint16 *)(v))
#define	AG_WriteSint16At(ds,v,o) AG_WriteUint16At((ds),(Uint16)(v),(o))

#define	AG_ReadSint32(ds)	 ((Sint32)AG_ReadUint32(ds))
#define	AG_ReadSint32v(ds,v)	 AG_ReadUint32v(ds, (Sint32 *)(v))
#define	AG_WriteSint32(ds,v)	 AG_WriteUint32((ds),(Uint32)(v))
#define	AG_WriteSint32v(ds,v)	 AG_WriteUint32v((ds),(Uint32 *)(v))
#define	AG_WriteSint32At(ds,v,o) AG_WriteUint32At((ds),(Uint32)(v),(o))

#ifdef AG_HAVE_64BIT
# define AG_ReadSint64(ds)	  ((Sint64)AG_ReadUint64(ds))
# define AG_ReadSint64v(ds,v)	  AG_ReadUint64v(ds, (Sint64 *)(v))
# define AG_WriteSint64(ds,v)	  AG_WriteUint64((ds),(Uint64)(v))
# define AG_WriteSint64v(ds,v)	  AG_WriteUint64v((ds),(Uint64 *)(v))
# define AG_WriteSint64At(ds,v,o) AG_WriteUint64At((ds),(Uint64)(v),(o))
#endif

/*
 * 8-bit integers
 */
static __inline__ Uint8
AG_ReadUint8(AG_DataSource *_Nonnull ds)
{
	Uint8 i;
#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_UINT8) == -1)
		return (0);
#endif
	if (AG_Read(ds, &i, sizeof(i)) != 0) {
		AG_DataSourceError(ds, NULL);
		return (0);
	}
	return (i);
}
static __inline__ int
AG_ReadUint8v(AG_DataSource *_Nonnull ds, Uint8 *_Nonnull v)
{
	Uint8 i;
#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_UINT8) == -1)
		return (-1);
#endif
	if (AG_Read(ds, &i, sizeof(i)) != 0) {
		return (-1);
	}
	*v = i;
	return (0);
}
static __inline__ void
AG_WriteUint8(AG_DataSource *_Nonnull ds, Uint8 i)
{
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCode(ds, AG_SOURCE_UINT8); }
#endif
	if (AG_Write(ds, &i, sizeof(i)) != 0)
		AG_DataSourceError(ds, NULL);
}
static __inline__ int
AG_WriteUint8v(AG_DataSource *_Nonnull ds, const Uint8 *_Nonnull i)
{
#ifdef AG_DEBUG
	if (ds->debug && AG_WriteTypeCodeE(ds, AG_SOURCE_UINT8) == -1)
		return (-1);
#endif
	return AG_Write(ds, i, sizeof(Uint8));
}
static __inline__ void
AG_WriteUint8At(AG_DataSource *_Nonnull ds, Uint8 i, AG_Offset pos)
{
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCodeAt(ds, AG_SOURCE_UINT8, pos); }
#endif
	if (AG_WriteAt(ds, &i, sizeof(i), AG_WRITEAT_DEBUGOFFS(ds,pos)) != 0)
		AG_DataSourceError(ds, NULL);
}

/*
 * 16-bit integers
 */
static __inline__ Uint16
AG_ReadUint16(AG_DataSource *_Nonnull ds)
{
	Uint16 i;

#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_UINT16) == -1)
		return (0);
#endif
	if (AG_Read(ds, &i, sizeof(i)) != 0) {
		AG_DataSourceError(ds, NULL);
		return (0);
	}
	return (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE16(i) :
	                                             AG_SwapLE16(i);
}
static __inline__ int
AG_ReadUint16v(AG_DataSource *_Nonnull ds, Uint16 *_Nonnull v)
{
	Uint16 i;

#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_UINT16) == -1)
		return (-1);
#endif
	if (AG_Read(ds, &i, sizeof(i)) != 0) {
		return (-1);
	}
	*v = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE16(i) :
	                                           AG_SwapLE16(i);
	return (0);
}
static __inline__ void
AG_WriteUint16(AG_DataSource *_Nonnull ds, Uint16 u16)
{
	Uint16 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE16(u16) :
	                                                 AG_SwapLE16(u16);
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCode(ds, AG_SOURCE_UINT16); }
#endif
	if (AG_Write(ds, &i, sizeof(i)) != 0)
		AG_DataSourceError(ds, NULL);
}
static __inline__ int
AG_WriteUint16v(AG_DataSource *_Nonnull ds, const Uint16 *_Nonnull u16)
{
	Uint16 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE16(*u16) :
	                                                 AG_SwapLE16(*u16);
#ifdef AG_DEBUG
	if (ds->debug && AG_WriteTypeCodeE(ds, AG_SOURCE_UINT16) == -1)
		return (-1);
#endif
	return AG_Write(ds, &i, sizeof(i));
}
static __inline__ void
AG_WriteUint16At(AG_DataSource *_Nonnull ds, Uint16 u16, AG_Offset pos)
{
	Uint16 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE16(u16) :
	                                                 AG_SwapLE16(u16);
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCodeAt(ds, AG_SOURCE_UINT16, pos); }
#endif
	if (AG_WriteAt(ds, &i, sizeof(i), AG_WRITEAT_DEBUGOFFS(ds,pos)) != 0)
		AG_DataSourceError(ds, NULL);
}

/*
 * 32-bit integers
 */
static __inline__ Uint32
AG_ReadUint32(AG_DataSource *_Nonnull ds)
{
	Uint32 i;
#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_UINT32) == -1)
		return (0);
#endif
	if (AG_Read(ds, &i, sizeof(i)) != 0) {
		AG_DataSourceError(ds, NULL);
		return (0);
	}
	return ((ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(i) :
	                                              AG_SwapLE32(i));
}
static __inline__ int
AG_ReadUint32v(AG_DataSource *_Nonnull ds, Uint32 *_Nonnull v)
{
	Uint32 i;
#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_UINT32) == -1)
		return (-1);
#endif
	if (AG_Read(ds, &i, sizeof(i)) != 0) {
		return (-1);
	}
	*v = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(i) :
	                                           AG_SwapLE32(i);
	return (0);
}
static __inline__ void
AG_WriteUint32(AG_DataSource *_Nonnull ds, Uint32 u32)
{
	Uint32 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(u32) :
	                                                 AG_SwapLE32(u32);
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCode(ds, AG_SOURCE_UINT32); }
#endif
	if (AG_Write(ds, &i, sizeof(i)) != 0)
		AG_DataSourceError(ds, NULL);
}
static __inline__ int
AG_WriteUint32v(AG_DataSource *_Nonnull ds, const Uint32 *_Nonnull u32)
{
	Uint32 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(*u32) :
	                                                 AG_SwapLE32(*u32);
#ifdef AG_DEBUG
	if (ds->debug && AG_WriteTypeCodeE(ds, AG_SOURCE_UINT32) == -1)
		return (-1);
#endif
	return AG_Write(ds, &i, sizeof(i));
}
static __inline__ void
AG_WriteUint32At(AG_DataSource *_Nonnull ds, Uint32 u32, AG_Offset pos)
{
	Uint32 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(u32) :
	                                                 AG_SwapLE32(u32);
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCodeAt(ds, AG_SOURCE_UINT32, pos); }
#endif
	if (AG_WriteAt(ds, &i, sizeof(i), AG_WRITEAT_DEBUGOFFS(ds,pos)) != 0)
		AG_DataSourceError(ds, NULL);
}

#ifdef AG_HAVE_64BIT
/*
 * 64-bit integers
 */
static __inline__ Uint64
AG_ReadUint64(AG_DataSource *_Nonnull ds)
{
	Uint64 i;
#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_UINT64) == -1)
		return (0);
#endif
	if (AG_Read(ds, &i, sizeof(i)) != 0) {
		AG_DataSourceError(ds, NULL);
		return (0);
	}
	return ((ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE64(i) :
	                                              AG_SwapLE64(i));
}
static __inline__ int
AG_ReadUint64v(AG_DataSource *_Nonnull ds, Uint64 *_Nonnull v)
{
	Uint64 i;
#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_UINT64) == -1)
		return (-1);
#endif
	if (AG_Read(ds, &i, sizeof(i)) != 0) {
		return (-1);
	}
	*v = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE64(i) :
	                                           AG_SwapLE64(i);
	return (0);
}
static __inline__ void
AG_WriteUint64(AG_DataSource *_Nonnull ds, Uint64 u64)
{
	Uint64 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE64(u64) :
	                                                 AG_SwapLE64(u64);
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCode(ds, AG_SOURCE_UINT64); }
#endif
	if (AG_Write(ds, &i, sizeof(i)) != 0)
		AG_DataSourceError(ds, NULL);
}
static __inline__ int
AG_WriteUint64v(AG_DataSource *_Nonnull ds, const Uint64 *_Nonnull u64)
{
	Uint64 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE64(*u64) :
	                                                 AG_SwapLE64(*u64);
#ifdef AG_DEBUG
	if (ds->debug && AG_WriteTypeCodeE(ds, AG_SOURCE_UINT64) == -1)
		return (-1);
#endif
	return AG_Write(ds, &i, sizeof(i));
}
static __inline__ void
AG_WriteUint64At(AG_DataSource *_Nonnull ds, Uint64 u64, AG_Offset pos)
{
	Uint64 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE64(u64) :
	                                                 AG_SwapLE64(u64);
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCodeAt(ds, AG_SOURCE_UINT64, pos); }
#endif
	if (AG_WriteAt(ds, &i, sizeof(i), AG_WRITEAT_DEBUGOFFS(ds,pos)) != 0)
		AG_DataSourceError(ds, NULL);
}
#endif /* AG_HAVE_64BIT */

__END_DECLS
#include <agar/core/close.h>

#endif /* _AGAR_CORE_LOAD_INTEGRAL_H_ */
