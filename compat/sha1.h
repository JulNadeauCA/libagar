/*	$Csoft$	*/
/*	$OpenBSD: sha1.h,v 1.23 2004/06/22 01:57:30 jfb Exp $	*/

/*
 * SHA-1 in C
 * By Steve Reid <steve@edmweb.com>
 * 100% Public Domain
 */

#include <config/have_sha1.h>

#ifdef HAVE_SHA1

#include <sys/types.h>
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

#include <sys/cdefs.h>

__BEGIN_DECLS
void SHA1Init(SHA1_CTX *);
void SHA1Pad(SHA1_CTX *);
void SHA1Transform(Uint32, const Uint8);
void SHA1Update(SHA1_CTX *, const Uint8 *, size_t)
	BOUNDED_ATTRIBUTE(__string__,2,3);
void SHA1Final(Uint8, SHA1_CTX *);
char *SHA1End(SHA1_CTX *, char *);
char *SHA1File(const char *, char *);
char *SHA1FileChunk(const char *, char *, off_t, off_t);
char *SHA1Data(const Uint8 *, size_t, char *)
	BOUNDED_ATTRIBUTE(__string__,1,2);
__END_DECLS

#endif /* !HAVE_SHA1 */
