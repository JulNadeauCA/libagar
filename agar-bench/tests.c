/*	$Csoft: tests.c,v 1.1 2005/09/27 03:48:58 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <engine/engine.h>
#include <engine/input.h>
#include <engine/config.h>
#include <engine/view.h>

#include <string.h>
#include <unistd.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/tlist.h>
#include <engine/widget/button.h>
#include <engine/widget/label.h>
#include <engine/widget/separator.h>

#include "test.h"

extern struct test_ops pixelops_test;
extern struct test_ops primitives_test;

struct test_ops *tests[] = {
	&pixelops_test,
	&primitives_test
};
int ntests = sizeof(tests) / sizeof(tests[0]);

static void
test_params(int argc, union evarg *argv)
{
	AG_Tableview *tv = argv[1].p;
	AG_TableviewRow *row;
	AG_Window *win;
	struct test_ops *ops = NULL;

	TAILQ_FOREACH(row, &tv->children, siblings) {
		if (row->selected) {
			ops = row->userp;
		}
	}
	if (ops == NULL)
		return;

	win = AG_WindowNew(0, NULL);
	AG_WindowSetCaption(win, ops->name);
	ops->edit(win);
	AG_WindowShow(win);
}

static void
run_tests(int argc, union evarg *argv)
{
	AG_Tableview *tv = argv[1].p;
	u_int i, j, ti, fi;
	AG_Window *win;
	AG_Button *btn;
	Uint32 t1, t2;
	u_long tTot;

	for (ti = 0; ti < ntests; ti++) {
		struct test_ops *test = tests[ti];

		for (fi = 0; fi < test->nfuncs; fi++) {
			struct testfn_ops *ops = &test->funcs[fi];
			u_long tot;

			if (((ops->flags & TEST_SDL) && agView->opengl) ||
			    ((ops->flags & TEST_GL)  && !agView->opengl))
				continue;

			if (ops->init != NULL) ops->init();
			for (i = 0, tTot = 0; i < ops->runs; i++) {
				t1 = SDL_GetTicks();
				for (j = 0; j < ops->iterations; j++) {
					ops->run();
				}
				t2 = SDL_GetTicks();
				tTot += (t2 - t1);
			}
			if (ops->destroy != NULL) ops->destroy();

			tot = (u_long)(tTot / ops->runs);
			ops->last_result = ((double)tTot /
			                    (double)ops->iterations) * 1e5;
		}
	}
}

static char *
tests_callback(AG_Tableview *tv, AG_TableviewColID cid, AG_TableviewRowID rid)
{
	static char text[32];
	AG_TableviewRow *row = AG_TableviewRowGet(tv, rid);
	struct testfn_ops *ops = row->userp;

	switch (cid) {
	case 0:
		return (ops->name);
	case 1:
		snprintf(text, sizeof(text), "%u", ops->runs);
		return (text);
	case 2:
		snprintf(text, sizeof(text), "%u", ops->iterations);
		return (text);
	case 3:
		if (ops->last_result > 10000) {
			snprintf(text, sizeof(text), "%f ms (%u/%u)",
			    (double)(ops->last_result*1e-06), ops->runs,
			    ops->iterations);
		} else {
			snprintf(text, sizeof(text), "%g ns (%u/%u)",
			    ops->last_result, ops->runs, ops->iterations);
		}
		return (text);
	}
}

static void
tests_window(void)
{
	AG_Window *win;
	AG_Tableview *tv;
	AG_Button *btn;
	AG_Box *bo;
	int i, id = 0;

	win = AG_WindowNew(0, "agar-bench");
	AG_WindowSetCaption(win, "Agar Benchmarks");

	tv = AG_TableviewNew(win, 0, tests_callback, NULL);
	AG_TableviewSetUpdate(tv, 250);
	AG_TableviewColAdd(tv, AG_TABLEVIEW_COL_DYNAMIC|AG_TABLEVIEW_COL_UPDATE,
			      0, "Test",
			      "< VIEW_PUT_PIXEL_2_CLIPPED() -- Clipped >");
	AG_TableviewColAdd(tv, AG_TABLEVIEW_COL_DYNAMIC|AG_TABLEVIEW_COL_UPDATE,
			      1, "Runs", "<XX>");
	AG_TableviewColAdd(tv, AG_TABLEVIEW_COL_DYNAMIC|AG_TABLEVIEW_COL_UPDATE,
			      2, "Iterations", "<XXXXXXXXX>");
	AG_TableviewColAdd(tv, AG_TABLEVIEW_COL_DYNAMIC|AG_TABLEVIEW_COL_UPDATE,
			      3, "Result", NULL);

	for (i = 0; i < ntests; i++) {
		struct test_ops *test = tests[i];
		AG_TableviewRow *pRow;
		int j;

		pRow = AG_TableviewRowAdd(tv, AG_TABLEVIEW_STATIC_ROW,
		    NULL, NULL, id++,
		    0, test->name,
		    1, "",
		    2, "",
		    3, "");

		for (j = 0; j < test->nfuncs; j++) {
			struct testfn_ops *func = &test->funcs[j];

			AG_TableviewRowAdd(tv, 0, pRow, func, id++,
			    0, func->name,
			    1, "",
			    2, "",
			    3, "");
		}
	}

	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_WFILL|AG_BOX_HOMOGENOUS);
	{
		btn = AG_ButtonNew(bo, "Run tests");
		AG_SetEvent(btn, "button-pushed", run_tests, "%p", tv);

		btn = AG_ButtonNew(bo, "Parameters");
		AG_SetEvent(btn, "button-pushed", test_params, "%p", tv);
	}

	AG_WindowShow(win);
	AG_WindowSetGeometry(win,
	    10, 10,
	    agView->w - 20, agView->h - 20);
}

int
main(int argc, char *argv[])
{
	int c, i, fps = -1;
	char *s;

	if (AG_InitCore("agar-bench", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}

	while ((c = getopt(argc, argv, "?vfFegGw:h:t:r:T:")) != -1) {
		extern char *optarg;

		switch (c) {
		case 'v':
			exit(0);
		case 'f':
			AG_SetBool(agConfig, "view.full-screen", 1);
			break;
		case 'F':
			AG_SetBool(agConfig, "view.full-screen", 0);
			break;
#ifdef HAVE_OPENGL
		case 'g':
			AG_SetBool(agConfig, "view.opengl", 1);
			break;
		case 'G':
			AG_SetBool(agConfig, "view.opengl", 0);
			break;
#endif
		case 'w':
			AG_SetUint16(agConfig, "view.w", atoi(optarg));
			break;
		case 'h':
			AG_SetUint16(agConfig, "view.h", atoi(optarg));
			break;
		case 't':
			AG_TextParseFontSpec(optarg);
			break;
		case 'T':
			AG_SetString(agConfig, "font-path", "%s", optarg);
			break;
		case 'r':
			fps = atoi(optarg);
			break;
		case '?':
		default:
			printf("%s [-vfF] [-w width] [-h height] [-r fps]"
			       " [-t font,size,flags] [-T font-path]",
			    agProgName);
#ifdef HAVE_OPENGL
			printf(" [-gG]");
#endif
			printf("\n");
			exit(0);
		}
	}

	if (AG_InitVideo(320, 240, 32, 0) == -1 ||
	    AG_InitInput(AG_INPUT_KBDMOUSE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_InitConfigWin(AG_CONFIG_FULLSCREEN|AG_CONFIG_GL|
	                 AG_CONFIG_RESOLUTION);
	AG_SetRefreshRate(fps);

	tests_window();

	AG_EventLoop();
	AG_Quit();
	return (0);
fail:
	AG_Quit();
	return (1);
}

