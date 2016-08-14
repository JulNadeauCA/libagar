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
	AG_UnlockDataSource(ds);
	AG_DataSourceError(ds, NULL);
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
	AG_UnlockDataSource(ds);
	AG_DataSourceError(ds, NULL);
	return (NULL);
}

/* Write a variable-length string. */
void
AG_WriteString(AG_DataSource *ds, const char *s)
{
	Uint32 encLen;
	size_t slen;
	int rv;

	if (s == NULL || *s == '\0') {
		s = "";
		slen = 0;
		encLen = 0;
	} else {
		slen = strlen(s);
		encLen = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(slen) :
		                                               AG_SwapLE32(slen);
	}

	AG_LockDataSource(ds);
	/* Header */
	if (ds->debug) {
		AG_WriteTypeCode(ds, AG_SOURCE_STRING);
	}
	if ((rv = ds->write(ds, &encLen, sizeof(encLen), &ds->wrLast)) != 0) {
		goto fail;
	}
	ds->wrTotal += ds->wrLast;
	
	/* String */
	if (slen > 0) {
		if ((rv = ds->write(ds, s, slen, &ds->wrLast)) != 0) {
			goto fail;
		}
		ds->wrTotal += ds->wrLast;
	}
	AG_UnlockDataSource(ds);
	return;
fail:
	AG_UnlockDataSource(ds);
	AG_DataSourceError(ds, NULL);
}

/* Write a C string encoded as a fixed-length record. */
void
AG_WriteStringPadded(AG_DataSource *ds, const char *s, size_t lenPadded)
{
	size_t slen, padLen, chunkLen;
	Uint32 encLen[2];
	int rv;

	if (s == NULL) {
		s = "";
		slen = 0;
	} else {
		if ((slen = strlen(s)) > lenPadded) {
			AG_DataSourceError(ds, NULL);
			return;
		}
	}

	encLen[0] = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(slen) :
	                                                  AG_SwapLE32(slen);
	encLen[1] = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(lenPadded) :
	                                                  AG_SwapLE32(lenPadded);

	AG_LockDataSource(ds);

	/* Header */
	if (ds->debug) {
		AG_WriteTypeCode(ds, AG_SOURCE_STRING_PAD);
	}
	if ((rv = ds->write(ds, encLen, sizeof(encLen), &ds->wrLast)) != 0) {
		goto fail;
	}
	ds->wrTotal += ds->wrLast;

	/* String */
	if (slen > 0) {
		if ((rv = ds->write(ds, s, slen, &ds->wrLast)) != 0) {
			goto fail;
		}
		ds->wrTotal += ds->wrLast;
	}
	
	/* Padding */
	padLen = lenPadded - slen;
	while (padLen > 0) {
		static char zeroBuf[1024];
	
		chunkLen = AG_MIN(padLen, sizeof(zeroBuf));
		if ((rv = ds->write(ds, zeroBuf, chunkLen, &ds->wrLast)) != 0) {
			goto fail;
		}
		ds->wrTotal += ds->wrLast;
		padLen -= ds->wrLast;
	}
	AG_UnlockDataSource(ds);
	return;
fail:
	AG_UnlockDataSource(ds);
	AG_DataSourceError(ds, NULL);
}

/*
 * Copy at most dst_size bytes from a variable-length string into the
 * destination fixed-size buffer.
 *
 * Return the number of bytes that would have been copied were dst_size
 * unlimited. The returned C string is always NUL-terminated.
 */
size_t
AG_CopyString(char *dst, AG_DataSource *ds, size_t dst_size)
{
	size_t rvLen;
	Uint32 len;
	int rv;

	AG_LockDataSource(ds);

	/* Header */
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_STRING) == -1) {
		goto fail;
	}
	if ((rv = ds->read(ds, &len, sizeof(len), &ds->rdLast)) != 0) {
		goto fail;
	}
	ds->rdTotal += ds->rdLast;
	if (ds->rdLast < sizeof(len)) {
		AG_SetError("String header");
		goto fail;
	}
	len = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(len) :
	                                            AG_SwapLE32(len);
	if (len > (Uint32)(dst_size-1)) {
#ifdef AG_DEBUG
		Verbose("ds(%x): %luB string truncated to fit %luB buffer\n",
		    (Uint)AG_Tell(ds), (Ulong)len, (Ulong)dst_size);
#endif
		rvLen = (size_t)len+1;	/* Save the intended length */
		len = dst_size-1;
	} else {
		rvLen = (size_t)len;
	}
	if (len == 0) {
		*dst = '\0';
	} else {
		if ((rv = ds->read(ds, dst, len, &ds->rdLast)) != 0) {
			goto fail;
		}
		ds->rdTotal += ds->rdLast;
		if (ds->rdLast < len) {
			AG_SetError("Reading string");
			goto fail;
		}
		dst[ds->rdLast] = '\0';
	}
	AG_UnlockDataSource(ds);
	return (rvLen);			/* Count does not include NUL */
fail:
	*dst = '\0';
	AG_UnlockDataSource(ds);
	AG_DataSourceError(ds, NULL);
	return (0);
}

/* Variant of AG_CopyString() for fixed-length records. */
size_t
AG_CopyStringPadded(char *dst, AG_DataSource *ds, size_t dst_size)
{
	size_t rvLen, len, lenPadded, lenPadding;
	Uint32 encLen[2];
	int rv;

	AG_LockDataSource(ds);

	/* Header */
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_STRING_PAD) == -1) {
		goto fail;
	}
	if ((rv = ds->read(ds, encLen, sizeof(encLen), &ds->rdLast)) != 0) {
		goto fail;
	}
	if (ds->rdLast < sizeof(encLen)) {
		AG_SetError("Padded string header");
		goto fail;
	}
	ds->rdTotal += ds->rdLast;
	if (ds->byte_order == AG_BYTEORDER_BE) {
		len = AG_SwapBE32(encLen[0]);
		lenPadded = AG_SwapBE32(encLen[1]);
	} else {
		len = AG_SwapLE32(encLen[0]);
		lenPadded = AG_SwapLE32(encLen[1]);
	}
	if (len > (Uint32)(dst_size-1)) {
#ifdef AG_DEBUG
		Verbose("0x%x: %luB string truncated to fit %luB buffer\n",
		    (Uint)AG_Tell(ds), (Ulong)len, (Ulong)dst_size);
#endif
		rvLen = (size_t)len+1;		/* Save the intended length */
		len = dst_size-1;
	} else {
		rvLen = (size_t)len;
	}

	/* String */
	if (len == 0) {
		*dst = '\0';
	} else {
		if ((rv = ds->read(ds, dst, len, &ds->rdLast)) != 0) {
			goto fail;
		}
		ds->rdTotal += ds->rdLast;
		if (ds->rdLast < len) {
			AG_SetError("Padded string");
			goto fail;
		}
		dst[ds->rdLast] = '\0';
	}

	/* Padding */
	if ((lenPadding = (size_t)(lenPadded-len)) > 0 &&
	    AG_Seek(ds, lenPadding, AG_SEEK_CUR) == -1) {	/* Skip over */
		goto fail;
	}
	AG_UnlockDataSource(ds);
	return (rvLen);				/* Count does not include NUL */
fail:
	AG_UnlockDataSource(ds);
	dst[0] = '\0';
	AG_DataSourceError(ds, NULL);
	return (0);
}

/* Skip over a variable-length string. */
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
	if (AG_Seek(ds, (size_t)len, AG_SEEK_CUR) == -1) {
		goto fail;
	}
	AG_UnlockDataSource(ds);
	return;
fail:
	AG_UnlockDataSource(ds);
	AG_DataSourceError(ds, NULL);
}

/* Skip over a fixed-length string. */
void
AG_SkipStringPadded(AG_DataSource *ds)
{
	Uint32 len, lenPadded;

	AG_LockDataSource(ds);
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_STRING_PAD) == -1) {
		goto fail;
	}
	if (AG_ReadUint32v(ds, &len) == -1 ||
	    AG_ReadUint32v(ds, &lenPadded) == -1) {
		AG_SetError("Padded length: %s", AG_GetError());
		goto fail;
	}
	if (AG_Seek(ds, (size_t)lenPadded, AG_SEEK_CUR) == -1) {
		goto fail;
	}
	AG_UnlockDataSource(ds);
	return;
fail:
	AG_UnlockDataSource(ds);
	AG_DataSourceError(ds, NULL);
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
	dst[0] = '\0';
	AG_UnlockDataSource(ds);
	AG_DataSourceError(ds, NULL);
	return (0);
}
