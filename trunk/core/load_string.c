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
 * Serialization functions for strings.
 */

#include <agar/core/core.h>

/* Allocate and read a length-encoded string. */
char *
AG_ReadStringLen(AG_DataSource *ds, size_t maxlen)
{
	Uint32 len;
	char *s;

	AG_LockDataSource(ds);

	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_STRING) == -1) {
		goto fail;
	}
	if (AG_ReadUint32v(ds, &len) == -1) {
		AG_SetError("String length: %s", AG_GetError());
		goto fail;
	}
	if (len > (Uint32)maxlen) {
		AG_SetError("String (%luB): Exceeds %luB limit", (Ulong)len,
		    (Ulong)maxlen);
		goto fail;
	}
	if ((s = TryMalloc((size_t)len+1)) == NULL) {
		goto fail;
	}
	if (len > 0) {
	  	if (AG_Read(ds, s, len) != 0) {
			AG_SetError("String (%luB): %s", (Ulong)len,
			    AG_GetError());
			Free(s);
			goto fail;
		}
	}
	s[len] = '\0';

	AG_UnlockDataSource(ds);
	return (s);
fail:
	AG_DataSourceError(ds, NULL);
	AG_UnlockDataSource(ds);
	return (NULL);
}

/*
 * Read a length-encoded string (no exceptions) and return its contents
 * in newly-allocated memory.
 *
 * If s is non-NULL, *s is taken to be an existing, valid buffer which will
 * be reallocated to fit the new string size.
 */
int
AG_ReadStringLenv(AG_DataSource *ds, size_t maxlen, char **s)
{
	Uint32 len;
	char *sp;

	AG_LockDataSource(ds);

	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_STRING) == -1) {
		goto fail;
	}
	if (AG_ReadUint32v(ds, &len) == -1) {
		AG_SetError("String length: %s", AG_GetError());
		goto fail;
	}
	if (len > (Uint32)maxlen) {
		AG_SetError("String (%luB): Exceeds %luB limit", (Ulong)len,
		    (Ulong)maxlen);
		goto fail;
	}
	if ((sp = TryRealloc(*s, (size_t)len+1)) == NULL) {
		goto fail;
	}
	*s = sp;
	if (len > 0) {
	  	if (AG_Read(ds, sp, len) != 0) {
			AG_SetError("String (%luB): %s", (Ulong)len,
			    AG_GetError());
			sp[0] = '\0';
			goto fail;
		}
		sp[len] = '\0';
	}

	AG_UnlockDataSource(ds);
	return (0);
fail:
	AG_DataSourceError(ds, NULL);
	return (-1);
}

/*
 * Allocate and read a length-encoded string with NUL-termination.
 * Type checking is never done; this function is useful when reading
 * non-Agar generated datasets.
 */
char *
AG_ReadNulStringLen(AG_DataSource *ds, size_t maxlen)
{
	Uint32 len;
	char *s;

	AG_LockDataSource(ds);

	if (AG_ReadUint32v(ds, &len) == -1) {
		AG_SetError("String length: %s", AG_GetError());
		goto fail;
	}
	if (len > (Uint32)maxlen) {
		AG_SetError("String (%luB): Exceeds %luB limit", (Ulong)len,
		    (Ulong)maxlen);
		goto fail;
	}
	if ((s = TryMalloc((size_t)len)) == NULL) {
		goto fail;
	}
	if (len > 0) {
		if (AG_Read(ds, s, (size_t)len) != 0) {
			AG_SetError("String (%luB): %s", (Ulong)len,
			    AG_GetError());
			Free(s);
			goto fail;
		}
	}
	AG_UnlockDataSource(ds);
	return (s);
fail:
	AG_DataSourceError(ds, NULL);
	AG_UnlockDataSource(ds);
	return (NULL);
}

/* Write a length-encoded string. */
void
AG_WriteString(AG_DataSource *ds, const char *s)
{
	size_t len;

	if (ds->debug)
		AG_WriteTypeCode(ds, AG_SOURCE_STRING);

	if (s == NULL || s[0] == '\0') {
		AG_WriteUint32(ds, 0);
	} else {
		len = strlen(s);
		AG_WriteUint32(ds, (Uint32)len);
		if (AG_Write(ds, s, len) != 0)
			AG_DataSourceError(ds, NULL);
	}
}

/* Write a length-encoded string (no exceptions). */
int
AG_WriteStringv(AG_DataSource *ds, const char *s)
{
	Uint32 len = (s != NULL && s[0] != '\0') ? strlen(s) : 0;
	
	if (ds->debug &&
	    AG_WriteTypeCodeE(ds, AG_SOURCE_STRING) == -1) {
		return (-1);
	}
	if (AG_WriteUint32v(ds, &len) == -1) {
		return (-1);
	}
	return (len > 0) ? AG_Write(ds, s, len) : 0;
}

/*
 * Copy at most dst_size bytes from a length-encoded string to a fixed-size
 * buffer, returning the number of bytes that would have been copied were
 * dst_size unlimited. The function NUL-terminates the string.
 */
size_t
AG_CopyString(char *dst, AG_DataSource *ds, size_t dst_size)
{
	size_t rv;
	Uint32 len;

	AG_LockDataSource(ds);
	
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_STRING) == -1) {
		goto fail;
	}
	if (AG_ReadUint32v(ds, &len) == -1) {
		AG_SetError("String length: %s", AG_GetError());
		goto fail;
	}
	if (len > (Uint32)(dst_size-1)) {
#ifdef AG_DEBUG
		Verbose("0x%x: %luB string truncated to fit %luB buffer\n",
		    (Uint)AG_Tell(ds), (Ulong)len, (Ulong)dst_size);
#endif
		rv = (size_t)len+1;		/* Save the intended length */
		len = dst_size-1;
	} else {
		rv = (size_t)len;
	}
	if (len == 0) {
		dst[0] = '\0';
	} else {
		if (AG_Read(ds, dst, (size_t)len) != 0) {
			goto fail;
		}
		dst[ds->rdLast] = '\0';
	}
	AG_UnlockDataSource(ds);
	return (rv);				/* Count does not include NUL */
fail:
	AG_DataSourceError(ds, NULL);
	AG_UnlockDataSource(ds);
	dst[0] = '\0';
	return (0);
}

/* Skip over a length-encoded string. */
void
AG_SkipString(AG_DataSource *ds)
{
	Uint32 len;

	AG_LockDataSource(ds);
	
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_STRING) == -1) {
		goto fail;
	}
	if (AG_ReadUint32v(ds, &len) == -1) {
		AG_SetError("String length: %s", AG_GetError());
		goto fail;
	}
	if (AG_Seek(ds, (size_t)len, AG_SEEK_CUR) == -1)
		goto fail;

	AG_UnlockDataSource(ds);
	return;
fail:
	AG_DataSourceError(ds, NULL);
	AG_UnlockDataSource(ds);
}

/*
 * Copy at most dst_size bytes from a length-encoded, NUL-terminated string
 * to a fixed-size buffer, returning the number of bytes that would have been
 * copied were dst_size unlimited. Type checking is never done.
 */
size_t
AG_CopyNulString(char *dst, AG_DataSource *ds, size_t dst_size)
{
	size_t rv;
	Uint32 len;

	AG_LockDataSource(ds);

	if (AG_ReadUint32v(ds, &len) == -1) {
		AG_SetError("String length: %s", AG_GetError());
		goto fail;
	}
	if (len > (Uint32)dst_size) {
#ifdef AG_DEBUG
		Verbose("0x%x: %luB string truncated to fit %luB buffer\n",
		    (Uint)AG_Tell(ds), (Ulong)len, (Ulong)dst_size);
#endif
		rv = (size_t)len;		/* Save the intended length */
		len = dst_size;
	} else {
		rv = (size_t)len;
	}
	if (AG_Read(ds, dst, (size_t)len) != 0)
		goto fail;
	
	AG_UnlockDataSource(ds);
	return (rv-1);			/* Count does not include NUL */
fail:
	AG_DataSourceError(ds, NULL);
	dst[0] = '\0';
	AG_UnlockDataSource(ds);
	return (0);
}
