/*	$Csoft: tests.c,v 1.2 2005/05/12 06:57:32 vedge Exp $	*/

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
	struct tableview *tv = argv[1].p;
	struct tableview_row *row;
	struct test_ops *ops = NULL;
	struct window *win;

	TAILQ_FOREACH(row, &tv->children, siblings) {
		if (row->selected) {
			ops = row->userp;
		}
	}
	if (ops == NULL)
		return;

	win = window_new(0, NULL);
	window_set_caption(win, ops->name);
	ops->edit(win);
	window_show(win);
}

static void
run_tests(int argc, union evarg *argv)
{
	struct tableview *tv = argv[1].p;
	u_int i, j, ti, fi;
	struct window *win;
	struct button *btn;
	Uint32 t1, t2;
	u_long tTot;

	for (ti = 0; ti < ntests; ti++) {
		struct test_ops *test = tests[ti];

		for (fi = 0; fi < test->nfuncs; fi++) {
			struct testfn_ops *ops = &test->funcs[fi];
			u_long tot;

			if (((ops->flags & TEST_SDL) && view->opengl) ||
			    ((ops->flags & TEST_GL)  && !view->opengl))
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
tests_callback(struct tableview *tv, colID cid, rowID rid)
{
	static char text[32];
	struct tableview_row *row = tableview_row_get(tv, rid);
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
	struct window *win;
	struct tableview *tv;
	int i;
	struct button *btn;
	struct box *bo;
	int id = 0;

	win = window_new(0, "tests");
	window_set_caption(win, "Agar Test Suite");

	tv = tableview_new(win, 0, tests_callback, NULL);
	tableview_set_update(tv, 250);
	tableview_col_add(tv, TABLEVIEW_COL_DYNAMIC|TABLEVIEW_COL_UPDATE,
			      0, "Test",
			      "< VIEW_PUT_PIXEL_2_CLIPPED() -- Clipped >");
	tableview_col_add(tv, TABLEVIEW_COL_DYNAMIC|TABLEVIEW_COL_UPDATE,
			      1, "Runs", "<XX>");
	tableview_col_add(tv, TABLEVIEW_COL_DYNAMIC|TABLEVIEW_COL_UPDATE,
			      2, "Iterations", "<XXXXXXXXX>");
	tableview_col_add(tv, TABLEVIEW_COL_DYNAMIC|TABLEVIEW_COL_UPDATE,
			      3, "Result", NULL);

	for (i = 0; i < ntests; i++) {
		struct test_ops *test = tests[i];
		struct tableview_row *pRow;
		int j;

		pRow = tableview_row_add(tv, TABLEVIEW_STATIC_ROW,
		    NULL, NULL, id++,
		    0, test->name,
		    1, "",
		    2, "",
		    3, "");

		for (j = 0; j < test->nfuncs; j++) {
			struct testfn_ops *func = &test->funcs[j];

			tableview_row_add(tv, 0, pRow, func, id++,
			    0, func->name,
			    1, "",
			    2, "",
			    3, "");
		}
	}

	bo = box_new(win, BOX_HORIZ, BOX_WFILL|BOX_HOMOGENOUS);
	{
		btn = button_new(bo, "Run tests");
		event_new(btn, "button-pushed", run_tests, "%p", tv);

		btn = button_new(bo, "Parameters");
		event_new(btn, "button-pushed", test_params, "%p", tv);
	}

	window_show(win);
	window_set_geometry(win,
	    10, 10,
	    view->w - 20, view->h - 20);
}

int
main(int argc, char *argv[])
{
	int c, i;
	char *s;

	if (engine_preinit("agar-tests") == -1) {
		fprintf(stderr, "%s\n", error_get());
		return (1);
	}

	engine_set_gfxmode(GFX_ENGINE_GUI);

	while ((c = getopt(argc, argv, "?vfFegGw:h:t:r:T:")) != -1) {
		extern char *optarg;

		switch (c) {
		case 'v':
			exit(0);
		case 'f':
			prop_set_bool(config, "view.full-screen", 1);
			break;
		case 'F':
			prop_set_bool(config, "view.full-screen", 0);
			break;
#ifdef HAVE_OPENGL
		case 'g':
			prop_set_bool(config, "view.opengl", 1);
			break;
		case 'G':
			prop_set_bool(config, "view.opengl", 0);
			break;
#endif
		case 'w':
			prop_set_uint16(config, "view.w", atoi(optarg));
			break;
		case 'h':
			prop_set_uint16(config, "view.h", atoi(optarg));
			break;
		case 't':
			text_parse_fontspec(optarg);
			break;
		case 'T':
			prop_set_string(config, "font-path", "%s", optarg);
			break;
		case 'r':
			view_parse_fpsspec(optarg);
			break;
		case '?':
		default:
			printf("%s [-vfF] [-w width] [-h height] [-r fps]"
			       " [-t font,size,flags] [-T font-path]",
			    progname);
#ifdef HAVE_OPENGL
			printf(" [-gG]");
#endif
			printf("\n");
			exit(0);
		}
	}

	if (engine_init() == -1) {
		fprintf(stderr, "%s\n", error_get());
		return (-1);
	}

	tests_window();

	event_loop();
	engine_destroy();
	return (0);
fail:
	engine_destroy();
	return (1);
}

