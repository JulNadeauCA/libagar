/*	$Csoft$	*/
/*	Public domain	*/

#include <config/historical_units.h>

#include <engine/engine.h>

#include "units.h"

double
unit_fahrenheit(double n, int f2k)
{
	if (f2k) {
		return ((n+459.67) / 1.8);
	} else {
		return (1.8*n - 459.67);
	}
}

double
unit_celsius(double n, int c2k)
{
	if (c2k) {
		return (n+273.15);
	} else {
		return (n-273.15);
	}
}

#ifdef HISTORICAL_UNITS
double
unit_rankine(double n, int ra2k)
{
	if (ra2k) {
		return (n/1.8);
	} else {
		return (1.8*n);
	}
}

double
unit_reaumur(double n, int re2k)
{
	if (re2k) {
		return (n*1.25 + 273.15);
	} else {
		return ((n-273.15)/1.25);
	}
}
#endif /* HISTORICAL_UNITS */

