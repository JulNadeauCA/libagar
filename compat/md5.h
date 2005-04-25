/*	$Csoft$	*/
/*	$OpenBSD: md5.h,v 1.16 2004/06/22 01:57:30 jfb Exp $	*/

/*
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 */

#include <config/have_md5.h>
#ifdef HAVE_MD5

#include <sys/types.h>
#include <md5.h>

#else /* !HAVE_MD5 */

#undef MD5_BLOCK_LENGTH
#undef MD5_DIGEST_LENGTH
#undef MD5_DIGEST_STRING_LENGTH

#define	MD5_BLOCK_LENGTH		64
#define	MD5_DIGEST_LENGTH		16
#define	MD5_DIGEST_STRING_LENGTH	(MD5_DIGEST_LENGTH * 2 + 1)

typedef struct MD5Context {
	Uint32 state[4];			/* state */
	Uint64 count;				/* number of bits, mod 2^64 */
	Uint8 buffer[MD5_BLOCK_LENGTH];		/* input buffer */
} MD5_CTX;

__BEGIN_DECLS
void	 MD5Init(MD5_CTX *);
void	 MD5Update(MD5_CTX *, const Uint8 *, size_t)
	 	BOUNDED_ATTRIBUTE(__string__,2,3);
void	 MD5Pad(MD5_CTX *);
void	 MD5Final(Uint8, MD5_CTX *);
void	 MD5Transform(Uint32, const Uint8)
char	*MD5End(MD5_CTX *, char *);
char	*MD5File(const char *, char *);
char	*MD5FileChunk(const char *, char *, off_t, off_t)
char	*MD5Data(const Uint8 *, size_t, char *)
		BOUNDED_ATTROBUTE(__string__,1,2);
__END_DECLS

#endif /* !HAVE_MD5 */
