/*	$Csoft: tests.c,v 1.2 2005/09/27 04:11:01 vedge Exp $	*/

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

#include <engine/widget/gui.h>

#include "test.h"

extern struct test_ops pixelops_test;
extern struct test_ops primitives_test;
extern struct test_ops surfaceops_test;

struct test_ops *tests[] = {
	&pixelops_test,
	&primitives_test,
	&surfaceops_test
};
int ntests = sizeof(tests) / sizeof(tests[0]);

static void
run_tests(int argc, union evarg *argv)
{
	AG_Table *t = argv[1].p;
	u_int i, j, m;
	Uint32 t1, t2;
	u_long tTot;
	struct testfn_ops *ops;

	for (m = 0; m < t->m; m++) {
		if (!AG_TableRowSelected(t, m)) {
			continue;
		}
		ops = t->cells[m][4].data.p;
		if (((ops->flags & TEST_SDL) && agView->opengl) ||
		    ((ops->flags & TEST_GL)  && !agView->opengl)) {
			continue;
		}
		fprintf(stderr, "Running test: %s...", ops->name);
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
		ops->last_result = (u_long)(tTot / ops->runs);
		fprintf(stderr, "%gms.\n", ops->last_result);
	}
}

static void
poll_test(int argc, union evarg *argv)
{
	AG_Table *t = argv[0].p;
	struct test_ops *test = tests[argv[1].i];
	int i;

	AG_TableBegin(t);
	for (i = 0; i < test->nfuncs; i++) {
		struct testfn_ops *fn = &test->funcs[i];

		AG_TableAddRow(t, "%s:%lu:%lu:%g:%pms", fn->name, fn->runs,
		    fn->iterations, fn->last_result, fn);
	}
	AG_TableEnd(t);
}

static void
quit_app(int argc, union evarg *argv)
{
	AG_Quit();
}

static void
tests_window(void)
{
	AG_Window *win;
	AG_Table *t;
	AG_Button *btn;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	int i;

	win = AG_WindowNew(0, "agar-benchmarks");
	AG_WindowSetCaption(win, "Agar Benchmarks");

	nb = AG_NotebookNew(win, AG_NOTEBOOK_WFILL|AG_NOTEBOOK_HFILL);
	for (i = 0; i < ntests; i++) {
		ntab = AG_NotebookAddTab(nb, tests[i]->name, AG_BOX_VERT);
		t = AG_TablePolled(ntab, AG_TABLE_MULTI, poll_test, "%i", i);
		AG_TableAddCol(t, "Test", "<XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX>",
		    NULL);
		AG_TableAddCol(t, "Runs", "<8888>", NULL);
		AG_TableAddCol(t, "Iterations", "<888888888>", NULL);
		AG_TableAddCol(t, "Result", "<8888888888888888>", NULL);
		AG_TableAddCol(t, "Pointer", NULL, NULL);
		
		btn = AG_ButtonNew(ntab, "Run tests");
		AG_SetEvent(btn, "button-pushed", run_tests, "%p", t);
		AGWIDGET(btn)->flags |= AG_WIDGET_WFILL;
	}
	btn = AG_ButtonNew(win, "Quit");
	AG_SetEvent(btn, "button-pushed", quit_app, NULL);
	AGWIDGET(btn)->flags |= AG_WIDGET_WFILL;

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

	if (AG_InitVideo(640, 480, 32, 0) == -1 ||
	    AG_InitInput(0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_InitConfigWin(AG_CONFIG_FULLSCREEN|AG_CONFIG_GL|
	                 AG_CONFIG_RESOLUTION);
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F1, KMOD_NONE, AG_ShowSettings);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);
	AG_SetRefreshRate(fps);

	tests_window();

	AG_EventLoop();
	AG_Destroy();
	return (0);
fail:
	AG_Destroy();
	return (1);
}

