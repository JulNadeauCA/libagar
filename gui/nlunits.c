/*	Public domain	*/

#include <agar/core/core.h>
#include <agar/gui/units.h>

double
AG_UnitFahrenheit(double n, int f2k)
{
	if (f2k) {
		return ((n+459.67) / 1.8);
	} else {
		return (1.8*n - 459.67);
	}
}

double
AG_UnitCelsius(double n, int c2k)
{
	if (c2k) {
		return (n+273.15);
	} else {
		return (n-273.15);
	}
}
