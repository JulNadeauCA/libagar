/*	$Csoft: math.c,v 1.3 2005/05/19 03:47:20 vedge Exp $	*/
/*	Public domain	*/

#include <core/core.h>
#include "math.h"

int
AG_PowOf2i(int i)
{
	int val = 1;

	while (val < i) { val <<= 1; }
	return (val);
}
