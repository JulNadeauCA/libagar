/*	$Csoft: arc4random.c,v 1.1 2003/12/31 01:55:15 vedge Exp $	*/
/*	$OpenBSD: arc4random.c,v 1.10 2003/11/26 21:40:08 djm Exp $	*/

/*
 * Arc4 random number generator for OpenBSD.
 * Copyright 1996 David Mazieres <dm@lcs.mit.edu>.
 *
 * Modification and redistribution in source and binary forms is
 * permitted provided that due credit is given to the author and the
 * OpenBSD project by leaving this copyright notice intact.
 */

/*
 * This code is derived from section 17.1 of Applied Cryptography,
 * second edition, which describes a stream cipher allegedly
 * compatible with RSA Labs "RC4" cipher (the actual description of
 * which is a trade secret).  The same algorithm is used as a stream
 * cipher called "arcfour" in Tatu Ylonen's ssh package.
 *
 * Here the stream cipher has been modified always to include the time
 * when initializing the state.  That makes it impossible to
 * regenerate the same random sequence twice, so this can't be used
 * for encryption, but will generate good random numbers.
 *
 * RC4 is a registered trademark of RSA Laboratories.
 */

#include <config/have_arc4random.h>

#ifndef HAVE_ARC4RANDOM

#include <engine/engine.h>
#include "arc4random.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#if 0
#include <sys/sysctl.h>
#endif

struct arc4_stream {
	u_int8_t i;
	u_int8_t j;
	u_int8_t s[256];
};

static int rs_initialized;
static struct arc4_stream rs;
static pid_t arc4_stir_pid;

static __inline__ u_int8_t arc4_getbyte(struct arc4_stream *);

static __inline__ void
arc4_init(struct arc4_stream *as)
{
	int     n;

	for (n = 0; n < 256; n++)
		as->s[n] = n;
	as->i = 0;
	as->j = 0;
}

static __inline__ void
arc4_addrandom(struct arc4_stream *as, u_char *dat, int datlen)
{
	int     n;
	u_int8_t si;

	as->i--;
	for (n = 0; n < 256; n++) {
		as->i = (as->i + 1);
		si = as->s[as->i];
		as->j = (as->j + si + dat[n % datlen]);
		as->s[as->i] = as->s[as->j];
		as->s[as->j] = si;
	}
	as->j = as->i;
}

static void
arc4_stir(struct arc4_stream *as)
{
	int     i;
	struct {
		struct timeval tv;
		u_int rnd[(128 - sizeof(struct timeval)) / sizeof(u_int)];
	}       rdat;
	int	fd;

	fd = open("/dev/random", O_RDONLY);

	gettimeofday(&rdat.tv, NULL);
	for (i = 0; i < sizeof(rdat.rnd) / sizeof(u_int); i ++) {
		read(fd, &rdat.rnd[i], sizeof(u_int));
	}

	arc4_stir_pid = getpid();
	arc4_addrandom(as, (void *) &rdat, sizeof(rdat));

	/*
	 * Discard early keystream, as per recommendations in:
	 * http://www.wisdom.weizmann.ac.il/~itsik/RC4/Papers/Rc4_ksa.ps
	 */
	for (i = 0; i < 256; i++)
		(void) arc4_getbyte(as);

	close(fd);
}

static __inline__ u_int8_t
arc4_getbyte(struct arc4_stream *as)
{
	u_int8_t si, sj;

	as->i = (as->i + 1);
	si = as->s[as->i];
	as->j = (as->j + si);
	sj = as->s[as->j];
	as->s[as->i] = sj;
	as->s[as->j] = si;
	return (as->s[(si + sj) & 0xff]);
}

static __inline__ u_int32_t
arc4_getword(struct arc4_stream *as)
{
	u_int32_t val;
	val = arc4_getbyte(as) << 24;
	val |= arc4_getbyte(as) << 16;
	val |= arc4_getbyte(as) << 8;
	val |= arc4_getbyte(as);
	return val;
}

void
arc4random_stir(void)
{
	if (!rs_initialized) {
		arc4_init(&rs);
		rs_initialized = 1;
	}
	arc4_stir(&rs);
}

void
arc4random_addrandom(u_char *dat, int datlen)
{
	if (!rs_initialized)
		arc4random_stir();
	arc4_addrandom(&rs, dat, datlen);
}

u_int32_t
arc4random(void)
{
	if (!rs_initialized || arc4_stir_pid != getpid())
		arc4random_stir();
	return arc4_getword(&rs);
}

#if 0
/*-------- Test code for i386 --------*/
#include <stdio.h>
#include <machine/pctr.h>
int
main(int argc, char **argv)
{
	const int iter = 1000000;
	int     i;
	pctrval v;

	v = rdtsc();
	for (i = 0; i < iter; i++)
		arc4random();
	v = rdtsc() - v;
	v /= iter;

	printf("%qd cycles\n", v);
}
#endif

#endif	/* !HAVE_ARC4RANDOM */
