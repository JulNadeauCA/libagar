/*	$Csoft: asprintf.h,v 1.1 2002/11/07 17:51:17 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mcconfig.h>

#ifndef HAVE_ASPRINTF
#error "no compat asprintf()"
#endif

#ifdef __linux__
#define _GNU_SOURCE
#endif

#include <stdio.h>

