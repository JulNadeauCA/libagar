/*	$Csoft$	*/
/*	$NetBSD: libintl.h,v 1.1.1.1 2000/10/31 10:45:04 itojun Exp $	*/

/*-
 * Copyright (c) 2000 Citrus Project,
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef __LIBINTL_H_DEFINED__
#define __LIBINTL_H_DEFINED__
#include "begin_code.h"

__BEGIN_DECLS
__inline__ char	*gettext(const char *);
__inline__ char	*dgettext(const char *, const char *);
__inline__ char	*dcgettext(const char *, const char *, int);
__inline__ char	*ngettext(const char *, const char *, unsigned long int);
__inline__ char	*dngettext(const char *, const char *, const char *,
		           unsigned long int);
char		*dcngettext(const char *, const char *, const char *,
		            unsigned long int, int);
char	*textdomain(const char *);
char	*bindtextdomain(const char *, const char *);
char	*bind_textdomain_codeset(const char *, const char *);
__END_DECLS

#include "close_code.h"
#endif /*__LIBINTL_H_DEFINED__*/
