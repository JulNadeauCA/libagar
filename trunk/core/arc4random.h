/*	Public domain	*/

#include <agar/config/have_arc4random.h>
#include <agar/config/_mk_have_sys_types_h.h>

#ifndef HAVE_ARC4RANDOM
# include <SDL_types.h>
Uint32	arc4random(void);
void	arc4random_stir(void);
void	arc4random_addrandom(unsigned char *, int);
#else
# ifdef _MK_HAVE_SYS_TYPES_H
# include <sys/types.h>
# endif
# include <stdlib.h>
#endif
