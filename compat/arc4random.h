/*	$Csoft: arc4random.h,v 1.1 2003/12/31 01:55:15 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_arc4random.h>

#ifndef HAVE_ARC4RANDOM
#include <sys/types.h>
u_int32_t	arc4random(void);
void		arc4random_stir(void);
void		arc4random_addrandom(u_char *, int);
#else
#include <sys/types.h>
#include <stdlib.h>
#endif
