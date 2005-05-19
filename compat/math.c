/*	$Csoft: math.c,v 1.2 2005/05/19 01:47:56 vedge Exp $	*/
/*	Public domain	*/

#include <engine/engine.h>

#include "math.h"

fix30
fix30sqrt(fix30 f)
{
	Uint32 root = 0;
	Uint32 remHi = 0;
	Uint32 remLo = f;
	Uint32 count = 30;	/* 15 + (30 >> 1) */
	Uint32 testDiv;

	do {
		remHi = (remHi << 2) | (remLo >> 30);
		remLo <<= 2;
		root <<= 1;
		testDiv = (root << 1) + 1;
		if (remHi >= testDiv) {
			remHi -= testDiv;
			root += 1;
		}
	} while (count-- != 0);

	return (root);
}

int
ftrunc(double d)
{
	return ((int)floor(d));
}

double
ffrac(double d)
{
	return (d - floor(d));
}

double
finvfrac(double d)
{
	return (1 - (d - floor(d)));
}

int
powof2(int i)
{
	int val = 1;

	while (val < i) {
		val <<= 1;
	}
	return (val);
}

