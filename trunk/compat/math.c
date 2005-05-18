/*	$Csoft: snprintf.c,v 1.1 2004/02/26 09:19:38 vedge Exp $	*/
/*	Public domain	*/

fix30
fix30sqrt(fix30 f)
{
	Uint32 root = 0;
	Uint32 remHi = 0
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

