/*	$Csoft: arc4random.h,v 1.1 2004/02/26 09:19:37 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_arc4random.h>

#ifndef HAVE_ARC4RANDOM
#include <sys/types.h>
Uint32	arc4random(void);
void	arc4random_stir(void);
void	arc4random_addrandom(unsigned char *, int);
#else
#include <sys/types.h>
#include <stdlib.h>
#endif
