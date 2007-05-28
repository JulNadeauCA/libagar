/*	$Csoft: error.c,v 1.1.1.1 2005/01/29 01:06:08 vedge Exp $	*/
/*	Public domain	*/

#include <agar/config/network.h>
#ifdef NETWORK

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <err.h>

#include "net.h"

char *agnErrorMsg = NULL;

const char *
AGN_GetError(void)
{
	return ((const char *)agnErrorMsg);
}

void
AGN_SetError(const char *fmt, ...)
{
	va_list args;
	char *buf;
	
	va_start(args, fmt);
	if (vasprintf(&buf, fmt, args) == -1) {
		err(1, "vasprintf");
	}
	va_end(args);

	free(agnErrorMsg);
	agnErrorMsg = buf;
}

void *
AGN_Malloc(size_t size)
{
	void *p;

	if ((p = malloc(size)) == NULL) {
		err(1, "malloc %u", (unsigned)size);
	}
	return (p);
}

void *
AGN_Realloc(void *p, size_t size)
{
	void *rp;

	if ((rp = realloc(p, size)) == NULL) {
		err(1, "malloc %u", (unsigned)size);
	}
	return (rp);
}

#endif /* NETWORK */
