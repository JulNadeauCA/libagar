/*	$OpenBSD: sha1.h,v 1.23 2004/06/22 01:57:30 jfb Exp $	*/
/*
 * SHA-1 in C
 * By Steve Reid <steve@edmweb.com>
 * 100% Public Domain
 */

#include <agar/config/have_sha1.h>

#ifdef HAVE_SHA1

# include <agar/config/_mk_have_sys_types_h.h>
# ifdef _MK_HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif
# include <sha1.h>

# define AG_SHA1_BLOCK_LENGTH		SHA1_BLOCK_LENGTH
# define AG_SHA1_DIGEST_LENGTH		SHA1_DIGEST_LENGTH
# define AG_SHA1_DIGEST_STRING_LENGTH	SHA1_DIGEST_STRING_LENGTH

# define AG_SHA1Init		SHA1Init
# define AG_SHA1Pad		SHA1Pad
# define AG_SHA1Transform	SHA1Transform
# define AG_SHA1Update		SHA1Update
# define AG_SHA1Final		SHA1Final
# define AG_SHA1End		SHA1End
# define AG_SHA1Data		SHA1Data

#else /* !HAVE_SHA1 */

#include <agar/begin.h>

#define AG_SHA1_BLOCK_LENGTH		64
#define AG_SHA1_DIGEST_LENGTH		20
#define AG_SHA1_DIGEST_STRING_LENGTH	(AG_SHA1_DIGEST_LENGTH * 2 + 1)

typedef struct ag_sha1_ctx {
	Uint32 state[5];
	Uint64 count;
	Uint8 buffer[AG_SHA1_BLOCK_LENGTH];
} AG_SHA1_CTX;

__BEGIN_DECLS
void  AG_SHA1Init(AG_SHA1_CTX *);
void  AG_SHA1Pad(AG_SHA1_CTX *);
void  AG_SHA1Transform(Uint32 [5], const Uint8 [AG_SHA1_BLOCK_LENGTH]);
void  AG_SHA1Update(AG_SHA1_CTX *, const Uint8 *, size_t)
                    BOUNDED_ATTRIBUTE(__string__,2,3);
void  AG_SHA1Final(Uint8 [AG_SHA1_DIGEST_LENGTH], AG_SHA1_CTX *);
char *AG_SHA1End(AG_SHA1_CTX *, char *);
char *AG_SHA1Data(const Uint8 *, size_t, char *)
                  BOUNDED_ATTRIBUTE(__string__,1,2);
__END_DECLS

#include <agar/close.h>
#endif /* !HAVE_SHA1 */
