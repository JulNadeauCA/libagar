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

#include <agar/core/begin.h>

#define	AG_RMD160_BLOCK_LENGTH		64
#define	AG_RMD160_DIGEST_LENGTH		20
#define	AG_RMD160_DIGEST_STRING_LENGTH	(AG_RMD160_DIGEST_LENGTH * 2 + 1)

typedef struct ag_rmd160_ctx {
	Uint32 state[5];			/* state */
	Uint64 count;				/* number of bits, mod 2^64 */
	Uint8 buffer[AG_RMD160_BLOCK_LENGTH];	/* input buffer */
} AG_RMD160_CTX;

__BEGIN_DECLS
void  AG_RMD160Init(AG_RMD160_CTX *);
void  AG_RMD160Transform(Uint32 [5], const Uint8 [AG_RMD160_BLOCK_LENGTH]);
void  AG_RMD160Update(AG_RMD160_CTX *, const Uint8 *, size_t)
                      BOUNDED_ATTRIBUTE(__string__,2,3);
void  AG_RMD160Pad(AG_RMD160_CTX *);
void  AG_RMD160Final(Uint8 [AG_RMD160_DIGEST_LENGTH], AG_RMD160_CTX *);
char *AG_RMD160End(AG_RMD160_CTX *, char *);
char *AG_RMD160Data(const Uint8 *, size_t, char *)
                    BOUNDED_ATTRIBUTE(__string__,1,2);
__END_DECLS

#include <agar/core/close.h>
