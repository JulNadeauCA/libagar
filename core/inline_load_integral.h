/*	Public domain	*/

/*
 * Integer serialization
 */

/*
 * 8-bit integers
 */
#ifdef AG_INLINE_HEADER
static __inline__ Uint8
AG_ReadUint8(AG_DataSource *_Nonnull ds)
#else
Uint8
ag_read_uint8(AG_DataSource *ds)
#endif
{
	Uint8 i;
#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_UINT8) == -1) { return (0); }
#endif
	if (AG_Read(ds, &i, sizeof(i)) != 0) {
		AG_DataSourceError(ds, NULL);
		return (0);
	}
	return (i);
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_WriteUint8(AG_DataSource *_Nonnull ds, Uint8 i)
#else
void
ag_write_uint8(AG_DataSource *ds, Uint8 i)
#endif
{
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCode(ds, AG_SOURCE_UINT8); }
#endif
	if (AG_Write(ds, &i, sizeof(i)) != 0)
		AG_DataSourceError(ds, NULL);
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_WriteUint8At(AG_DataSource *_Nonnull ds, Uint8 i, AG_Offset pos)
#else
void
ag_write_uint8_at(AG_DataSource *_Nonnull ds, Uint8 i, AG_Offset pos)
#endif
{
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCodeAt(ds, AG_SOURCE_UINT8, pos); }
#endif
	if (AG_WriteAt(ds, &i, sizeof(i), AG_WRITEAT_OFFSET(ds,pos)) != 0)
		AG_DataSourceError(ds, NULL);
}

#ifdef AG_INLINE_HEADER
static __inline__ Sint8
AG_ReadSint8(AG_DataSource *_Nonnull ds)
#else
Sint8
ag_read_sint8(AG_DataSource *ds)
#endif
{
	Sint8 i;
#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_SINT8) == -1) { return (0); }
#endif
	if (AG_Read(ds, &i, sizeof(i)) != 0) {
		AG_DataSourceError(ds, NULL);
		return (0);
	}
	return (i);
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_WriteSint8(AG_DataSource *_Nonnull ds, Sint8 i)
#else
void
ag_write_sint8(AG_DataSource *ds, Sint8 i)
#endif
{
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCode(ds, AG_SOURCE_SINT8); }
#endif
	if (AG_Write(ds, &i, sizeof(i)) != 0)
		AG_DataSourceError(ds, NULL);
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_WriteSint8At(AG_DataSource *_Nonnull ds, Sint8 i, AG_Offset pos)
#else
void
ag_write_sint8_at(AG_DataSource *ds, Sint8 i, AG_Offset pos)
#endif
{
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCodeAt(ds, AG_SOURCE_SINT8, pos); }
#endif
	if (AG_WriteAt(ds, &i, sizeof(i), AG_WRITEAT_OFFSET(ds,pos)) != 0)
		AG_DataSourceError(ds, NULL);
}

/*
 * 16-bit integers
 */
#ifdef AG_INLINE_HEADER
static __inline__ Uint16
AG_ReadUint16(AG_DataSource *_Nonnull ds)
#else
Uint16
ag_read_uint16(AG_DataSource *ds)
#endif
{
	Uint16 i;
#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_UINT16) == -1) { return (0); }
#endif
	if (AG_Read(ds, &i, sizeof(i)) != 0) {
		AG_DataSourceError(ds, NULL);
		return (0);
	}
	return (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE16(i) :
	                                             AG_SwapLE16(i);
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_WriteUint16(AG_DataSource *_Nonnull ds, Uint16 u16)
#else
void
ag_write_uint16(AG_DataSource *ds, Uint16 u16)
#endif
{
	Uint16 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE16(u16) :
	                                                 AG_SwapLE16(u16);
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCode(ds, AG_SOURCE_UINT16); }
#endif
	if (AG_Write(ds, &i, sizeof(i)) != 0)
		AG_DataSourceError(ds, NULL);
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_WriteUint16At(AG_DataSource *_Nonnull ds, Uint16 u16, AG_Offset pos)
#else
void
ag_write_uint16_at(AG_DataSource *ds, Uint16 u16, AG_Offset pos)
#endif
{
	Uint16 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE16(u16) :
	                                                 AG_SwapLE16(u16);
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCodeAt(ds, AG_SOURCE_UINT16, pos); }
#endif
	if (AG_WriteAt(ds, &i, sizeof(i), AG_WRITEAT_OFFSET(ds,pos)) != 0)
		AG_DataSourceError(ds, NULL);
}

#ifdef AG_INLINE_HEADER
static __inline__ Sint16
AG_ReadSint16(AG_DataSource *_Nonnull ds)
#else
Sint16
ag_read_sint16(AG_DataSource *ds)
#endif
{
	Sint16 i;

#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_SINT16) == -1) { return (0); }
#endif
	if (AG_Read(ds, &i, sizeof(i)) != 0) {
		AG_DataSourceError(ds, NULL);
		return (0);
	}
	return (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE16(i) :
	                                             AG_SwapLE16(i);
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_WriteSint16(AG_DataSource *_Nonnull ds, Sint16 s16)
#else
void
ag_write_sint16(AG_DataSource *ds, Sint16 s16)
#endif
{
	Sint16 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE16(s16) :
	                                                 AG_SwapLE16(s16);
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCode(ds, AG_SOURCE_SINT16); }
#endif
	if (AG_Write(ds, &i, sizeof(i)) != 0)
		AG_DataSourceError(ds, NULL);
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_WriteSint16At(AG_DataSource *_Nonnull ds, Sint16 s16, AG_Offset pos)
#else
void
ag_write_sint16_at(AG_DataSource *ds, Sint16 s16, AG_Offset pos)
#endif
{
	Sint16 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE16(s16) :
	                                                 AG_SwapLE16(s16);
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCodeAt(ds, AG_SOURCE_SINT16, pos); }
#endif
	if (AG_WriteAt(ds, &i, sizeof(i), AG_WRITEAT_OFFSET(ds,pos)) != 0)
		AG_DataSourceError(ds, NULL);
}

/*
 * 32-bit integers
 */
#ifdef AG_INLINE_HEADER
static __inline__ Uint32
AG_ReadUint32(AG_DataSource *_Nonnull ds)
#else
Uint32
ag_read_uint32(AG_DataSource *ds)
#endif
{
	Uint32 i;
#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_UINT32) == -1) { return (0); }
#endif
	if (AG_Read(ds, &i, sizeof(i)) != 0) {
		AG_DataSourceError(ds, NULL);
		return (0);
	}
	return ((ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(i) :
	                                              AG_SwapLE32(i));
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_WriteUint32(AG_DataSource *_Nonnull ds, Uint32 u32)
#else
void
ag_write_uint32(AG_DataSource *ds, Uint32 u32)
#endif
{
	Uint32 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(u32) :
	                                                 AG_SwapLE32(u32);
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCode(ds, AG_SOURCE_UINT32); }
#endif
	if (AG_Write(ds, &i, sizeof(i)) != 0)
		AG_DataSourceError(ds, NULL);
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_WriteUint32At(AG_DataSource *_Nonnull ds, Uint32 u32, AG_Offset pos)
#else
void
ag_write_uint32_at(AG_DataSource *ds, Uint32 u32, AG_Offset pos)
#endif
{
	Uint32 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(u32) :
	                                                 AG_SwapLE32(u32);
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCodeAt(ds, AG_SOURCE_UINT32, pos); }
#endif
	if (AG_WriteAt(ds, &i, sizeof(i), AG_WRITEAT_OFFSET(ds,pos)) != 0)
		AG_DataSourceError(ds, NULL);
}

#ifdef AG_INLINE_HEADER
static __inline__ Sint32
AG_ReadSint32(AG_DataSource *_Nonnull ds)
#else
Sint32
ag_read_sint32(AG_DataSource *ds)
#endif
{
	Sint32 i;
#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_SINT32) == -1) { return (0); }
#endif
	if (AG_Read(ds, &i, sizeof(i)) != 0) {
		AG_DataSourceError(ds, NULL);
		return (0);
	}
	return ((ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(i) :
	                                              AG_SwapLE32(i));
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_WriteSint32(AG_DataSource *_Nonnull ds, Sint32 s32)
#else
void
ag_write_sint32(AG_DataSource *ds, Sint32 s32)
#endif
{
	Sint32 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(s32) :
	                                                 AG_SwapLE32(s32);
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCode(ds, AG_SOURCE_SINT32); }
#endif
	if (AG_Write(ds, &i, sizeof(i)) != 0)
		AG_DataSourceError(ds, NULL);
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_WriteSint32At(AG_DataSource *_Nonnull ds, Sint32 s32, AG_Offset pos)
#else
void
ag_write_sint32_at(AG_DataSource *ds, Sint32 s32, AG_Offset pos)
#endif
{
	Sint32 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(s32) :
	                                                 AG_SwapLE32(s32);
#ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCodeAt(ds, AG_SOURCE_SINT32, pos); }
#endif
	if (AG_WriteAt(ds, &i, sizeof(i), AG_WRITEAT_OFFSET(ds,pos)) != 0)
		AG_DataSourceError(ds, NULL);
}

#ifdef AG_HAVE_64BIT
/*
 * 64-bit integers
 */
# ifdef AG_INLINE_HEADER
static __inline__ Uint64
AG_ReadUint64(AG_DataSource *_Nonnull ds)
# else
Uint64
ag_read_uint64(AG_DataSource *ds)
# endif
{
	Uint64 i;
# ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_UINT64) == -1) { return (0); }
# endif
	if (AG_Read(ds, &i, sizeof(i)) != 0) {
		AG_DataSourceError(ds, NULL);
		return (0);
	}
	return ((ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE64(i) :
	                                              AG_SwapLE64(i));
}

# ifdef AG_INLINE_HEADER
static __inline__ void
AG_WriteUint64(AG_DataSource *_Nonnull ds, Uint64 u64)
# else
void
ag_write_uint64(AG_DataSource *ds, Uint64 u64)
# endif
{
	Uint64 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE64(u64) :
	                                                 AG_SwapLE64(u64);
# ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCode(ds, AG_SOURCE_UINT64); }
# endif
	if (AG_Write(ds, &i, sizeof(i)) != 0)
		AG_DataSourceError(ds, NULL);
}

# ifdef AG_INLINE_HEADER
static __inline__ void
AG_WriteUint64At(AG_DataSource *_Nonnull ds, Uint64 u64, AG_Offset pos)
# else
void
ag_write_uint64_at(AG_DataSource *ds, Uint64 u64, AG_Offset pos)
# endif
{
	Uint64 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE64(u64) :
	                                                 AG_SwapLE64(u64);
# ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCodeAt(ds, AG_SOURCE_UINT64, pos); }
# endif
	if (AG_WriteAt(ds, &i, sizeof(i), AG_WRITEAT_OFFSET(ds,pos)) != 0)
		AG_DataSourceError(ds, NULL);
}

# ifdef AG_INLINE_HEADER
static __inline__ Sint64
AG_ReadSint64(AG_DataSource *_Nonnull ds)
# else
Sint64
ag_read_sint64(AG_DataSource *ds)
# endif
{
	Sint64 i;
# ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_SINT64) == -1) { return (0); }
# endif
	if (AG_Read(ds, &i, sizeof(i)) != 0) {
		AG_DataSourceError(ds, NULL);
		return (0);
	}
	return ((ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE64(i) :
	                                              AG_SwapLE64(i));
}

# ifdef AG_INLINE_HEADER
static __inline__ void
AG_WriteSint64(AG_DataSource *_Nonnull ds, Sint64 s64)
# else
void
ag_write_sint64(AG_DataSource *ds, Sint64 s64)
# endif
{
	Sint64 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE64(s64) :
	                                                 AG_SwapLE64(s64);
# ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCode(ds, AG_SOURCE_SINT64); }
# endif
	if (AG_Write(ds, &i, sizeof(i)) != 0)
		AG_DataSourceError(ds, NULL);
}

# ifdef AG_INLINE_HEADER
static __inline__ void
AG_WriteSint64At(AG_DataSource *_Nonnull ds, Sint64 s64, AG_Offset pos)
# else
void
ag_write_sint64_at(AG_DataSource *ds, Sint64 s64, AG_Offset pos)
# endif
{
	Sint64 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE64(s64) :
	                                                 AG_SwapLE64(s64);
# ifdef AG_DEBUG
	if (ds->debug) { AG_WriteTypeCodeAt(ds, AG_SOURCE_SINT64, pos); }
# endif
	if (AG_WriteAt(ds, &i, sizeof(i), AG_WRITEAT_OFFSET(ds,pos)) != 0)
		AG_DataSourceError(ds, NULL);
}
#endif /* AG_HAVE_64BIT */
