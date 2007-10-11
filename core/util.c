/*	Public domain	*/

#include <core/core.h>

#include <config/have_math.h>

#ifdef HAVE_MATH
#include <math.h>
#endif

#include "util.h"

int
AG_PowOf2i(int i)
{
	int val = 1;

	while (val < i) { val <<= 1; }
	return (val);
}

int
AG_Truncf(double d)
{
	return ((int)floor(d));
}

double
AG_Fracf(double d)
{
	return (d - floor(d));
}

double
AG_FracInvf(double d)
{
	return (1 - (d - floor(d)));
}

