/*	$Csoft: arc4random.h,v 1.2 2004/04/21 00:15:21 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_arc4random.h>

#ifndef HAVE_ARC4RANDOM
#include <SDL/SDL_types.h>
#include <sys/types.h>
Uint32	arc4random(void);
void	arc4random_stir(void);
void	arc4random_addrandom(unsigned char *, int);
#else
#include <sys/types.h>
#include <stdlib.h>
#endif
