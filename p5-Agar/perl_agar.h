/*	Public domain	*/

typedef unsigned char Uchar;
typedef unsigned int Uint;
typedef unsigned long Ulong;

typedef AG_PixelFormat * Agar__PixelFormat;
typedef AG_Surface * Agar__Surface;
typedef AG_Window * Agar__Window;

typedef struct ap_flag_names {
	const char *name;
	Uint bitmask;
} AP_FlagNames;

/* Translate a hashref of options to a set of bit flags. */
static __inline__ void
AP_MapHashToFlags(void *pHV, const AP_FlagNames *map, Uint *pFlags)
{
	SV **val;
	int i;

	for (i = 0; map[i].name != NULL; i++) {
		val = hv_fetch((HV *)pHV, map[i].name, strlen(map[i].name), 0);
		if (val)
			*pFlags |= map[i].bitmask;
	}
}

