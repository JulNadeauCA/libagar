/*	$Csoft: agar-bench.c,v 1.2 2005/10/03 06:19:51 vedge Exp $	*/
/*	Public domain	*/

#include "agar-bench.h"
#include <engine/config.h>

#include <string.h>
#include <unistd.h>

extern struct test_ops pixelops_test;
extern struct test_ops primitives_test;
extern struct test_ops surfaceops_test;

struct test_ops *tests[] = {
	&pixelops_test,
	&primitives_test,
	&surfaceops_test
};
int ntests = sizeof(tests) / sizeof(tests[0]);

#if defined(i386) && defined(HAVE_RDTSC)
#define USE_RDTSC
#define RDTSC(t) asm __volatile__ (".byte 0x0f, 0x31; " : "=A" (t))
#endif

static void
run_tests(int argc, union evarg *argv)
{
	struct test_ops *test = argv[1].p;
	AG_Table *t = argv[2].p;
	u_int i, j, m;
	Uint64 t1, t2;
	Uint64 tTot;
	
	if ((test->flags & TEST_GL)  && !agView->opengl) {
		AG_TextMsg(AG_MSG_ERROR, "This test requires OpenGL mode.");
		return;
	}
	if ((test->flags & TEST_SDL) && agView->opengl) {
		AG_TextMsg(AG_MSG_ERROR,
		    "This test requires SDL direct video mode.");
		return;
	}

	for (m = 0; m < t->m; m++) {
		struct testfn_ops *ops = t->cells[m][2].data.p;

		if (!AG_TableRowSelected(t, m)) {
			continue;
		}
		fprintf(stderr, "Running test: %s...", ops->name);
		if (ops->init != NULL) ops->init();
		for (i = 0, tTot = 0; i < test->runs; i++) {
#ifdef USE_RDTSC
			RDTSC(t1);
#else
			t1 = SDL_GetTicks();
#endif
			for (j = 0; j < test->iterations; j++) {
				ops->run();
			}
#ifdef USE_RDTSC
			RDTSC(t2);
#else
			t2 = SDL_GetTicks();
#endif
			fprintf(stderr, " %llu", (t2 - t1));
			tTot += (t2 - t1);
		}
		fprintf(stderr, ".\n");
		if (ops->destroy != NULL) ops->destroy();
#ifdef USE_RDTSC
		ops->clks = (Uint64)(tTot / test->iterations / test->runs);
#else
		ops->clks = (Uint64)(tTot / test->runs);
#endif
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

#ifdef USE_RDTSC
		if (fn->clks >= 1e6) {
			AG_TableAddRow(t, "%s:%.04f MClks:%p", fn->name,
			    (double)(fn->clks/1e6), fn);
		} else if (fn->clks >= 1e3) {
			AG_TableAddRow(t, "%s:%.04f kClks:%p", fn->name,
			    (double)(fn->clks/1e3), fn);
		} else {
			AG_TableAddRow(t, "%s:%lu Clks:%p", fn->name,
			    (u_long)fn->clks, fn);
		}
#else
		AG_TableAddRow(t, "%s:%lu Ticks:%p", fn->name,
		    (u_long)fn->clks, fn);
#endif
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
	int i, j;

	win = AG_WindowNew(0, "agar-benchmarks");
	AG_WindowSetCaption(win, "Agar Benchmarks");

	nb = AG_NotebookNew(win, AG_NOTEBOOK_WFILL|AG_NOTEBOOK_HFILL);
	for (i = 0; i < ntests; i++) {
		struct test_ops *test = tests[i];

		ntab = AG_NotebookAddTab(nb, test->name, AG_BOX_VERT);
		t = AG_TablePolled(ntab, AG_TABLE_MULTI, poll_test, "%i", i);
		AG_TableAddCol(t, "Test", NULL, NULL);
		AG_TableAddCol(t, "Result", "<88888888888 MClks>", NULL);
		AG_TableAddCol(t, NULL, NULL, NULL);
		
		btn = AG_ButtonNew(ntab, "Run tests");
		AG_SetEvent(btn, "button-pushed", run_tests, "%p,%p", test, t);
		AGWIDGET(btn)->flags |= AG_WIDGET_WFILL;
	
		for (j = 0; j < test->nfuncs; j++) {
			struct testfn_ops *fn = &test->funcs[j];

			fn->clks = 0;
		}
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

