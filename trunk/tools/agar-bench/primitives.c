/*	$Csoft: primitives.c,v 1.4 2005/10/03 07:17:31 vedge Exp $	*/
/*	Public domain	*/

#include "agar-bench.h"
#include <agar/gui/primitive.h>

static AG_Widget wid;

static void
InitWidget(void)
{
	AG_ObjectInitStatic(&wid, &agWidgetClass);
	wid.x = 0;
	wid.y = 0;
	wid.w = agView->w-1;
	wid.h = agView->h-1;
	wid.rView = AG_RECT2(1, 1, wid.w, wid.h);
}

static void
FreeWidget(void)
{
	AG_ObjectDestroy(&wid);
}

static void
T_Box(void)
{
	AG_DrawBox(&wid, AG_RECT(0, 0, wid.w/2, wid.h/2), 1, 0);
}

static void
T_BoxChamfered(void)
{
	AG_DrawBoxRounded(&wid, AG_RECT(0,0,agView->w/2,agView->h/2), 1, 32, 0);
}

static void
T_Frame(void)
{
	AG_DrawFrame(&wid, AG_RECT(0, 0, wid.w, wid.h), 1, 0);
}

static void
T_Circle(void)
{
	AG_DrawCircle(&wid, 0, 0, wid.w/3, 0);
}

static void
T_Line(void)
{
	AG_DrawLine(&wid, 0, 0, wid.w, wid.h, 0);
	AG_DrawLine(&wid, 0, 0, wid.w, wid.h/2, 0);
}

static void
T_LineBlended(void)
{
	AG_Color C;

	AG_DrawLineBlended(&wid, 0, 0, wid.w, wid.h, C, AG_ALPHA_SRC);
	AG_DrawLineBlended(&wid, 0, 0, wid.w, wid.h/2, C, AG_ALPHA_SRC);
}

static void
T_HLine(void)
{
	AG_DrawLineH(&wid, 1, 479, 1, 0);
}

static void
T_VLine(void)
{
	AG_DrawLineV(&wid, 1, 1, 479, 0);
}

static void
T_RectFilled(void)
{
	AG_DrawRectFilled(&wid, AG_RECT(1, 1, 256, 256), 0);
}

static void
T_RectBlended(void)
{
	Uint8 c[4] = { 100,200,100,128 };
	AG_DrawRectBlended(&wid, AG_RECT(1, 1, 128, 128), c, AG_ALPHA_SRC);
}

static void
T_Tiling16(void)
{
	AG_DrawTiling(&wid, AG_RECT(0,0,wid.w,wid.h), 16, 0, 255, 0);
}

static void
T_Tiling32(void)
{
	AG_DrawTiling(&wid, AG_RECT(0,0,wid.w,wid.h), 32, 0, 255, 0);
}

static struct testfn_ops testfns[] = {
 { "Box", InitWidget, FreeWidget, T_Box },
 { "BoxRounded", InitWidget, FreeWidget, T_BoxChamfered },
 { "Frame", InitWidget, FreeWidget, T_Frame },
 { "Circle", InitWidget, FreeWidget, T_Circle },
 { "Line", InitWidget, FreeWidget, T_Line },
 { "LineBlended", InitWidget, FreeWidget, T_LineBlended },
 { "LineH(479px)", InitWidget, FreeWidget, T_HLine },
 { "LineV(479px)", InitWidget, FreeWidget, T_VLine },
 { "RectFilled(256x256)", InitWidget, FreeWidget, T_RectFilled },
 { "RectBlended(128x128)", InitWidget, FreeWidget, T_RectBlended },
 { "Tiling(16x16)", InitWidget, FreeWidget, T_Tiling16 },
 { "Tiling(32x32)", InitWidget, FreeWidget, T_Tiling32 },
};

struct test_ops primitives_test = {
	"Primitives",
	NULL,
	&testfns[0],
	sizeof(testfns) / sizeof(testfns[0]),
	TEST_SDL,
	4, 32, 0
};
