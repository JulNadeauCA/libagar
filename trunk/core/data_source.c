/*
 * Copyright (c) 2003-2012 Hypertriton, Inc. <http://hypertriton.com/>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Basic I/O abstraction routines.
 */

#include <agar/core/core.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static AG_Object errorMgr;

void
AG_DataSourceInitSubsystem(void)
{
	AG_ObjectInitStatic(&errorMgr, NULL);
}

void
AG_DataSourceDestroySubsystem(void)
{
	AG_ObjectDestroy(&errorMgr);
}

/* Assign an error callback routine to a data source. */
void
AG_DataSourceSetErrorFn(AG_DataSource *ds, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(&errorMgr);
	ds->errorFn = AG_SetEvent(&errorMgr, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(ds->errorFn, fmt);
	AG_ObjectUnlock(&errorMgr);
}

/* Raise a data source exception. */
void
AG_DataSourceError(AG_DataSource *ds, const char *fmt, ...)
{
	static char msg[256];
	va_list args;

	if (fmt != NULL) {
		va_start(args, fmt);
		Vsnprintf(msg, sizeof(msg), fmt, args);
		va_end(args);
	} else {
		Strlcpy(msg, AG_GetError(), sizeof(msg));
	}
	
	AG_ObjectLock(&errorMgr);
	AG_PostEventByPtr(NULL, &errorMgr, ds->errorFn, "%s", msg);
	AG_ObjectUnlock(&errorMgr);
}

/* Enable checking of debugging information. */
void
AG_DataSourceSetDebug(AG_DataSource *ds, int flag)
{
	ds->debug = flag;
}

/* Write type identifier for type safety checks. */
void
AG_WriteTypeCode(AG_DataSource *ds, Uint32 type)
{
	Uint32 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(type) :
	                                                 AG_SwapLE32(type);

	if (AG_Write(ds, &i, sizeof(i)) != 0)
		AG_DataSourceError(ds, NULL);
}

/* Write type identifier for type safety checks (offset). */
void
AG_WriteTypeCodeAt(AG_DataSource *ds, Uint32 type, off_t offs)
{
	Uint32 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(type) :
	                                                 AG_SwapLE32(type);

	if (AG_WriteAt(ds, &i, sizeof(i), offs) != 0)
		AG_DataSourceError(ds, NULL);
}

/* Write type identifier for type safety checks (error-check). */
int
AG_WriteTypeCodeE(AG_DataSource *ds, Uint32 type)
{
	Uint32 i;

	i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(type) :
	                                          AG_SwapLE32(type);

	return AG_Write(ds, &i, sizeof(i));
}

/* Check type identifier for type safety checks (error-check). */
int
AG_CheckTypeCode(AG_DataSource *ds, Uint32 type)
{
	Uint32 i;

	if (AG_Read(ds, &i, sizeof(i)) != 0) {
		AG_SetError("Reading type ID: %s", AG_GetError());
		return (-1);
	}
	i = ((ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(i) :
	                                           AG_SwapLE32(i));
	return (i == type) ? 0 : -1;
}

/*
 * No-ops
 */
static int
WriteNotSup(AG_DataSource *ds, const void *buf, size_t size, size_t *rv)
{
	AG_SetError(_("Operation not supported"));
	return (-1);
}
static int
WriteAtNotSup(AG_DataSource *ds, const void *buf, size_t size, off_t pos, size_t *rv)
{
	AG_SetError(_("Operation not supported"));
	return (-1);
}

#ifdef AG_NETWORK
static int
ReadAtNotSup(AG_DataSource *ds, void *buf, size_t size, off_t pos, size_t *rv)
{
	AG_SetError(_("Operation not supported"));
	return (-1);
}
static off_t
TellNotSup(AG_DataSource *ds)
{
	return (0);
}
static int
SeekNotSup(AG_DataSource *ds, off_t offs, enum ag_seek_mode mode)
{
	AG_SetError(_("Seek not supported by data source"));
	return (-1);
}
#endif /* AG_NETWORK */

/*
 * File operations.
 */
static int
FileRead(AG_DataSource *ds, void *buf, size_t size, size_t *rv)
{
	FILE *f = AG_FILE_SOURCE(ds)->file;

	clearerr(f);
	*rv = fread(buf, 1, size, f);
	if (*rv < size && ferror(f)) {
		AG_SetError(_("Read error"));
		return (-1);
	}
	return (0);
}
static int
FileReadAt(AG_DataSource *ds, void *buf, size_t size, off_t pos, size_t *rv)
{
	FILE *f = AG_FILE_SOURCE(ds)->file;
	long savedPos = ftell(f);

	if (fseek(f, pos, SEEK_SET) == -1) { goto fail_seek; }
	clearerr(f);
	*rv = fread(buf, 1, size, f);
	if (*rv < size && ferror(f)) {
		if (fseek(f, savedPos, SEEK_SET) == -1) { goto fail_seek; }
		AG_SetError(_("Read Error"));
		return (-1);
	}
	if (fseek(f, savedPos, SEEK_SET) == -1) { goto fail_seek; }
	return (0);
fail_seek:
	AG_SetError("fseek failed");
	return (-1);
}
static int
FileWrite(AG_DataSource *ds, const void *buf, size_t size, size_t *rv)
{
	FILE *f = AG_FILE_SOURCE(ds)->file;

	clearerr(f);
	*rv = fwrite(buf, 1, size, f);
	if (*rv < size && ferror(f)) {
		AG_SetError(_("Write error"));
		return (-1);
	}
	return (0);
}
static int
FileWriteAt(AG_DataSource *ds, const void *buf, size_t size, off_t pos,
    size_t *rv)
{
	FILE *f = AG_FILE_SOURCE(ds)->file;
	long savedPos = ftell(f);

	if (fseek(f, pos, SEEK_SET) == -1) { goto fail_seek; }
	clearerr(f);
	*rv = fwrite(buf, 1, size, f);
	if (*rv < size && ferror(f)) {
		if (fseek(f, savedPos, SEEK_SET) == -1) { goto fail_seek; }
		AG_SetError(_("Write Error"));
		return (-1);
	}
	if (fseek(f, savedPos, SEEK_SET) == -1) { goto fail_seek; }
	return (0);
fail_seek:
	AG_SetError("fseek failed");
	return (-1);
}
static off_t
FileTell(AG_DataSource *ds)
{
	return ftell(AG_FILE_SOURCE(ds)->file);
}
static int
FileSeek(AG_DataSource *ds, off_t offs, enum ag_seek_mode mode)
{
	FILE *f = AG_FILE_SOURCE(ds)->file;

	if (fseek(f, (long)offs,
	    (mode == AG_SEEK_SET) ? SEEK_SET :
	    (mode == AG_SEEK_CUR) ? SEEK_CUR :
	    SEEK_END) == -1) {
		AG_SetError("fseek failed");
		return (-1);
	}
	return (0);
}
void
AG_CloseFile(AG_DataSource *ds)
{
	AG_FileSource *fs = AG_FILE_SOURCE(ds);

	fclose(fs->file);
	Free(fs->path);
	AG_DataSourceDestroy(ds);
}

/*
 * Memory operations.
 */
static __inline__ int
CoreLimitBounds(AG_CoreSource *cs, off_t pos, size_t sizeReq, size_t *size)
{
	if (pos+sizeReq > cs->size) {
		*size = cs->size - cs->offs;
	} else {
		*size = sizeReq;
	}
	return (0);
}
static int
CoreRead(AG_DataSource *ds, void *buf, size_t sizeReq, size_t *rv)
{
	AG_CoreSource *cs = AG_CORE_SOURCE(ds);
	size_t size;

	if (CoreLimitBounds(cs, cs->offs, sizeReq, &size) == -1) {
		return (-1);
	}
	memcpy(buf, &cs->data[cs->offs], size);
	*rv = size;
	cs->offs += size;
	return (0);
}
static int
CoreReadAt(AG_DataSource *ds, void *buf, size_t sizeReq, off_t pos, size_t *rv)
{
	AG_CoreSource *cs = AG_CORE_SOURCE(ds);
	size_t size;

	if (CoreLimitBounds(cs, pos, sizeReq, &size) == -1) {
		return (-1);
	}
	memcpy(buf, &cs->data[pos], size);
	*rv = size;
	return (0);
}
static int
CoreWrite(AG_DataSource *ds, const void *buf, size_t sizeReq, size_t *rv)
{
	AG_CoreSource *cs = AG_CORE_SOURCE(ds);
	size_t size;

	if (CoreLimitBounds(cs, cs->offs, sizeReq, &size) == -1) {
		return (-1);
	}
	memcpy(&cs->data[cs->offs], buf, size);
	*rv = size;
	cs->offs += size;
	return (0);
}
static int
CoreAutoWrite(AG_DataSource *ds, const void *buf, size_t size, size_t *rv)
{
	AG_CoreSource *cs = AG_CORE_SOURCE(ds);
	Uint8 *dataNew;

	if (cs->offs+size > cs->size) {
		if ((dataNew = TryRealloc(cs->data, (cs->offs+size))) == NULL) {
			return (-1);
		}
		cs->data = dataNew;
	}
	memcpy(&cs->data[cs->offs], buf, size);
	cs->size += size;
	cs->offs += size;
	*rv = size;
	return (0);
}
static int
CoreWriteAt(AG_DataSource *ds, const void *buf, size_t sizeReq, off_t pos,
    size_t *rv)
{
	AG_CoreSource *cs = AG_CORE_SOURCE(ds);
	size_t size;

	if (CoreLimitBounds(cs, pos, sizeReq, &size) == -1) {
		return (-1);
	}
	memcpy(&cs->data[pos], buf, size);
	*rv = size;
	return (0);
}
static int
CoreAutoWriteAt(AG_DataSource *ds, const void *buf, size_t size, off_t pos,
    size_t *rv)
{
	AG_CoreSource *cs = AG_CORE_SOURCE(ds);
	Uint8 *dataNew;

	if (pos < 0) {
		AG_SetError("Bad offset");
		return (-1);
	}
	if (pos+size > cs->size) {
		if ((dataNew = TryRealloc(cs->data, (pos+size))) == NULL) {
			return (-1);
		}
		cs->data = dataNew;
		cs->size = pos+size;
	}
	memcpy(&cs->data[pos], buf, size);
	*rv = size;
	return (0);
}
static off_t
CoreTell(AG_DataSource *ds)
{
	return AG_CORE_SOURCE(ds)->offs;
}
static int
CoreSeek(AG_DataSource *ds, off_t offs, enum ag_seek_mode mode)
{
	AG_CoreSource *cs = AG_CORE_SOURCE(ds);
	off_t nOffs;

	switch (mode) {
	case AG_SEEK_SET:
		nOffs = offs;
		break;
	case AG_SEEK_CUR:
		nOffs = cs->offs + offs;
		break;
	case AG_SEEK_END:
	default:
		nOffs = cs->size - offs;
		break;
	}
	if (nOffs < 0 || nOffs >= cs->size) {
		AG_SetError("Bad offset %ld", (long)nOffs);
		return (-1);
	}
	cs->offs = nOffs;
	return (0);
}
void
AG_CloseCore(AG_DataSource *ds)
{
	AG_DataSourceDestroy(ds);
}
void
AG_CloseAutoCore(AG_DataSource *ds)
{
	Free(AG_CORE_SOURCE(ds)->data);
	AG_DataSourceDestroy(ds);
}

#ifdef AG_NETWORK
/*
 * Network socket operations
 */
static int
NetSocketRead(AG_DataSource *ds, void *buf, size_t size, size_t *rv)
{
	AG_NetSocketSource *nss = AG_NET_SOCKET_SOURCE(ds);
	return AG_NetRead(nss->sock, buf, size, rv);
}
static int
NetSocketWrite(AG_DataSource *ds, const void *buf, size_t size, size_t *rv)
{
	AG_NetSocketSource *nss = AG_NET_SOCKET_SOURCE(ds);
	return AG_NetWrite(nss->sock, buf, size, rv);
}
void
AG_CloseNetSocket(AG_DataSource *ds)
{
	AG_DataSourceDestroy(ds);
}
#endif

/* Default error handler */
static void
ErrorDefault(AG_Event *event)
{
	AG_FatalError("Data source error: %s", AG_GetError());
}

/* Initialize the data source structure. */
void
AG_DataSourceInit(AG_DataSource *ds)
{
	ds->debug = 0;
	ds->byte_order = AG_BYTEORDER_BE;
	ds->rdLast = 0;
	ds->wrLast = 0;
	ds->rdTotal = 0;
	ds->wrTotal = 0;
	ds->read = NULL;
	ds->read_at = NULL;
	ds->write = NULL;
	ds->write_at = NULL;
	ds->tell = NULL;
	ds->seek = NULL;
	ds->close = NULL;
	AG_MutexInitRecursive(&ds->lock);
	AG_DataSourceSetErrorFn(ds, ErrorDefault, "%p", ds);
}

/* Close a data source of any type. */
void
AG_CloseDataSource(AG_DataSource *ds)
{
	ds->close(ds);
}

/* Release the resources allocated by the data source structure. */
void
AG_DataSourceDestroy(AG_DataSource *ds)
{
	AG_MutexDestroy(&ds->lock);
	Free(ds);
}

/* Create a data source from a stdio file handle. */
AG_DataSource *
AG_OpenFileHandle(FILE *f)
{
	AG_FileSource *fs;

	fs = Malloc(sizeof(AG_FileSource));
	AG_DataSourceInit(&fs->ds);
	fs->path = NULL;
	fs->file = f;
	fs->ds.read = FileRead;
	fs->ds.read_at = FileReadAt;
	fs->ds.write = FileWrite;
	fs->ds.write_at = FileWriteAt;
	fs->ds.tell = FileTell;
	fs->ds.seek = FileSeek;
	fs->ds.close = AG_CloseFile;
	return (&fs->ds);
}

/* Create a data source from a specified file path. */
AG_DataSource *
AG_OpenFile(const char *path, const char *mode)
{
	FILE *f;

	if ((f = fopen(path, mode)) == NULL) {
		AG_SetError(_("Unable to open %s"), path);
		return (NULL);
	}
	return AG_OpenFileHandle(f);
}

/* Create a data source from a specified chunk of memory. */
AG_DataSource *
AG_OpenCore(void *data, size_t size)
{
	AG_CoreSource *cs;

	if ((cs = TryMalloc(sizeof(AG_CoreSource))) == NULL) {
		return (NULL);
	}
	AG_DataSourceInit(&cs->ds);
	cs->data = (Uint8 *)data;
	cs->size = size;
	cs->offs = 0;
	cs->ds.read = CoreRead;
	cs->ds.read_at = CoreReadAt;
	cs->ds.write = CoreWrite;
	cs->ds.write_at = CoreWriteAt;
	cs->ds.tell = CoreTell;
	cs->ds.seek = CoreSeek;
	cs->ds.close = AG_CloseCore;
	return (&cs->ds);
}

/* Create a data source from a specified chunk of memory (read-only). */
AG_DataSource *
AG_OpenConstCore(const void *data, size_t size)
{
	AG_ConstCoreSource *cs;

	if ((cs = TryMalloc(sizeof(AG_ConstCoreSource))) == NULL) {
		return (NULL);
	}
	AG_DataSourceInit(&cs->ds);
	cs->data = (const Uint8 *)data;
	cs->size = size;
	cs->offs = 0;
	cs->ds.read = CoreRead;
	cs->ds.read_at = CoreReadAt;
	cs->ds.write = WriteNotSup;
	cs->ds.write_at = WriteAtNotSup;
	cs->ds.tell = CoreTell;
	cs->ds.seek = CoreSeek;
	cs->ds.close = AG_CloseCore;
	return (&cs->ds);
}

/* Create a data source using dynamically-allocated memory. */
AG_DataSource *
AG_OpenAutoCore(void)
{
	AG_CoreSource *cs;

	if ((cs = TryMalloc(sizeof(AG_CoreSource))) == NULL) {
		return (NULL);
	}
	AG_DataSourceInit(&cs->ds);
	cs->data = NULL;
	cs->size = 0;
	cs->offs = 0;
	cs->ds.read = CoreRead;
	cs->ds.read_at = CoreReadAt;
	cs->ds.write = CoreAutoWrite;
	cs->ds.write_at = CoreAutoWriteAt;
	cs->ds.tell = CoreTell;
	cs->ds.seek = CoreSeek;
	cs->ds.close = AG_CloseAutoCore;
	return (&cs->ds);
}

#ifdef AG_NETWORK
/* Create a data source using a network socket. */
AG_DataSource *
AG_OpenNetSocket(AG_NetSocket *ns)
{
	AG_NetSocketSource *ss;

	if ((ss = TryMalloc(sizeof(AG_NetSocketSource))) == NULL) {
		return (NULL);
	}
	AG_DataSourceInit(&ss->ds);
	ss->sock = ns;
	ss->ds.read = NetSocketRead;
	ss->ds.read_at = ReadAtNotSup;
	ss->ds.write = NetSocketWrite;
	ss->ds.write_at = WriteAtNotSup;
	ss->ds.tell = TellNotSup;
	ss->ds.seek = SeekNotSup;
	ss->ds.close = AG_CloseNetSocket;
	return (&ss->ds);
}
#endif /* AG_NETWORK */

/* Select the byte order of the source. */
void
AG_SetByteOrder(AG_DataSource *ds, enum ag_byte_order order)
{
	AG_MutexLock(&ds->lock);
	ds->byte_order = order;
	AG_MutexUnlock(&ds->lock);
}

/* Toggle encoding/decoding of debugging data. */
void
AG_SetSourceDebug(AG_DataSource *ds, int enable)
{
	AG_MutexLock(&ds->lock);
	ds->debug = enable;
	AG_MutexUnlock(&ds->lock);
}

/* Low-level read operation. */
int
AG_Read(AG_DataSource *ds, void *ptr, size_t size)
{
	int rv;
	AG_MutexLock(&ds->lock);
	rv = ds->read(ds, ptr, size, &ds->rdLast);
	ds->rdTotal += ds->rdLast;
	if (ds->rdLast < size) {
		AG_SetError("Short read");
		rv = -1;
	}
	AG_MutexUnlock(&ds->lock);
	return (rv);
}

/* Low-level read operation (partial reads allowed). */
int
AG_ReadP(AG_DataSource *ds, void *ptr, size_t size, size_t *nRead)
{
	int rv;
	AG_MutexLock(&ds->lock);
	rv = ds->read(ds, ptr, size, &ds->rdLast);
	ds->rdTotal += ds->rdLast;
	if (nRead != NULL) { *nRead = ds->rdLast; }
	AG_MutexUnlock(&ds->lock);
	return (rv);
}

/* Low-level read-at-offset operation. */
int
AG_ReadAt(AG_DataSource *ds, void *ptr, size_t size, off_t pos)
{
	int rv;
	AG_MutexLock(&ds->lock);
	rv = ds->read_at(ds, ptr, size, pos, &ds->rdLast);
	ds->rdTotal += ds->rdLast;
	if (ds->rdLast < size) {
		AG_SetError("Short read");
		rv = -1;
	}
	AG_MutexUnlock(&ds->lock);
	return (rv);
}

/* Low-level read-at-offset operation (partial reads allowed). */
int
AG_ReadAtP(AG_DataSource *ds, void *ptr, size_t size, off_t pos, size_t *nRead)
{
	int rv;
	AG_MutexLock(&ds->lock);
	rv = ds->read_at(ds, ptr, size, pos, &ds->rdLast);
	ds->rdTotal += ds->rdLast;
	if (nRead != NULL) { *nRead = ds->rdLast; }
	AG_MutexUnlock(&ds->lock);
	return (rv);
}

/* Low-level write operation. */
int
AG_Write(AG_DataSource *ds, const void *ptr, size_t size)
{
	int rv;
	AG_MutexLock(&ds->lock);
	rv = ds->write(ds, ptr, size, &ds->wrLast);
	ds->wrTotal += ds->wrLast;
	if (ds->wrLast < size) {
		AG_SetError("Short write");
		rv = -1;
	}
	AG_MutexUnlock(&ds->lock);
	return (rv);
}

/* Low-level write operation (partial writes allowed). */
int
AG_WriteP(AG_DataSource *ds, const void *ptr, size_t size, size_t *nWrote)
{
	int rv;
	AG_MutexLock(&ds->lock);
	rv = ds->write(ds, ptr, size, &ds->wrLast);
	ds->wrTotal += ds->wrLast;
	if (nWrote != NULL) { *nWrote = ds->wrLast; }
	AG_MutexUnlock(&ds->lock);
	return (rv);
}

/* Low-level write-at-offset operation. */
int
AG_WriteAt(AG_DataSource *ds, const void *ptr, size_t size, off_t pos)
{
	int rv;
	AG_MutexLock(&ds->lock);
	rv = ds->write_at(ds, ptr, size, pos, &ds->wrLast);
	ds->wrTotal += ds->wrLast;
	if (ds->wrLast < size) {
		AG_SetError("Short write");
		rv = -1;
	}
	AG_MutexUnlock(&ds->lock);
	return (rv);
}

/* Low-level write-at-offset operation (partial writes allowed) */
int
AG_WriteAtP(AG_DataSource *ds, const void *ptr, size_t size, off_t pos, size_t *nWrote)
{
	int rv;
	AG_MutexLock(&ds->lock);
	rv = ds->write_at(ds, ptr, size, pos, &ds->wrLast);
	ds->wrTotal += ds->wrLast;
	if (nWrote != NULL) { *nWrote = ds->wrLast; }
	AG_MutexUnlock(&ds->lock);
	return (rv);
}

