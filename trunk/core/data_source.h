/*	Public domain	*/

#ifndef _AGAR_CORE_DATA_SOURCE_H_
#define _AGAR_CORE_DATA_SOURCE_H_
#include <agar/core/begin.h>

struct ag_event;
struct ag_net_socket;

/* Data source byte order */
enum ag_byte_order {
	AG_BYTEORDER_BE,		/* Big-endian */
	AG_BYTEORDER_LE			/* Little-endian */
};

/* Mode for seek() operation */
enum ag_seek_mode {
	AG_SEEK_SET,
	AG_SEEK_CUR,
	AG_SEEK_END
};

/*
 * Signatures for type-safety checks.
 * The range 0x41470000 - 0x4147ffff is reserved by Agar.
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
	AG_SOURCE_COLOR_RGBA =	0x4147000d
};

/* Generic data source object */
typedef struct ag_data_source {
	AG_Mutex lock;				/* Lock on all operations */
	int debug;
	struct ag_event *errorFn;		/* Exception handler */
	enum ag_byte_order byte_order;		/* Byte order of source */
	size_t wrLast;				/* Last write count (bytes) */
	size_t rdLast;				/* Last read count (bytes) */
	size_t wrTotal;				/* Total write count (bytes) */
	size_t rdTotal;				/* Total read count (bytes) */

	int   (*read)(struct ag_data_source *, void *, size_t, size_t *);
	int   (*read_at)(struct ag_data_source *, void *, size_t, off_t, size_t *);
	int   (*write)(struct ag_data_source *, const void *, size_t, size_t *);
	int   (*write_at)(struct ag_data_source *, const void *, size_t, off_t, size_t *);
	off_t (*tell)(struct ag_data_source *);
	int   (*seek)(struct ag_data_source *, off_t, enum ag_seek_mode);
	void  (*close)(struct ag_data_source *);
} AG_DataSource;

/* File */
typedef struct ag_file_source {
	struct ag_data_source ds;
	char *path;			/* Open file path (or NULL) */
	FILE *file;			/* Opened file */
} AG_FileSource;

/* Memory region */
typedef struct ag_core_source {
	struct ag_data_source ds;
	Uint8 *data;			/* Pointer to data */
	size_t size;			/* Current size */
	off_t  offs;			/* Current position */
} AG_CoreSource;

/* Memory region (const) */
typedef struct ag_const_core_source {
	struct ag_data_source ds;
	const Uint8 *data;		/* Pointer to data */
	size_t size;			/* Current size */
	off_t  offs;			/* Current position */
} AG_ConstCoreSource;

/* Network socket */
typedef struct ag_net_socket_source {
	struct ag_data_source ds;
	struct ag_net_socket *sock;	/* Network socket */
} AG_NetSocketSource;

#define AG_DATA_SOURCE(ds) ((AG_DataSource *)(ds))
#define AG_FILE_SOURCE(ds) ((AG_FileSource *)(ds))
#define AG_CORE_SOURCE(ds) ((AG_CoreSource *)(ds))
#define AG_CONST_CORE_SOURCE(ds) ((AG_ConstCoreSource *)(ds))
#define AG_NET_SOCKET_SOURCE(ds) ((AG_NetSocketSource *)(ds))

__BEGIN_DECLS
void AG_DataSourceInitSubsystem(void);
void AG_DataSourceDestroySubsystem(void);

void AG_DataSourceInit(AG_DataSource *);
void AG_DataSourceDestroy(AG_DataSource *);
void AG_DataSourceSetDebug(AG_DataSource *, int);
void AG_DataSourceSetErrorFn(AG_DataSource *, void (*)(struct ag_event *),
                             const char *, ...);
void AG_DataSourceError(AG_DataSource *, const char *, ...);
void AG_SetByteOrder(AG_DataSource *, enum ag_byte_order);
void AG_SetSourceDebug(AG_DataSource *, int);
void AG_CloseDataSource(AG_DataSource *);

AG_DataSource *AG_OpenFile(const char *, const char *);
AG_DataSource *AG_OpenFileHandle(FILE *);
AG_DataSource *AG_OpenCore(void *, size_t)
                           BOUNDED_ATTRIBUTE(__buffer__,1,2);
AG_DataSource *AG_OpenConstCore(const void *, size_t)
                                BOUNDED_ATTRIBUTE(__buffer__,1,2);
AG_DataSource *AG_OpenAutoCore(void);
AG_DataSource *AG_OpenNetSocket(struct ag_net_socket *);

int     AG_Read(AG_DataSource *, void *, size_t)
                BOUNDED_ATTRIBUTE(__buffer__,2,3);
int     AG_ReadP(AG_DataSource *, void *, size_t, size_t *)
                 BOUNDED_ATTRIBUTE(__buffer__,2,3);
int     AG_ReadAt(AG_DataSource *, void *, size_t, off_t);
int     AG_ReadAtP(AG_DataSource *, void *, size_t, off_t, size_t *);

int     AG_Write(AG_DataSource *, const void *, size_t)
                 BOUNDED_ATTRIBUTE(__buffer__,2,3);
int     AG_WriteP(AG_DataSource *, const void *, size_t, size_t *)
                  BOUNDED_ATTRIBUTE(__buffer__,2,3);
int     AG_WriteAt(AG_DataSource *, const void *, size_t, off_t);
int     AG_WriteAtP(AG_DataSource *, const void *, size_t, off_t, size_t *);

void    AG_CloseFile(AG_DataSource *);
#define AG_CloseFileHandle(ds) AG_CloseFile(ds)
void    AG_CloseCore(AG_DataSource *);
#define AG_CloseConstCore(ds) AG_CloseCore(ds)
void    AG_CloseAutoCore(AG_DataSource *);
void    AG_CloseNetSocket(AG_DataSource *);

void    AG_WriteTypeCode(AG_DataSource *, Uint32);
void    AG_WriteTypeCodeAt(AG_DataSource *, Uint32, off_t);
int     AG_WriteTypeCodeE(AG_DataSource *, Uint32);
int     AG_CheckTypeCode(AG_DataSource *, Uint32);

#define AG_LockDataSource(ds) AG_MutexLock(&(ds)->lock);
#define AG_UnlockDataSource(ds) AG_MutexUnlock(&(ds)->lock);

/* For AG_WriteFooAt() */
#define AG_WRITEAT_DEBUGOFFS(ds,pos) ((ds)->debug ? (pos)+sizeof(Uint32) : (pos))

/* Reallocate the buffer of a dynamically-allocated memory source. */
static __inline__ int
AG_DataSourceRealloc(void *obj, size_t size)
{
	AG_CoreSource *cs = (AG_CoreSource *)obj;
	Uint8 *dataNew;
		
	if ((dataNew = (Uint8 *)AG_TryRealloc(cs->data, size)) == NULL) {
		return (-1);
	}
	cs->data = dataNew;
	cs->size = size;
	return (0);
}

/* Return current position. */
static __inline__ off_t
AG_Tell(AG_DataSource *ds)
{
	off_t pos;
	AG_MutexLock(&ds->lock);
	pos = (ds->tell != NULL) ? ds->tell(ds) : 0;
	AG_MutexUnlock(&ds->lock);
	return (pos);
}

/* Seek to position. */
static __inline__ int
AG_Seek(AG_DataSource *ds, off_t pos, enum ag_seek_mode mode)
{
	int rv;
	AG_MutexLock(&ds->lock);
	rv = ds->seek(ds, pos, mode);
	AG_MutexUnlock(&ds->lock);
	return (rv);
}
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_DATA_SOURCE_H_ */
