/*	$Csoft: asprintf.h,v 1.2 2002/10/19 06:42:42 vedge Exp $	*/
/*	Public domain	*/

#include <compat/config.h>

#ifndef HAVE_ASPRINTF
#error "no compat asprintf()"
#endif

#ifdef __linux__
#define _GNU_SOURCE
#endif

#include <stdio.h>

