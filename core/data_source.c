/*
 * Copyright (c) 2003-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/config/ag_serialization.h>
#ifdef AG_SERIALIZATION

#include <agar/core/core.h>

#include <agar/config/have_fdclose.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static AG_Object errorMgr;

void
AG_DataSourceInitSubsystem(void)
{
	AG_ObjectInit(&errorMgr, NULL);
	errorMgr.flags |= AG_OBJECT_STATIC;
}

void
AG_DataSourceDestroySubsystem(void)
{
	AG_ObjectDestroy(&errorMgr);
}

/* Assign an error callback routine to a data source. */
void
AG_DataSourceSetErrorFn(AG_DataSource *ds, AG_EventFn fn,
    const char *_Nullable fmt, ...)
{
	AG_ObjectLock(&errorMgr);
	ds->errorFn = AG_SetEvent(&errorMgr, NULL, fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(ds->errorFn, fmt, ap);
		va_end(ap);
	}
	AG_ObjectUnlock(&errorMgr);
}

/* Raise a data source exception. */
void
AG_DataSourceError(AG_DataSource *ds, const char *fmt, ...)
{
	static char msg[64];
	va_list args;

	if (fmt != NULL) {
		va_start(args, fmt);
		Vsnprintf(msg, sizeof(msg), fmt, args);
		va_end(args);
	} else {
		Strlcpy(msg, AG_GetError(), sizeof(msg));
	}
	
	AG_PostEventByPtr(&errorMgr, ds->errorFn, "%s", msg);
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
AG_WriteTypeCodeAt(AG_DataSource *ds, Uint32 type, AG_Offset offs)
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

/* Reallocate the buffer of a dynamically-allocated memory source. */
int
AG_DataSourceRealloc(void *obj, AG_Size size)
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

/* Return current position in the data stream. */
AG_Offset
AG_Tell(AG_DataSource *ds)
{
	AG_Offset pos;

	AG_MutexLock(&ds->lock);
	pos = (ds->tell != NULL) ? ds->tell(ds) : 0;
	AG_MutexUnlock(&ds->lock);
	return (pos);
}

/* Seek to position. */
int
AG_Seek(AG_DataSource *ds, AG_Offset pos, enum ag_seek_mode mode)
{
	int rv;

	AG_MutexLock(&ds->lock);
	rv = ds->seek(ds, pos, mode);
	AG_MutexUnlock(&ds->lock);
	return (rv);
}

/* Close a datasource of any type. */
void
AG_CloseDataSource(AG_DataSource *ds)
{
	ds->close(ds);
}

/* Free all resources allocated by a data source. */
void
AG_DataSourceDestroy(AG_DataSource *ds)
{
	AG_MutexDestroy(&ds->lock);
	AG_Free(ds);
}

/*
 * No-ops
 */
static int
WriteNotSup(AG_DataSource *_Nonnull ds, const void *_Nonnull buf,
    AG_Size size, AG_Size *_Nonnull rv)
{
	AG_SetErrorS(_("Operation not supported"));
	return (-1);
}
static int
WriteAtNotSup(AG_DataSource *_Nonnull ds, const void *_Nonnull buf,
    AG_Size size, AG_Offset pos, AG_Size *_Nonnull rv)
{
	AG_SetErrorS(_("Operation not supported"));
	return (-1);
}

#ifdef AG_NETWORK
static int
ReadAtNotSup(AG_DataSource *_Nonnull ds, void *_Nonnull buf,
    AG_Size size, AG_Offset pos, AG_Size *_Nonnull rv)
{
	AG_SetErrorS(_("Operation not supported"));
	return (-1);
}
static AG_Offset
TellNotSup(AG_DataSource *_Nonnull ds)
{
	return (0);
}
static int
SeekNotSup(AG_DataSource *_Nonnull ds, AG_Offset offs, enum ag_seek_mode mode)
{
	AG_SetErrorS(_("Seek not supported by data source"));
	return (-1);
}
#endif /* AG_NETWORK */

/*
 * File operations.
 */
static int
FileRead(AG_DataSource *_Nonnull ds, void *_Nonnull buf, AG_Size size,
    AG_Size *_Nonnull rv)
{
	FILE *f = AG_FILE_SOURCE(ds)->file;

	clearerr(f);
	*rv = fread(buf, 1, size, f);
	if (*rv < size && ferror(f)) {
		AG_SetErrorS(_("Read error"));
		return (-1);
	}
	return (0);
}
static int
FileReadAt(AG_DataSource *_Nonnull ds, void *_Nonnull buf, AG_Size size,
    AG_Offset pos, AG_Size *_Nonnull rv)
{
	FILE *f = AG_FILE_SOURCE(ds)->file;
	long savedPos = ftell(f);

	if (fseek(f, pos, SEEK_SET) == -1) { goto fail_seek; }
	clearerr(f);
	*rv = fread(buf, 1, size, f);
	if (*rv < size && ferror(f)) {
		if (fseek(f, savedPos, SEEK_SET) == -1) { goto fail_seek; }
		AG_SetErrorS(_("Read Error"));
		return (-1);
	}
	if (fseek(f, savedPos, SEEK_SET) == -1) { goto fail_seek; }
	return (0);
fail_seek:
	AG_SetErrorS("fseek failed");
	return (-1);
}
static int
FileWrite(AG_DataSource *_Nonnull ds, const void *_Nonnull buf, AG_Size size,
    AG_Size *_Nonnull rv)
{
	FILE *f = AG_FILE_SOURCE(ds)->file;

	clearerr(f);
	*rv = fwrite(buf, 1, size, f);
	if (*rv < size && ferror(f)) {
		AG_SetErrorS(_("Write error"));
		return (-1);
	}
	return (0);
}
static int
FileWriteAt(AG_DataSource *_Nonnull ds, const void *_Nonnull buf, AG_Size size,
    AG_Offset pos, AG_Size *_Nonnull rv)
{
	FILE *f = AG_FILE_SOURCE(ds)->file;
	long savedPos = ftell(f);

	if (fseek(f, pos, SEEK_SET) == -1) { goto fail_seek; }
	clearerr(f);
	*rv = fwrite(buf, 1, size, f);
	if (*rv < size && ferror(f)) {
		if (fseek(f, savedPos, SEEK_SET) == -1) { goto fail_seek; }
		AG_SetErrorS(_("Write Error"));
		return (-1);
	}
	if (fseek(f, savedPos, SEEK_SET) == -1) { goto fail_seek; }
	return (0);
fail_seek:
	AG_SetErrorS("fseek failed");
	return (-1);
}
static AG_Offset
FileTell(AG_DataSource *_Nonnull ds)
{
	return ftell(AG_FILE_SOURCE(ds)->file);
}
static int
FileSeek(AG_DataSource *_Nonnull ds, AG_Offset offs, enum ag_seek_mode mode)
{
	FILE *f = AG_FILE_SOURCE(ds)->file;

	if (fseek(f, (long)offs,
	    (mode == AG_SEEK_SET) ? SEEK_SET :
	    (mode == AG_SEEK_CUR) ? SEEK_CUR :
	    SEEK_END) == -1) {
		AG_SetErrorS("fseek failed");
		return (-1);
	}
	return (0);
}

/*
 * Memory operations. Core operates on fixed-length memory. AutoCore operates
 * on dynamically-reallocated memory.
 */
static int
CoreRead(AG_DataSource *_Nonnull ds, void *_Nonnull buf, AG_Size len,
    AG_Size *_Nonnull rv)
{
	AG_CoreSource *cs = AG_CORE_SOURCE(ds);

	if (cs->offs+len > cs->size) {
		AG_SetError("Out of bounds (%lu+%lu > %lu)", (Ulong)cs->offs,
		    (Ulong)len, (Ulong)cs->size);
		return (-1);
	}
	memcpy(buf, &cs->data[cs->offs], len);
	*rv = len;
	cs->offs += len;
	return (0);
}
static int
CoreReadAt(AG_DataSource *_Nonnull ds, void *_Nonnull buf, AG_Size len,
    AG_Offset pos, AG_Size *_Nonnull rv)
{
	AG_CoreSource *cs = AG_CORE_SOURCE(ds);

	if (pos+len > cs->size) {
		AG_SetError("Out of bounds (@%lu+%lu > %lu)", (Ulong)pos, (Ulong)len,
		    (Ulong)cs->size);
		return (-1);
	}
	memcpy(buf, &cs->data[pos], len);
	*rv = len;
	return (0);
}
static int
CoreWrite(AG_DataSource *_Nonnull ds, const void *_Nonnull buf, AG_Size len,
    AG_Size *_Nonnull rv)
{
	AG_CoreSource *cs = AG_CORE_SOURCE(ds);

	if (cs->offs+len > cs->size) {
		AG_SetError("Out of bounds (>%lu+%lu > %lu)", (Ulong)cs->offs,
		    (Ulong)len, (Ulong)cs->size);
		return (-1);
	}
	memcpy(&cs->data[cs->offs], buf, len);
	*rv = len;
	cs->offs += len;
	return (0);
}
static int
CoreAutoWrite(AG_DataSource *_Nonnull ds, const void *_Nonnull buf, AG_Size size,
    AG_Size *_Nonnull rv)
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
CoreWriteAt(AG_DataSource *_Nonnull ds, const void *_Nonnull buf, AG_Size len,
    AG_Offset pos, AG_Size *_Nonnull rv)
{
	AG_CoreSource *cs = AG_CORE_SOURCE(ds);

	if (pos+len > cs->size) {
		AG_SetError("Out of bounds (>@%lu+%lu > %lu)", (Ulong)pos, (Ulong)len,
		    (Ulong)cs->size);
		return (-1);
	}
	memcpy(&cs->data[pos], buf, len);
	*rv = len;
	return (0);
}
static int
CoreAutoWriteAt(AG_DataSource *_Nonnull ds, const void *_Nonnull buf, AG_Size len,
    AG_Offset pos, AG_Size *_Nonnull rv)
{
	AG_CoreSource *cs = AG_CORE_SOURCE(ds);
	Uint8 *dataNew;

	if (pos < 0) {
		AG_SetErrorS("Bad offset");
		return (-1);
	}
	if (pos+len > cs->size) {
		if ((dataNew = TryRealloc(cs->data, pos+len)) == NULL) {
			return (-1);
		}
		cs->data = dataNew;
		cs->size = pos+len;
	}
	memcpy(&cs->data[pos], buf, len);
	*rv = len;
	return (0);
}
static AG_Offset
CoreTell(AG_DataSource *_Nonnull ds)
{
	return AG_CORE_SOURCE(ds)->offs;
}
static int
CoreSeek(AG_DataSource *_Nonnull ds, AG_Offset offs, enum ag_seek_mode mode)
{
	AG_CoreSource *cs = AG_CORE_SOURCE(ds);
	AG_Offset nOffs;

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
AG_CloseCore(AG_DataSource *_Nonnull ds)
{
	AG_DataSourceDestroy(ds);
}
void
AG_CloseAutoCore(AG_DataSource *_Nonnull ds)
{
	Free(AG_CORE_SOURCE(ds)->data);
	AG_DataSourceDestroy(ds);
}

#ifdef AG_NETWORK
/*
 * Network socket operations
 */
static int
NetSocketRead(AG_DataSource *_Nonnull ds, void *_Nonnull buf, AG_Size size,
    AG_Size *_Nonnull rv)
{
	AG_NetSocketSource *nss = AG_NET_SOCKET_SOURCE(ds);

	return AG_NetRead(nss->sock, buf, size, rv);
}
static int
NetSocketWrite(AG_DataSource *_Nonnull ds, const void *_Nonnull buf, AG_Size size,
    AG_Size *_Nonnull rv)
{
	AG_NetSocketSource *nss = AG_NET_SOCKET_SOURCE(ds);

	return AG_NetWrite(nss->sock, buf, size, rv);
}
void
AG_CloseNetSocket(AG_DataSource *_Nonnull ds)
{
	AG_DataSourceDestroy(ds);
}
#endif /* AG_NETWORK */

/* Default error handler */
static void
ErrorDefault(AG_Event *event)
{
	AG_SetError("AG_DataSource: %s", AG_GetError());
	AG_FatalError(NULL);
}

/* Initialize the data source structure. */
void
AG_DataSourceInit(AG_DataSource *_Nonnull ds)
{
	AG_MutexInitRecursive(&ds->lock);
	ds->errorFn = NULL;
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
	AG_DataSourceSetErrorFn(ds, ErrorDefault, "%p", ds);
}

AG_DataSource *
AG_OpenFileHandle(void *_Nonnull pf)
{
	FILE *f = pf;
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
	fs->ds.close = AG_CloseFileHandle;
	return (&fs->ds);
}

/* Close file handle created by AG_OpenFileHandle() */
void
AG_CloseFileHandle(AG_DataSource *_Nonnull ds)
{
	AG_FileSource *fs = AG_FILE_SOURCE(ds);

#ifdef HAVE_FDCLOSE
	fdclose(fs->file, NULL);
#else
	fflush(fs->file);
#endif
	AG_DataSourceDestroy(ds);
}

/* Close file handle created by AG_OpenFile() */
void
AG_CloseFile(AG_DataSource *_Nonnull ds)
{
	AG_FileSource *fs = AG_FILE_SOURCE(ds);

	fclose(fs->file);
	AG_Free(fs->path);
	AG_DataSourceDestroy(ds);
}

/* Create a data source from a specified file path. */
AG_DataSource *
AG_OpenFile(const char *_Nonnull path, const char *_Nonnull mode)
{
	AG_FileSource *fs;
	FILE *f;

	if ((f = fopen(path, mode)) == NULL) {
		AG_SetError(_("Unable to open %s"), path);
		return (NULL);
	}
	fs = Malloc(sizeof(AG_FileSource));
	AG_DataSourceInit(&fs->ds);
	fs->path = TryStrdup(path);
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

/* Create a data source from a specified chunk of memory. */
AG_DataSource *
AG_OpenCore(void *_Nonnull data, AG_Size size)
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
AG_OpenConstCore(const void *_Nonnull data, AG_Size size)
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
AG_OpenNetSocket(AG_NetSocket *_Nonnull ns)
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

/*
 * Select the byte order of a data source. Return the previous setting.
 *
 * The byte order can be switched while a data stream is being read or
 * written. Library routines must be careful to restore previous byte
 * order before returning.
 */
AG_ByteOrder
AG_SetByteOrder(AG_DataSource *_Nonnull ds, AG_ByteOrder order)
{
	AG_ByteOrder orderPrev;

	orderPrev = ds->byte_order;
	ds->byte_order = order;
	return (orderPrev);
}

/* Toggle encoding/decoding of debugging data. Return previous setting. */
int
AG_SetSourceDebug(AG_DataSource *_Nonnull ds, int enable)
{
	int debugPrev;

	debugPrev = ds->debug;
	ds->debug = enable;
	return (debugPrev);
}

/* Standard read operation (read complete size or fail). */
int
AG_Read(AG_DataSource *_Nonnull ds, void *_Nonnull ptr, AG_Size size)
{
	int rv;

	AG_MutexLock(&ds->lock);
	rv = ds->read(ds, ptr, size, &ds->rdLast);
	ds->rdTotal += ds->rdLast;
	if (ds->rdLast < size) {
		AG_SetErrorS("Short read");
		rv = -1;
	}
	AG_MutexUnlock(&ds->lock);
	return (rv);
}

/* Standard read operation (partial reads allowed). */
int
AG_ReadP(AG_DataSource *_Nonnull ds, void *_Nonnull ptr, AG_Size size,
    AG_Size *nRead)
{
	int rv;

	AG_MutexLock(&ds->lock);
	rv = ds->read(ds, ptr, size, &ds->rdLast);
	ds->rdTotal += ds->rdLast;
	if (nRead != NULL) { *nRead = ds->rdLast; }
	AG_MutexUnlock(&ds->lock);
	return (rv);
}

/* Read data from a particular offset (read complete or fail). */
int
AG_ReadAt(AG_DataSource *_Nonnull ds, void *_Nonnull ptr, AG_Size size, AG_Offset pos)
{
	int rv;

	AG_MutexLock(&ds->lock);
	rv = ds->read_at(ds, ptr, size, pos, &ds->rdLast);
	ds->rdTotal += ds->rdLast;
	if (ds->rdLast < size) {
		AG_SetErrorS("Short read");
		rv = -1;
	}
	AG_MutexUnlock(&ds->lock);
	return (rv);
}

/* Read data from a particular offset (partial reads allowed). */
int
AG_ReadAtP(AG_DataSource *_Nonnull ds, void *_Nonnull ptr, AG_Size size, AG_Offset pos,
    AG_Size *nRead)
{
	int rv;

	AG_MutexLock(&ds->lock);
	rv = ds->read_at(ds, ptr, size, pos, &ds->rdLast);
	ds->rdTotal += ds->rdLast;
	if (nRead != NULL) { *nRead = ds->rdLast; }
	AG_MutexUnlock(&ds->lock);
	return (rv);
}

/* Standard write operation (write complete or fail). */
int
AG_Write(AG_DataSource *_Nonnull ds, const void *_Nonnull ptr, AG_Size size)
{
	int rv;

	AG_MutexLock(&ds->lock);
	rv = ds->write(ds, ptr, size, &ds->wrLast);
	ds->wrTotal += ds->wrLast;
	if (ds->wrLast < size) {
		AG_SetErrorS("Short write");
		rv = -1;
	}
	AG_MutexUnlock(&ds->lock);
	return (rv);
}

/* Standard write operation (partial writes allowed). */
int
AG_WriteP(AG_DataSource *_Nonnull ds, const void *_Nonnull ptr, AG_Size size,
    AG_Size *nWrote)
{
	int rv;

	AG_MutexLock(&ds->lock);
	rv = ds->write(ds, ptr, size, &ds->wrLast);
	ds->wrTotal += ds->wrLast;
	if (nWrote != NULL) { *nWrote = ds->wrLast; }
	AG_MutexUnlock(&ds->lock);
	return (rv);
}

/* Write data at particular offset (write complete or fail). */
int
AG_WriteAt(AG_DataSource *_Nonnull ds, const void *_Nonnull ptr,
    AG_Size size, AG_Offset pos)
{
	int rv;

	AG_MutexLock(&ds->lock);
	rv = ds->write_at(ds, ptr, size, pos, &ds->wrLast);
	ds->wrTotal += ds->wrLast;
	if (ds->wrLast < size) {
		AG_SetErrorS("Short write");
		rv = -1;
	}
	AG_MutexUnlock(&ds->lock);
	return (rv);
}

/* Write data at particular offset (partial writes allowed). */
int
AG_WriteAtP(AG_DataSource *_Nonnull ds, const void *_Nonnull ptr, AG_Size size,
    AG_Offset pos, AG_Size *nWrote)
{
	int rv;

	AG_MutexLock(&ds->lock);
	rv = ds->write_at(ds, ptr, size, pos, &ds->wrLast);
	ds->wrTotal += ds->wrLast;
	if (nWrote != NULL) { *nWrote = ds->wrLast; }
	AG_MutexUnlock(&ds->lock);
	return (rv);
}

#endif /* AG_SERIALIZATION */
