/*	Public domain	*/

#include <agar/config/have_asprintf.h>
#include <agar/config/_mk_have_sys_types_h.h>

#if defined(__linux__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#ifdef _MK_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <agar/core/core.h>

#ifndef HAVE_ASPRINTF

int
AG_TryAsprintf(char **ret, const char *fmt, ...)
{
	char *buf, *bufNew;
	int size;
	AG_Size buflen;
	va_list ap;

	buflen = strlen(fmt) + 128;	/* Guess */
	if ((buf = TryMalloc(buflen)) == NULL) {
		return (-1);
	}
	va_start(ap, fmt);
	size = vsprintf(buf, fmt, ap);
	va_end(ap);
	if (size <= buflen) {
		*ret = buf;
		return (size);
	}
	if ((bufNew = TryRealloc(buf, size+1)) == NULL) {
		Free(buf);
		return (-1);
	}
	buf = bufNew;
	va_start(ap, fmt);
	size = vsprintf(buf, fmt, ap);
	va_end(ap);
	*ret = buf;
	return (size);
}

void
AG_Asprintf(char **ret, const char *fmt, ...)
{
	char *buf, *bufNew;
	int size;
	AG_Size buflen;
	va_list ap;

	buflen = strlen(fmt) + 128;	/* Guess */
	if ((buf = TryMalloc(buflen)) == NULL) {
		goto fail;
	}
	va_start(ap, fmt);
	size = vsprintf(buf, fmt, ap);
	va_end(ap);
	if (size <= buflen) {
		*ret = buf;
		return;
	}
	if ((bufNew = TryRealloc(buf, size+1)) == NULL) {
		Free(buf);
		goto fail;
	}
	buf = bufNew;
	va_start(ap, fmt);
	size = vsprintf(buf, fmt, ap);
	va_end(ap);
	*ret = buf;
	return;
fail:
	AG_FatalError("asprintf: Out of memory");
}

#else /* HAVE_ASPRINTF */

int
AG_TryAsprintf(char **ret, const char *fmt, ...)
{
	va_list ap;
	int rv;

	va_start(ap, fmt);
	rv = AG_TryVasprintf(ret, fmt, ap);
	va_end(ap);

	if (rv == -1) {
		AG_SetError("asprintf: Out of memory");
		return (-1);
	}
	return (rv);
}

void
AG_Asprintf(char **ret, const char *fmt, ...)
{
	va_list ap;
	int rv;

	va_start(ap, fmt);
	rv = AG_TryVasprintf(ret, fmt, ap);
	va_end(ap);

	if (rv == -1)
		AG_FatalError("asprintf: Out of memory");
}
#endif /* !HAVE_ASPRINTF */
