/*	$Csoft: asprintf.h,v 1.2 2002/11/07 18:25:13 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mcconfig.h>

#ifndef HAVE_ASPRINTF
#error "no compat asprintf()"
#endif

#ifdef __linux__
#define _GNU_SOURCE
#endif

#include <stdio.h>

#undef _GNU_SOURCE

