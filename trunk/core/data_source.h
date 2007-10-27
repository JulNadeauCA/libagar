/*	Public domain	*/

#ifndef _AGAR_LOADER_NETBUF_H_
#define _AGAR_LOADER_NETBUF_H_
#include "begin_code.h"

enum ag_byte_order {
	AG_BIG_ENDIAN,
	AG_LITTLE_ENDIAN
};
enum ag_seek_mode {
	AG_SEEK_SET,
	AG_SEEK_CUR,
	AG_SEEK_END
};
typedef enum ag_io_status {
	AG_IO_SUCCESS = 0,
	AG_IO_EOF = 1,
	AG_IO_ERROR = 2,
	AG_IO_UNAVAIL = 3
} AG_IOStatus;

typedef struct ag_data_source {
	AG_Mutex lock;				/* Lock on all operations */
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
#define AG_CONST_CORE_SOURCE(ds) ((AG_ConstCoreSource *)(ds))

__BEGIN_DECLS
void           AG_DataSourceInit(AG_DataSource *);
void           AG_DataSourceDestroy(AG_DataSource *);
void	       AG_SetByteOrder(AG_DataSource *, enum ag_byte_order);
void           AG_CloseDataSource(AG_DataSource *);

AG_DataSource *AG_OpenFile(const char *, const char *);
AG_DataSource *AG_OpenFileHandle(FILE *);
AG_DataSource *AG_OpenCore(void *, size_t);
AG_DataSource *AG_OpenConstCore(const void *, size_t);
void           AG_CloseFile(AG_DataSource *);
#define        AG_CloseFileHandle(ds) AG_CloseFile(ds)
void           AG_CloseCore(AG_DataSource *);
#define        AG_CloseConstCore(ds) AG_CloseCore(ds)

AG_IOStatus    AG_Read(AG_DataSource *, void *, size_t, size_t);
AG_IOStatus    AG_ReadAt(AG_DataSource *, void *, size_t, size_t, off_t);
AG_IOStatus    AG_Write(AG_DataSource *, const void *, size_t, size_t);
AG_IOStatus    AG_WriteAt(AG_DataSource *, const void *, size_t, size_t, off_t);
off_t          AG_Tell(AG_DataSource *);
int            AG_Seek(AG_DataSource *, off_t, enum ag_seek_mode);

#define        AG_LockDataSource(ds) AG_MutexLock(&(ds)->lock);
#define        AG_UnlockDataSource(ds) AG_MutexUnlock(&(ds)->lock);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_LOADER_NETBUF_H_ */
