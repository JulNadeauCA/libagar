/*
 * Copyright (c) 2003-2008 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>

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
	AG_PostEvent(NULL, &errorMgr, ds->errorFn->name, "%s", msg);
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

	if (AG_Write(ds, &i, sizeof(i), 1) != 0)
		AG_DataSourceError(ds, NULL);
}

/* Write type identifier for type safety checks (offset). */
void
AG_WriteTypeCodeAt(AG_DataSource *ds, Uint32 type, off_t offs)
{
	Uint32 i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(type) :
	                                                 AG_SwapLE32(type);

	if (AG_WriteAt(ds, &i, sizeof(i), 1, offs) != 0)
		AG_DataSourceError(ds, NULL);
}

/* Write type identifier for type safety checks (error-check). */
int
AG_WriteTypev(AG_DataSource *ds, Uint32 type)
{
	Uint32 i;

	if (!ds->debug) {
		return (0);
	}
	i = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(type) :
	                                          AG_SwapLE32(type);

	return AG_Write(ds, &i, sizeof(i), 1);
}

/* Check type identifier for type safety checks (error-check). */
int
AG_CheckTypev(AG_DataSource *ds, Uint32 type)
{
	Uint32 i;

	if (!ds->debug) {
		return (0);
	}
	if (AG_Read(ds, &i, sizeof(i), 1) != 0) {
		AG_SetError("Reading type ID: %s", AG_GetError());
		return (-1);
	}
	i = ((ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(i) :
	                                           AG_SwapLE32(i));
	return (i == type) ? 0 : -1;
}

/*
 * File operations.
 */
static AG_IOStatus
FileRead(AG_DataSource *ds, void *buf, size_t size, size_t nmemb, size_t *rv)
{
	FILE *f = AG_FILE_SOURCE(ds)->file;

	clearerr(f);
	*rv = fread(buf, size, nmemb, f) * size;
	if (*rv < (nmemb*size)) {
		if (ferror(f)) {
			AG_SetError(_("Read error"));
			return (AG_IO_ERROR);
		} else if (feof(f)) {
			return (AG_IO_EOF);
		}
	}
	return (AG_IO_SUCCESS);
}

static AG_IOStatus
FileReadAt(AG_DataSource *ds, void *buf, size_t size, size_t nmemb, off_t pos,
    size_t *rv)
{
	FILE *f = AG_FILE_SOURCE(ds)->file;
	long savedPos;

	savedPos = ftell(f);
	if (fseek(f, pos, SEEK_SET) == -1) {
		goto fail_seek;
	}
	clearerr(f);
	*rv = fread(buf, size, nmemb, f) * size;
	if (*rv < (nmemb*size)) {
		if (fseek(f, savedPos, SEEK_SET) == -1) {
			goto fail_seek;
		}
		if (feof(f)) {
			fseek(f, savedPos, SEEK_SET);
			return (AG_IO_EOF);
		} else if (ferror(f)) {
			AG_SetError(_("Write Error"));
			return (AG_IO_ERROR);
		}
	}
	if (fseek(f, savedPos, SEEK_SET) == -1) {
		goto fail_seek;
	}
	return (AG_IO_SUCCESS);
fail_seek:
	AG_SetError("fseek: failed");
	return (AG_IO_ERROR);
}

static AG_IOStatus
FileWrite(AG_DataSource *ds, const void *buf, size_t size, size_t nmemb,
    size_t *rv)
{
	FILE *f = AG_FILE_SOURCE(ds)->file;

	clearerr(f);
	*rv = fwrite(buf, size, nmemb, f) * size;
	if (*rv < (nmemb*size)) {
		if (ferror(f)) {
			AG_SetError(_("Write error"));
			return (AG_IO_ERROR);
		} else {
			return (AG_IO_EOF);
		}
	}
	return (AG_IO_SUCCESS);
}

static AG_IOStatus
FileWriteAt(AG_DataSource *ds, const void *buf, size_t size, size_t nmemb,
    off_t pos, size_t *rv)
{
	FILE *f = AG_FILE_SOURCE(ds)->file;
	long savedPos;

	savedPos = ftell(f);
	if (fseek(f, pos, SEEK_SET) == -1) {
		goto fail_seek;
	}
	clearerr(f);
	*rv = fwrite(buf, size, nmemb, f) * size;
	if (*rv < (nmemb*size)) {
		fseek(f, savedPos, SEEK_SET);
		if (feof(f)) {
			return (AG_IO_EOF);
		} else {
			AG_SetError(_("Write Error"));
			return (AG_IO_ERROR);
		}
	}
	if (fseek(f, savedPos, SEEK_SET) == -1) {
		goto fail_seek;
	}
	return (AG_IO_SUCCESS);
fail_seek:
	AG_SetError("fseek failed");
	return (AG_IO_ERROR);
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

/*
 * Memory operations.
 */

static AG_IOStatus
CoreRead(AG_DataSource *ds, void *buf, size_t msize, size_t nmemb, size_t *rv)
{
	AG_CoreSource *cs = AG_CORE_SOURCE(ds);
	size_t size = msize*nmemb;

	if (cs->offs+size > cs->size) {
		AG_SetError("EOF (%u+%u > %u)", (Uint)cs->offs, (Uint)size,
		    (Uint)cs->size);
		return (AG_IO_EOF);
	}
	memcpy(buf, &cs->data[cs->offs], size);
	*rv = size;
	cs->offs += size;
	return (AG_IO_SUCCESS);
}

static AG_IOStatus
CoreReadAt(AG_DataSource *ds, void *buf, size_t msize, size_t nmemb, off_t pos,
    size_t *rv)
{
	AG_CoreSource *cs = AG_CORE_SOURCE(ds);
	size_t size = msize*nmemb;

	if (pos < 0) {
		AG_SetError("Bad position");
		return (AG_IO_ERROR);
	}
	if (pos+size > cs->size) {
		AG_SetError("EOF (%u+%u > %u)", (Uint)pos, (Uint)size,
		    (Uint)cs->size);
		return (AG_IO_EOF);
	}
	memcpy(buf, &cs->data[pos], size);
	*rv = size;
	return (AG_IO_SUCCESS);
}

static AG_IOStatus
CoreWrite(AG_DataSource *ds, const void *buf, size_t msize, size_t nmemb,
    size_t *rv)
{
	AG_CoreSource *cs = AG_CORE_SOURCE(ds);
	size_t size = msize*nmemb;

	if (cs->offs+size > cs->size) {
		AG_SetError("EOF");
		return (AG_IO_EOF);
	}
	memcpy(&cs->data[cs->offs], buf, size);
	*rv = size;
	cs->offs += size;
	return (AG_IO_SUCCESS);
}

static AG_IOStatus
CoreAutoWrite(AG_DataSource *ds, const void *buf, size_t msize, size_t nmemb,
    size_t *rv)
{
	AG_CoreSource *cs = AG_CORE_SOURCE(ds);
	size_t size = msize*nmemb;

	cs->data = Realloc(cs->data, (cs->size+size));
	memcpy(&cs->data[cs->offs], buf, size);
	cs->size += size;
	cs->offs += size;
	*rv = size;
	return (AG_IO_SUCCESS);
}

static AG_IOStatus
CoreWriteAt(AG_DataSource *ds, const void *buf, size_t msize, size_t nmemb,
    off_t pos, size_t *rv)
{
	AG_CoreSource *cs = AG_CORE_SOURCE(ds);
	size_t size = msize*nmemb;

	if (pos < 0) {
		AG_SetError("Bad position");
		return (AG_IO_ERROR);
	}
	if (pos+size > cs->size) {
		AG_SetError("EOF");
		return (AG_IO_EOF);
	}
	memcpy(&cs->data[pos], buf, size);
	*rv = size;
	return (AG_IO_SUCCESS);
}

static AG_IOStatus
CoreAutoWriteAt(AG_DataSource *ds, const void *buf, size_t msize, size_t nmemb,
    off_t pos, size_t *rv)
{
	AG_CoreSource *cs = AG_CORE_SOURCE(ds);
	size_t size = msize*nmemb;

	if (pos < 0) {
		AG_SetError("Bad position");
		return (AG_IO_ERROR);
	}
	if (pos+size > cs->size) {
		cs->data = Realloc(cs->data, (pos+size));
		cs->size = pos+size;
	}
	memcpy(&cs->data[pos], buf, size);
	*rv = size;
	return (AG_IO_SUCCESS);
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
		AG_SetError("Unable to open %s", path);
		return (NULL);
	}
	return AG_OpenFileHandle(f);
}

/* Create a data source from a specified chunk of memory. */
AG_DataSource *
AG_OpenCore(void *data, size_t size)
{
	AG_CoreSource *cs;

	cs = Malloc(sizeof(AG_CoreSource));
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

	cs = Malloc(sizeof(AG_ConstCoreSource));
	AG_DataSourceInit(&cs->ds);
	cs->data = (const Uint8 *)data;
	cs->size = size;
	cs->offs = 0;
	cs->ds.read = CoreRead;
	cs->ds.read_at = CoreReadAt;
	cs->ds.write = NULL;
	cs->ds.write_at = NULL;
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

	cs = Malloc(sizeof(AG_CoreSource));
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

void
AG_SetByteOrder(AG_DataSource *ds, enum ag_byte_order order)
{
	AG_MutexLock(&ds->lock);
	ds->byte_order = order;
	AG_MutexUnlock(&ds->lock);
}

void
AG_SetSourceDebug(AG_DataSource *ds, int enable)
{
	AG_MutexLock(&ds->lock);
	ds->debug = enable;
	AG_MutexUnlock(&ds->lock);
}

void
AG_CloseFile(AG_DataSource *ds)
{
	AG_FileSource *fs = AG_FILE_SOURCE(ds);

	fclose(fs->file);
	Free(fs->path);
	AG_DataSourceDestroy(ds);
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

AG_IOStatus
AG_Read(AG_DataSource *ds, void *ptr, size_t size, size_t nmemb)
{
	AG_IOStatus rv;
	
	AG_MutexLock(&ds->lock);
	if (ds->read == NULL) {
		AG_SetError(_("Illegal operation"));
		return (AG_IO_ERROR);
	}
	rv = ds->read(ds, ptr, size, nmemb, &ds->rdLast);
	ds->rdTotal += ds->rdLast;
	AG_MutexUnlock(&ds->lock);
	return (rv);
}

AG_IOStatus
AG_ReadAt(AG_DataSource *ds, void *ptr, size_t size, size_t nmemb, off_t pos)
{
	AG_IOStatus rv;
	
	AG_MutexLock(&ds->lock);
	if (ds->read_at == NULL) {
		AG_SetError(_("Illegal operation"));
		return (AG_IO_ERROR);
	}
	rv = ds->read_at(ds, ptr, size, nmemb, pos, &ds->rdLast);
	ds->rdTotal += ds->rdLast;
	AG_MutexUnlock(&ds->lock);
	return (rv);
}

AG_IOStatus
AG_Write(AG_DataSource *ds, const void *ptr, size_t size, size_t nmemb)
{
	AG_IOStatus rv;
	
	AG_MutexLock(&ds->lock);
	if (ds->write == NULL) {
		AG_SetError(_("Illegal operation"));
		return (AG_IO_ERROR);
	}
	rv = ds->write(ds, ptr, size, nmemb, &ds->wrLast);
	ds->wrTotal += ds->wrLast;
	AG_MutexUnlock(&ds->lock);
	return (rv);
}

AG_IOStatus
AG_WriteAt(AG_DataSource *ds, const void *ptr, size_t size, size_t nmemb,
    off_t pos)
{
	AG_IOStatus rv;
	
	AG_MutexLock(&ds->lock);
	if (ds->write_at == NULL) {
		AG_SetError(_("Illegal operation"));
		return (AG_IO_ERROR);
	}
	rv = ds->write_at(ds, ptr, size, nmemb, pos, &ds->wrLast);
	ds->wrTotal += ds->wrLast;
	AG_MutexUnlock(&ds->lock);
	return (rv);
}

off_t
AG_Tell(AG_DataSource *ds)
{
	off_t pos;
	
	AG_MutexLock(&ds->lock);
	pos = (ds->tell != NULL) ? ds->tell(ds) : 0;
	AG_MutexUnlock(&ds->lock);
	return (pos);
}

int
AG_Seek(AG_DataSource *ds, off_t pos, enum ag_seek_mode mode)
{
	int rv;

	if (ds->seek == NULL) {
		AG_SetError(_("Illegal operation"));
		return (-1);
	}
	AG_MutexLock(&ds->lock);
	rv = ds->seek(ds, pos, mode);
	AG_MutexUnlock(&ds->lock);
	return (rv);
}
