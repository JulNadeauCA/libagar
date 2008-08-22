/*	$OpenBSD: sha1.h,v 1.23 2004/06/22 01:57:30 jfb Exp $	*/
/*
 * SHA-1 in C
 * By Steve Reid <steve@edmweb.com>
 * 100% Public Domain
 */

#include <agar/config/have_sha1.h>
#ifdef HAVE_SHA1

#include <agar/config/_mk_have_sys_types_h.h>
#ifdef _MK_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <sha1.h>

#else /* !HAVE_SHA1 */

#undef SHA1_BLOCK_LENGTH
#undef SHA1_DIGEST_LENGTH
#undef SHA1_DIGEST_STRING_LENGTH

#define	SHA1_BLOCK_LENGTH		64
#define	SHA1_DIGEST_LENGTH		20
#define	SHA1_DIGEST_STRING_LENGTH	(SHA1_DIGEST_LENGTH * 2 + 1)

typedef struct {
    Uint32 state[5];
    Uint64 count;
    Uint8 buffer[SHA1_BLOCK_LENGTH];
} SHA1_CTX;

#include <agar/config/have_bounded_attribute.h>
#ifndef BOUNDED_ATTRIBUTE
# ifdef HAVE_BOUNDED_ATTRIBUTE
#  define BOUNDED_ATTRIBUTE(t, a, b) __attribute__((__bounded__ (t,a,b)))
# else
#  define BOUNDED_ATTRIBUTE(t, a, b)
# endif
#endif

__BEGIN_DECLS
void SHA1Init(SHA1_CTX *);
void SHA1Pad(SHA1_CTX *);
void SHA1Transform(Uint32 [5], const Uint8 [SHA1_BLOCK_LENGTH]);
void SHA1Update(SHA1_CTX *, const Uint8 *, size_t)
	BOUNDED_ATTRIBUTE(__string__,2,3);
void SHA1Final(Uint8 [SHA1_DIGEST_LENGTH], SHA1_CTX *);
char *SHA1End(SHA1_CTX *, char *);
char *SHA1Data(const Uint8 *, size_t, char *)
	BOUNDED_ATTRIBUTE(__string__,1,2);
__END_DECLS

#endif /* !HAVE_SHA1 */
