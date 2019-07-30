/*	Public domain	*/

#ifndef _AGAR_CORE_DATA_SOURCE_H_
#define _AGAR_CORE_DATA_SOURCE_H_

#ifdef AG_SERIALIZATION
#include <agar/core/begin.h>

struct ag_event;
struct ag_net_socket;

/* Data source byte order */
typedef enum ag_byte_order {
	AG_BYTEORDER_BE,		/* Big-endian */
	AG_BYTEORDER_LE			/* Little-endian */
} AG_ByteOrder;

/* Mode for seek() operation */
enum ag_seek_mode {
	AG_SEEK_SET,
	AG_SEEK_CUR,
	AG_SEEK_END
};

/*
 * Signatures for serialization markers (DEBUG mode only).
 * Range 0x41470000 - 0x4147ffff is reserved by ag_core.
 */
enum ag_data_source_type {
	AG_SOURCE_UINT8 =	0x41470001,
	AG_SOURCE_SINT8	=	0x41470002,
	AG_SOURCE_UINT16 =	0x41470003,
	AG_SOURCE_SINT16 =	0x41470004,
	AG_SOURCE_UINT32 =	0x41470005,
	AG_SOURCE_SINT32 =	0x41470006,
	AG_SOURCE_UINT64 =	0x41470007,
	AG_SOURCE_SINT64 =	0x41470008,
	AG_SOURCE_FLOAT =	0x41470009,
	AG_SOURCE_DOUBLE =	0x4147000a,
	AG_SOURCE_LONG_DOUBLE =	0x4147000b,
	AG_SOURCE_STRING =	0x4147000c,
	AG_SOURCE_COLOR_RGBA =	0x4147000d,
	AG_SOURCE_STRING_PAD =	0x4147000e,
};

/* Generic data source object */
typedef struct ag_data_source {
	_Nonnull_Mutex AG_Mutex lock;		/* Lock on all operations */
	struct ag_event *_Nullable errorFn;	/* Exception handler */
	int debug;
	AG_ByteOrder byte_order;		/* Effective byte order */
	AG_Size wrLast;				/* Last write count (bytes) */
	AG_Size rdLast;				/* Last read count (bytes) */
	AG_Size wrTotal;			/* Total write count (bytes) */
	AG_Size rdTotal;			/* Total read count (bytes) */

	int   (*_Nullable read)(struct ag_data_source *_Nonnull,
	                        void *_Nonnull, AG_Size,
				AG_Size *_Nonnull);
	int   (*_Nullable read_at)(struct ag_data_source *_Nonnull,
	                           void *_Nonnull, AG_Size, AG_Offset,
				   AG_Size *_Nonnull);
	int   (*_Nullable write)(struct ag_data_source *_Nonnull,
	                         const void *_Nonnull, AG_Size,
				 AG_Size *_Nonnull);
	int   (*_Nullable write_at)(struct ag_data_source *_Nonnull,
	                            const void *_Nonnull, AG_Size,
				    AG_Offset, AG_Size *_Nonnull);
	AG_Offset (*_Nullable tell)(struct ag_data_source *_Nonnull);
	int   (*_Nullable seek)(struct ag_data_source *_Nonnull, AG_Offset,
	                        enum ag_seek_mode);
	void  (*_Nullable close)(struct ag_data_source *_Nonnull);
} AG_DataSource;

/* File */
typedef struct ag_file_source {
	struct ag_data_source ds;
	char *_Nullable path;		/* Open file path */
	void *_Nonnull file;		/* Opened FILE */
} AG_FileSource;

/* Memory region */
typedef struct ag_core_source {
	struct ag_data_source ds;
	Uint8 *_Nonnull data;		/* Pointer to data */
	AG_Size size;			/* Current size */
	AG_Offset offs;			/* Current position */
} AG_CoreSource;

/* Read-only memory region */
typedef struct ag_const_core_source {
	struct ag_data_source ds;
	const Uint8 *_Nonnull data;	/* Pointer to data */
	AG_Size size;			/* Current size */
	AG_Offset offs;			/* Current position */
} AG_ConstCoreSource;

/* Network socket */
typedef struct ag_net_socket_source {
	struct ag_data_source ds;
	struct ag_net_socket *_Nonnull sock;	/* Network socket */
} AG_NetSocketSource;

#define AG_DATA_SOURCE(ds) ((AG_DataSource *)(ds))
#define AG_FILE_SOURCE(ds) ((AG_FileSource *)(ds))
#define AG_CORE_SOURCE(ds) ((AG_CoreSource *)(ds))
#define AG_CONST_CORE_SOURCE(ds) ((AG_ConstCoreSource *)(ds))
#define AG_NET_SOCKET_SOURCE(ds) ((AG_NetSocketSource *)(ds))

/* For AG_Write<Type>At() */
#ifdef AG_DEBUG
# define AG_WRITEAT_OFFSET(ds,pos) ((ds)->debug ? (pos)+sizeof(Uint32) : (pos))
#else
# define AG_WRITEAT_OFFSET(ds,pos) (pos)
#endif

__BEGIN_DECLS
void AG_DataSourceInitSubsystem(void);
void AG_DataSourceDestroySubsystem(void);

void AG_DataSourceInit(AG_DataSource *_Nonnull);
void AG_DataSourceSetDebug(AG_DataSource *_Nonnull, int);
void AG_DataSourceSetErrorFn(AG_DataSource *_Nonnull,
                             void (*_Nullable)(struct ag_event *_Nonnull),
                             const char *_Nullable, ...);
void AG_DataSourceError(AG_DataSource *_Nonnull, const char *_Nullable, ...);

AG_ByteOrder AG_SetByteOrder(AG_DataSource *_Nonnull, AG_ByteOrder);
int          AG_SetSourceDebug(AG_DataSource *_Nonnull, int);

AG_DataSource *_Nullable AG_OpenFile(const char *_Nonnull, const char *_Nonnull)
                                     _Warn_Unused_Result;
AG_DataSource *_Nullable AG_OpenFileHandle(void *_Nonnull) _Warn_Unused_Result;
AG_DataSource *_Nullable AG_OpenCore(void *_Nonnull, AG_Size) _Warn_Unused_Result;
AG_DataSource *_Nullable AG_OpenConstCore(const void *_Nonnull, AG_Size) _Warn_Unused_Result;
AG_DataSource *_Nullable AG_OpenAutoCore(void) _Warn_Unused_Result;
AG_DataSource *_Nullable AG_OpenNetSocket(struct ag_net_socket *_Nonnull) _Warn_Unused_Result;

int AG_Read(AG_DataSource *_Nonnull, void *_Nonnull, AG_Size);
int AG_ReadP(AG_DataSource *_Nonnull, void *_Nonnull, AG_Size, AG_Size *_Nonnull);
int AG_ReadAt(AG_DataSource *_Nonnull, void *_Nonnull, AG_Size, AG_Offset);
int AG_ReadAtP(AG_DataSource *_Nonnull, void *_Nonnull, AG_Size, AG_Offset,
	       AG_Size *_Nullable);

int AG_Write(AG_DataSource *_Nonnull, const void *_Nonnull, AG_Size);
int AG_WriteP(AG_DataSource *_Nonnull, const void *_Nonnull, AG_Size, AG_Size *_Nullable);
int AG_WriteAt(AG_DataSource *_Nonnull, const void *_Nonnull, AG_Size, AG_Offset);
int AG_WriteAtP(AG_DataSource *_Nonnull, const void *_Nonnull, AG_Size, AG_Offset,
                AG_Size *_Nullable);

void    AG_CloseFile(AG_DataSource *_Nonnull);
void    AG_CloseFileHandle(AG_DataSource *_Nonnull);
void    AG_CloseCore(AG_DataSource *_Nonnull);
#define AG_CloseConstCore(ds) AG_CloseCore(ds)
void    AG_CloseAutoCore(AG_DataSource *_Nonnull);
void    AG_CloseNetSocket(AG_DataSource *_Nonnull);

void    AG_WriteTypeCode(AG_DataSource *_Nonnull, Uint32);
void    AG_WriteTypeCodeAt(AG_DataSource *_Nonnull, Uint32, AG_Offset);
int     AG_WriteTypeCodeE(AG_DataSource *_Nonnull, Uint32);
int     AG_CheckTypeCode(AG_DataSource *_Nonnull, Uint32);

#define AG_LockDataSource(ds) AG_MutexLock(&(ds)->lock);
#define AG_UnlockDataSource(ds) AG_MutexUnlock(&(ds)->lock);

int       AG_DataSourceRealloc(void *_Nonnull, AG_Size);
AG_Offset AG_Tell(AG_DataSource *_Nonnull);
int       AG_Seek(AG_DataSource *_Nonnull, AG_Offset, enum ag_seek_mode);
void      AG_CloseDataSource(AG_DataSource *_Nonnull);
void      AG_DataSourceDestroy(AG_DataSource *_Nonnull);
__END_DECLS

#include <agar/core/close.h>
#endif /* AG_SERIALIZATION */
#endif /* _AGAR_CORE_DATA_SOURCE_H_ */
