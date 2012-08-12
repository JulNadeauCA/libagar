/*	Public domain	*/

/*
 * This program tests the string routines in ag_core.
 */

#include "agartest.h"

#include <string.h>

static int
Test(void *obj)
{
	AG_TestInstance *ti = obj;
	int i = -123;
	Uint u = 123;
	double dbl = 2.345;
	long l = -123456;
	Ulong ul = 123456;
	Uint8 u8 = 255;
	Sint8 s8 = -100;
	Uint16 u16 = 1616;
	Sint16 s16 = -1616;
	Uint32 u32 = 323232;
	Sint32 s32 = -323232;

	TestMsgS(ti, "AG_Printf() test:");
	TestMsgS(ti, AG_Printf("String: \"%s\", \"%10s\"", "Some string", "Pad"));
	TestMsgS(ti, AG_Printf("Int: [%d] [%8d] [%08d]", i, i, i));
	TestMsgS(ti, AG_Printf("Uint: [%u] [%8u] [%08u]", u, u, u));
	TestMsgS(ti, AG_Printf("Long: [%ld] [%8ld] [%08ld]", l, l, l));
	TestMsgS(ti, AG_Printf("Ulong: [%lu] [%8lu] [%08lu]", ul, ul, ul));
	TestMsgS(ti, AG_Printf("Dbl: [%f] [%.0f] [%.1f] [%.2f] [%.08f]", dbl, dbl, dbl, dbl, dbl));
	TestMsgS(ti, AG_Printf("int=%d Uint=%u dbl=%f long=%ld ulong=%lu", i, u, dbl, l, ul));
	
	TestMsgS(ti, "AG_Printf() extended specifiers:");
	TestMsgS(ti, AG_Printf("u8=%[u8], s8=%[s8]", &u8, &s8));
	TestMsgS(ti, AG_Printf("u16=%[u16], s16=%[s16]", &u16, &s16));
	TestMsgS(ti, AG_Printf("u32=%[u32], s32=%[s32]", &u32, &s32));

	return (0);
}

const AG_TestCase stringTest = {
	"string",
	N_("Test the ag_core string functions"),
	"1.5.0",
	0,
	sizeof(AG_TestInstance),
	NULL,	/* init */
	NULL,	/* destroy */
	Test,
	NULL	/* testGUI */
};
