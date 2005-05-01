/*	$Csoft: rmd160.h,v 1.1 2005/04/25 07:47:50 vedge Exp $	*/
/*	$OpenBSD: rmd160.h,v 1.16 2004/06/22 01:57:30 jfb Exp $	*/

/*
 * Copyright (c) 2001 Markus Friedl.  All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <config/have_rmd160.h>

#ifdef HAVE_RMD160

#include <sys/types.h>
#include <rmd160.h>

#else /* !HAVE_RMD160 */

#undef RMD160_BLOCK_LENGTH
#undef RMD160_DIGEST_LENGTH
#undef RMD160_DIGEST_STRING_LENGTH

#define	RMD160_BLOCK_LENGTH		64
#define	RMD160_DIGEST_LENGTH		20
#define	RMD160_DIGEST_STRING_LENGTH	(RMD160_DIGEST_LENGTH * 2 + 1)

typedef struct RMD160Context {
	Uint32 state[5];			/* state */
	Uint64 count;				/* number of bits, mod 2^64 */
	Uint8 buffer[RMD160_BLOCK_LENGTH];	/* input buffer */
} RMD160_CTX;

#include <sys/cdefs.h>

__BEGIN_DECLS
void	 RMD160Init(RMD160_CTX *);
void	 RMD160Transform(Uint32 [5], const Uint8 [RMD160_BLOCK_LENGTH]);
void	 RMD160Update(RMD160_CTX *, const Uint8 *, size_t)
		BOUNDED_ATTRIBUTE(__string__,2,3);
void	 RMD160Pad(RMD160_CTX *);
void	 RMD160Final(Uint8 [RMD160_DIGEST_LENGTH], RMD160_CTX *);
char	*RMD160End(RMD160_CTX *, char *);
char	*RMD160File(const char *, char *);
char	*RMD160FileChunk(const char *, char *, off_t, off_t);
char	*RMD160Data(const Uint8 *, size_t, char *)
		BOUNDED_ATTRIBUTE(__string__,1,2);
__END_DECLS

#endif /* !HAVE_RMD160 */
