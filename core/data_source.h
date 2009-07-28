/*	Public domain	*/

#ifndef _AGAR_CORE_DATA_SOURCE_H_
#define _AGAR_CORE_DATA_SOURCE_H_
#include <agar/core/begin.h>

struct ag_event;

/* Datafile byte order */
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

/* Signatures for type-safety checks */
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

typedef enum ag_io_status {
	AG_IO_SUCCESS = 0,
	AG_IO_EOF = 1,
	AG_IO_ERROR = 2,
	AG_IO_UNAVAIL = 3
} AG_IOStatus;

typedef struct ag_data_source {
	AG_Mutex lock;				/* Lock on all operations */
	int debug;

	struct ag_event *errorFn;		/* Exception handler */

	enum ag_byte_order byte_order;		/* Byte order of source */
	size_t wrLast;				/* Last write count (bytes) */
	size_t rdLast;				/* Last read count (bytes) */
	size_t wrTotal;				/* Total write count (bytes) */
	size_t rdTotal;				/* Total read count (bytes) */

	AG_IOStatus (*read)(struct ag_data_source *, void *, size_t, size_t,
	                    size_t *);
	AG_IOStatus (*read_at)(struct ag_data_source *, void *, size_t, size_t,
	                       off_t, size_t *);
	AG_IOStatus (*write)(struct ag_data_source *, const void *, size_t,
	                     size_t, size_t *);
	AG_IOStatus (*write_at)(struct ag_data_source *, const void *, size_t,
	                        size_t, off_t, size_t *);
	off_t       (*tell)(struct ag_data_source *);
	int         (*seek)(struct ag_data_source *, off_t, enum ag_seek_mode);
	void        (*close)(struct ag_data_source *);
} AG_DataSource;

typedef struct ag_file_source {
	struct ag_data_source ds;
	char *path;			/* Open file path (or NULL) */
	FILE *file;			/* Opened file */
} AG_FileSource;

typedef struct ag_core_source {
	struct ag_data_source ds;
	Uint8 *data;			/* Pointer to data */
	size_t size;			/* Current size */
	off_t  offs;			/* Current position */
} AG_CoreSource;

typedef struct ag_const_core_source {
	struct ag_data_source ds;
	const Uint8 *data;		/* Pointer to data */
	size_t size;			/* Current size */
	off_t  offs;			/* Current position */
} AG_ConstCoreSource;

#define AG_DATA_SOURCE(ds) ((AG_DataSource *)(ds))
#define AG_FILE_SOURCE(ds) ((AG_FileSource *)(ds))
#define AG_CORE_SOURCE(ds) ((AG_CoreSource *)(ds))
#define AG_AUTO_CORE_SOURCE(ds) ((AG_CoreSource *)(ds))
#define AG_CONST_CORE_SOURCE(ds) ((AG_ConstCoreSource *)(ds))

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
AG_DataSource *AG_OpenCore(void *, size_t);
AG_DataSource *AG_OpenConstCore(const void *, size_t);
AG_DataSource *AG_OpenAutoCore(void);

void           AG_CloseFile(AG_DataSource *);
#define        AG_CloseFileHandle(ds) AG_CloseFile(ds)
void           AG_CloseCore(AG_DataSource *);
#define        AG_CloseConstCore(ds) AG_CloseCore(ds)
void           AG_CloseAutoCore(AG_DataSource *);

AG_IOStatus    AG_Read(AG_DataSource *, void *, size_t, size_t);
AG_IOStatus    AG_ReadAt(AG_DataSource *, void *, size_t, size_t, off_t);
AG_IOStatus    AG_Write(AG_DataSource *, const void *, size_t, size_t);
AG_IOStatus    AG_WriteAt(AG_DataSource *, const void *, size_t, size_t, off_t);
off_t          AG_Tell(AG_DataSource *);
int            AG_Seek(AG_DataSource *, off_t, enum ag_seek_mode);
void           AG_WriteTypeCode(AG_DataSource *, Uint32);
void           AG_WriteTypeCodeAt(AG_DataSource *, Uint32, off_t);
int            AG_WriteTypev(AG_DataSource *, Uint32);
int            AG_CheckTypev(AG_DataSource *, Uint32);

#define AG_LockDataSource(ds) AG_MutexLock(&(ds)->lock);
#define AG_UnlockDataSource(ds) AG_MutexUnlock(&(ds)->lock);

#define AG_WriteType(ds,code)					\
	do {							\
		if ((ds)->debug)				\
			AG_WriteTypeCode((ds),(code));		\
	} while (0)

#define AG_CHECK_TYPE(ds,code,rv)					\
	do {								\
		if ((ds)->debug && AG_CheckTypev(ds,code) == -1) {	\
			return (rv);					\
		}							\
	} while (0)

#define AG_WriteTypeAt(ds,code,off)				\
	do {							\
		if ((ds)->debug)				\
			AG_WriteTypeCodeAt((ds),(code),(off));	\
	} while (0)

#define AG_TYPE_OFFSET(ds,pos) ((ds)->debug ? (pos)+sizeof(Uint32) : (pos))

__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_DATA_SOURCE_H_ */
