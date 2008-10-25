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

#include <agar/config/have_md5.h>

#ifdef HAVE_MD5

# include <agar/config/_mk_have_sys_types_h.h>
# ifdef _MK_HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif
# include <md5.h>

# define AG_MD5_BLOCK_LENGTH		MD5_BLOCK_LENGTH
# define AG_MD5_DIGEST_LENGTH		MD5_DIGEST_LENGTH
# define AG_MD5_DIGEST_STRING_LENGTH	MD5_DIGEST_STRING_LENGTH

# define AG_MD5Init		MD5Init
# define AG_MD5Update		MD5Update
# define AG_MD5Pad 		MD5Pad
# define AG_MD5Final		MD5Final
# define AG_MD5Transform	MD5Transform
# define AG_MD5End		MD5End
# define AG_MD5Data		MD5Data

#else /* !HAVE_MD5 */

#include <agar/begin.h>

#define	AG_MD5_BLOCK_LENGTH		64
#define	AG_MD5_DIGEST_LENGTH		16
#define	AG_MD5_DIGEST_STRING_LENGTH	(AG_MD5_DIGEST_LENGTH * 2 + 1)

typedef struct ag_md5_ctx {
	Uint32 state[4];			/* state */
	Uint64 count;				/* number of bits, mod 2^64 */
	Uint8 buffer[AG_MD5_BLOCK_LENGTH];	/* input buffer */
} AG_MD5_CTX;

__BEGIN_DECLS
void  AG_MD5Init(AG_MD5_CTX *);
void  AG_MD5Update(AG_MD5_CTX *, const Uint8 *, size_t)
                   BOUNDED_ATTRIBUTE(__string__,2,3);
void  AG_MD5Pad(AG_MD5_CTX *);
void  AG_MD5Final(Uint8 [AG_MD5_DIGEST_LENGTH], AG_MD5_CTX *);
void  AG_MD5Transform(Uint32 [4], const Uint8 [AG_MD5_BLOCK_LENGTH]);
char *AG_MD5End(AG_MD5_CTX *, char *);
char *AG_MD5Data(const Uint8 *, size_t, char *)
                 BOUNDED_ATTRIBUTE(__string__,1,2);
__END_DECLS

#include <agar/close.h>
#endif /* !HAVE_MD5 */
