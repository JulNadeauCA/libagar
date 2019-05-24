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
 * Serialization functions for strings.
 */
#include <agar/config/ag_serialization.h>
#ifdef AG_SERIALIZATION

#include <agar/core/core.h>

static __inline__ int
ReadLength(AG_DataSource *ds, Uint32 *len)
{
	Uint32 i;
#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_UINT32) == -1)
		return (-1);
#endif
	if (AG_Read(ds, &i, sizeof(i)) != 0) {
		return (-1);
	}
	*len = (ds->byte_order == AG_BYTEORDER_BE) ? AG_SwapBE32(i) :
	                                             AG_SwapLE32(i);
	return (0);
}

/* Allocate and read a length-encoded string. */
char *
AG_ReadStringLen(AG_DataSource *ds, AG_Size maxlen)
{
	Uint32 len;
	char *s;

	AG_LockDataSource(ds);
#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_STRING) == -1)
		goto fail;
#endif
	if (ReadLength(ds, &len) == -1) {
		goto fail;
	}
	if (len > (Uint32)maxlen) {
		AG_SetError("String (%luB): Exceeds %luB limit", (Ulong)len, (Ulong)maxlen);
		goto fail;
	}
	if ((s = TryMalloc((AG_Size)len+1)) == NULL) {
		goto fail;
	}
	if (len > 0) {
	  	if (AG_Read(ds, s, len) != 0) {
			AG_SetError("String (%luB): %s", (Ulong)len, AG_GetError());
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

/* Read a length-encoded string of up to AG_LOAD_STRING_MAX characters. */
char *
AG_ReadString(AG_DataSource *ds)
{
	return AG_ReadStringLen(ds, AG_LOAD_STRING_MAX);
}

/*
 * Allocate and read a length-encoded string with NUL-termination.
 * Type checking is never done; this function is useful when reading
 * non-Agar generated datasets.
 */
char *
AG_ReadNulStringLen(AG_DataSource *ds, AG_Size maxlen)
{
	Uint32 len;
	char *s;

	AG_LockDataSource(ds);
	if (ReadLength(ds, &len) == -1) {
		goto fail;
	}
	if (len > (Uint32)maxlen) {
		AG_SetError("String (%luB): Exceeds %luB limit", (Ulong)len, (Ulong)maxlen);
		goto fail;
	}
	if ((s = TryMalloc((AG_Size)len)) == NULL) {
		goto fail;
	}
	if (len > 0 &&
	    AG_Read(ds, s, (AG_Size)len) != 0) {
		AG_SetError("String (%luB): %s", (Ulong)len, AG_GetError());
		free(s);
		goto fail;
	}
	AG_UnlockDataSource(ds);
	return (s);
fail:
	AG_UnlockDataSource(ds);
	AG_DataSourceError(ds, NULL);
	return (NULL);
}

/*
 * Read a length-encoded and NUL-terminated string of up to AG_LOAD_STRING_MAX
 * characters. No type checking is done.
 */
char *
AG_ReadNulString(AG_DataSource *ds)
{
	return AG_ReadNulStringLen(ds, AG_LOAD_STRING_MAX);
}

/* Write a variable-length string. */
void
AG_WriteString(AG_DataSource *ds, const char *s)
{
	Uint32 encLen;
	AG_Size slen;
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
#ifdef AG_DEBUG
	if (ds->debug)
		AG_WriteTypeCode(ds, AG_SOURCE_STRING);
#endif
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
AG_WriteStringPadded(AG_DataSource *ds, const char *s, AG_Size lenPadded)
{
	AG_Size slen, padLen, chunkLen;
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

#ifdef AG_DEBUG
	if (ds->debug)
		AG_WriteTypeCode(ds, AG_SOURCE_STRING_PAD);
#endif
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
AG_Size
AG_CopyString(char *dst, AG_DataSource *ds, AG_Size dst_size)
{
	AG_Size rvLen;
	Uint32 len;
	int rv;

	AG_LockDataSource(ds);

#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_STRING) == -1)
		goto fail;
#endif
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
		rvLen = (AG_Size)len+1;	/* Save the intended length */
		len = dst_size-1;
	} else {
		rvLen = (AG_Size)len;
	}
	if (len == 0) {
		*dst = '\0';
	} else {
		if ((rv = ds->read(ds, dst, len, &ds->rdLast)) != 0) {
			goto fail;
		}
		ds->rdTotal += ds->rdLast;
		if (ds->rdLast < len) {
			AG_SetErrorS("Reading string");
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
AG_Size
AG_CopyStringPadded(char *dst, AG_DataSource *ds, AG_Size dst_size)
{
	AG_Size rvLen, len, lenPadded, lenPadding;
	Uint32 encLen[2];
	int rv;

	AG_LockDataSource(ds);

#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_STRING_PAD) == -1)
		goto fail;
#endif
	if ((rv = ds->read(ds, encLen, sizeof(encLen), &ds->rdLast)) != 0) {
		goto fail;
	}
	if (ds->rdLast < sizeof(encLen)) {
		AG_SetErrorS("Padded string header");
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
		rvLen = (AG_Size)len+1;		/* Save the intended length */
		len = dst_size-1;
	} else {
		rvLen = (AG_Size)len;
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
			AG_SetErrorS("Padded string");
			goto fail;
		}
		dst[ds->rdLast] = '\0';
	}

	/* Padding */
	if ((lenPadding = (AG_Size)(lenPadded-len)) > 0 &&
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

/* Variant of AG_ReadString() for fixed-length records. */
char *
AG_ReadStringPadded(AG_DataSource *ds, AG_Size maxlen)
{
	char *s;

	if ((s = TryMalloc(maxlen+1)) == NULL) {
		return (NULL);
	}
	if (AG_CopyStringPadded(s, ds, maxlen) == 0) {
		free(s);
		return (NULL);
	}
	return (s);
}

/* Skip over a variable-length string. */
void
AG_SkipString(AG_DataSource *ds)
{
	Uint32 len;

	AG_LockDataSource(ds);
#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_STRING) == -1)
		goto fail;
#endif
	if (ReadLength(ds, &len) == -1) {
		goto fail;
	}
	if (AG_Seek(ds, (AG_Size)len, AG_SEEK_CUR) == -1) {
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
#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_STRING_PAD) == -1)
		goto fail;
#endif
	if (ReadLength(ds, &len) == -1 ||
	    ReadLength(ds, &lenPadded) == -1) {
		goto fail;
	}
	if (AG_Seek(ds, (AG_Size)lenPadded, AG_SEEK_CUR) == -1) {
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
AG_Size
AG_CopyNulString(char *dst, AG_DataSource *ds, AG_Size dst_size)
{
	AG_Size rv;
	Uint32 len;

	AG_LockDataSource(ds);

	if (ReadLength(ds, &len) == -1) {
		goto fail;
	}
	if (len > (Uint32)dst_size) {
#ifdef AG_DEBUG
		Verbose("0x%x: %luB string truncated to fit %luB buffer\n",
		    (Uint)AG_Tell(ds), (Ulong)len, (Ulong)dst_size);
#endif
		rv = (AG_Size)len;		/* Save the intended length */
		len = dst_size;
	} else {
		rv = (AG_Size)len;
	}
	if (AG_Read(ds, dst, (AG_Size)len) != 0)
		goto fail;
	
	AG_UnlockDataSource(ds);
	return (rv-1);			/* Count does not include NUL */
fail:
	dst[0] = '\0';
	AG_UnlockDataSource(ds);
	AG_DataSourceError(ds, NULL);
	return (0);
}
#endif /* AG_SERIALIZATION */
