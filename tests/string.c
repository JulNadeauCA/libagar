/*	Public domain	*/

/*
 * This program tests the string routines in ag_core.
 */

#include <agar/config/ag_enable_string.h>

#include "agartest.h"

#include <agar/math/m.h>
#include <agar/math/m_gui.h>

#include <string.h>

static int
Init(void *obj)
{
	M_InitSubsystem();
	return (0);
}

static int
Test(void *obj)
{
	char someString[64], buf[1024];
	AG_TestInstance *ti = obj;
	int i = -123;
	Uint u = 123;
	float flt = 1.234f;
	double dbl = 2.345;
	long l = -123456;
	Ulong ul = 123456;
	Uint8 u8 = 255;
	Sint8 s8 = -100;
	Uint16 u16 = 1616;
	Sint16 s16 = -1616;
	Uint32 u32 = 323232;
	Sint32 s32 = -323232;
	AG_FmtString *fs;
	M_Vector2 v2 = M_VECTOR2(2.1, M_PI);
	M_Vector3 v3 = M_VECTOR3(3.1, 3.2, M_E);

	Strlcpy(someString, "Some string", sizeof(someString));

	TestMsgS(ti, "AG_Printf() test:");
	TestMsgS(ti, AG_Printf("\tSome string: \"%s\", \"%10s\"", someString, someString));
	TestMsgS(ti, AG_Printf("\tInt: [%d] [%8d] [%08d]", i, i, i));
	TestMsgS(ti, AG_Printf("\tUint: [%u] [%8u] [%08u]", u, u, u));
	TestMsgS(ti, AG_Printf("\tLong: [%ld] [%8ld] [%08ld]", l, l, l));
	TestMsgS(ti, AG_Printf("\tUlong: [%lu] [%8lu] [%08lu]", ul, ul, ul));
	TestMsgS(ti, AG_Printf("\tDbl: [%f] [%.0f] [%.1f] [%.2f] [%.08f]", dbl, dbl, dbl, dbl, dbl));
	TestMsgS(ti, AG_Printf("\tInt=%d Uint=%u Dbl=%f Long=%ld Ulong=%lu", i, u, dbl, l, ul));
	
	TestMsgS(ti, "AG_Printf() extended specifiers:");
	TestMsgS(ti, AG_Printf("\tu8=%[u8], s8=%[s8]", &u8, &s8));
	TestMsgS(ti, AG_Printf("\tu16=%[u16], s16=%[s16]", &u16, &s16));
	TestMsgS(ti, AG_Printf("\tu32=%[u32], s32=%[s32]", &u32, &s32));
	TestMsgS(ti, AG_Printf("\tv2=%[V2], v3=%[V3]", &v2, &v3));

	TestMsgS(ti, "AG_PrintfP() test:");
	fs = AG_PrintfP("\tString: \"%s\"", someString);
	AG_ProcessFmtString(fs, buf, sizeof(buf));
	TestMsgS(ti, buf);
	fs = AG_PrintfP("\tNatural integers: %d, %u", &i, &u);
	AG_ProcessFmtString(fs, buf, sizeof(buf));
	TestMsgS(ti, buf);
	fs = AG_PrintfP("\tFixed integers: %[u8],%[s8],%[u16],%[s16],"
	                "%[u32],%[s32]", &u8, &s8, &u16, &s16, &u32, &s32);
	AG_ProcessFmtString(fs, buf, sizeof(buf));
	TestMsgS(ti, buf);
	fs = AG_PrintfP("\tFloats: %f, %lf", &flt, &dbl);
	AG_ProcessFmtString(fs, buf, sizeof(buf));
	TestMsgS(ti, buf);
	fs = AG_PrintfP("\tVectors: %[V2], %[V3]", &v2, &v3);
	AG_ProcessFmtString(fs, buf, sizeof(buf));
	TestMsgS(ti, buf);

	return (0);
}

const AG_TestCase stringTest = {
	"string",
	N_("Test the ag_core string functions"),
	"1.5.0",
	0,
	sizeof(AG_TestInstance),
	Init,
	NULL,		/* destroy */
	Test,
	NULL,		/* testGUI */
	NULL		/* bench */
};
