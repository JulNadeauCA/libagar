/*	$Csoft: math.c,v 1.2 2005/05/19 01:47:56 vedge Exp $	*/
/*	Public domain	*/

#include <engine/engine.h>

#include "math.h"

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

