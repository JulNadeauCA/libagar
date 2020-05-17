/*	Public domain	*/

/*
 * Agar's interactive test suite.
 */

#include "agartest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <agar/config/ag_unicode.h>
#include <agar/config/have_opengl.h>

#include <agar/core/agsi.h>

#include "config/have_agar_au.h"
#include "config/have_agar_math.h"
#include "config/datadir.h"

extern const AG_TestCase buttonsTest;
extern const AG_TestCase checkboxTest;
extern const AG_TestCase configsettingsTest;
extern const AG_TestCase consoleTest;
extern const AG_TestCase customwidgetTest;
extern const AG_TestCase fixedresTest;
extern const AG_TestCase focusingTest;
extern const AG_TestCase fontsTest;
extern const AG_TestCase fspathsTest;
extern const AG_TestCase imageloadingTest;
extern const AG_TestCase keyeventsTest;
extern const AG_TestCase loaderTest;
extern const AG_TestCase maximizedTest;
extern const AG_TestCase minimalTest;
extern const AG_TestCase paletteTest;
extern const AG_TestCase paneTest;
extern const AG_TestCase radioTest;
extern const AG_TestCase rendertosurfaceTest;
extern const AG_TestCase scrollbarTest;
extern const AG_TestCase scrollviewTest;
extern const AG_TestCase socketsTest;
extern const AG_TestCase tableTest;
extern const AG_TestCase textboxTest;
extern const AG_TestCase textdlgTest;
extern const AG_TestCase threadsTest;
extern const AG_TestCase unitconvTest;
extern const AG_TestCase widgetsTest;
extern const AG_TestCase windowsTest;
#if defined(AG_TIMERS) && defined(AG_HAVE_FLOAT)
extern const AG_TestCase compositingTest;
#endif
#ifdef AG_UNICODE
extern const AG_TestCase charsetsTest;
#endif
#if defined(HAVE_AGAR_AU) && !defined(_WIN32)
extern const AG_TestCase audioTest;
#endif
#ifdef HAVE_OPENGL
extern const AG_TestCase glviewTest;
#endif
#ifdef AG_TIMERS
extern const AG_TestCase objsystemTest;
extern const AG_TestCase timeoutsTest;
#endif
#ifdef AG_USER
extern const AG_TestCase userTest;
#endif
#ifdef HAVE_AGAR_MATH
extern const AG_TestCase bezierTest;
extern const AG_TestCase mathTest;
extern const AG_TestCase plottingTest;
extern const AG_TestCase stringTest;
#endif

const AG_TestCase *testCases[] = {
	&buttonsTest,
	&checkboxTest,
	&configsettingsTest,
	&consoleTest,
	&customwidgetTest,
	&fixedresTest,
	&focusingTest,
	&fontsTest,
	&fspathsTest,
	&imageloadingTest,
	&keyeventsTest,
	&loaderTest,
	&maximizedTest,
	&minimalTest,
	&paletteTest,
	&paneTest,
	&radioTest,
	&rendertosurfaceTest,
	&scrollbarTest,
	&scrollviewTest,
	&socketsTest,
	&tableTest,
	&textboxTest,
	&textdlgTest,
	&threadsTest,
	&unitconvTest,
	&widgetsTest,
	&windowsTest,
#if defined(HAVE_AGAR_AU) && !defined(_WIN32)
	&audioTest,
#endif
#if defined(AG_TIMERS) && defined(AG_HAVE_FLOAT)
	&compositingTest,
#endif
#ifdef AG_UNICODE
	&charsetsTest,
#endif
#ifdef HAVE_OPENGL
	&glviewTest,
#endif
#ifdef AG_TIMERS
	&objsystemTest,
	&timeoutsTest,
#endif
#ifdef AG_USER
	&userTest,
#endif
#ifdef HAVE_AGAR_MATH
	&bezierTest,
	&mathTest,
	&plottingTest,
	&stringTest,
#endif
	NULL
};

const char *agarBuildOpts[] = {
#ifdef AG_DEBUG
	"DEBUG  ",
#endif
#ifdef AG_ENABLE_DSO
	"DSO  ",
#endif
#ifdef AG_LEGACY
	"LEGACY  ",
#endif
#ifdef AG_NAMESPACES
	"NAMESPACES  ",
#endif
#ifdef AG_SERIALIZATION
	"SERIALIZATION  ",
#endif
#ifdef AG_THREADS
	"THREADS  ",
#endif
#ifdef AG_TIMERS
	"TIMERS  ",
#endif
#ifdef AG_TYPE_SAFETY
	"TYPE_SAFETY  ",
#endif
#ifdef AG_UNICODE
	"UNICODE  ",
#endif
#ifdef AG_USER
	"USER  ",
#endif
#ifdef AG_VERBOSITY
	"VERBOSITY  ",
#endif
	NULL
};

TAILQ_HEAD_(ag_test_instance) tests;		/* Running tests */
AG_Window *winMain;				/* Main window */
AG_Statusbar *statusBar;
AG_Label *status;
AG_Console *console = NULL;
AG_Button *btnTest;
char consoleBuf[1024];

static void RunBench(AG_Event *);

static void
SelectedTest(AG_Event *event)
{
	AG_TlistItem *it = AG_TLIST_ITEM_PTR(1);
	AG_TestCase *tc = it->p1;

	AG_ButtonText(btnTest, _("Run %s"), tc->name);
	AG_LabelText(status, "%s: %s", tc->name, tc->descr);
	if (tc->test != NULL || tc->testGUI != NULL) {
		AG_WidgetEnable(btnTest);
	} else {
		AG_WidgetDisable(btnTest);
	}
}

#ifdef AG_THREADS
static void *
RunBenchmarks(void *arg)
{
	AG_TestInstance *ti = arg;

	if (ti->tc->bench(ti) == 0) {
		AG_ConsoleMsg(console, _("%s: Success"), ti->tc->name);
	} else {
		AG_ConsoleMsg(console, _("%s: Failed (%s)"), ti->tc->name, AG_GetError());
	}
	free(ti);
	return (NULL);
}
#endif

static AG_TestInstance *
CreateTestInstance(AG_TestCase *tc)
{
	AG_TestInstance *ti;

	if ((ti = TryMalloc(tc->size)) == NULL) {
		goto fail;
	}
	ti->name = tc->name;
	ti->tc = tc;
	ti->flags = 0;
	ti->score = 1.0;
	ti->console = console;
	ti->win = NULL;

	if (tc->init != NULL &&
	    tc->init(ti) == -1) {
		AG_ConsoleMsg(console, _("%s: Failed: %s"), tc->name,
		    AG_GetError());
		goto fail;
	}
	return (ti);
fail:
	AG_LabelTextS(status, AG_GetError());
	return (NULL);
}

static void
RequestTestClose(AG_Event *event)
{
	AG_Window *win = AG_WINDOW_PTR(1);
	
	AG_PostEvent(win, "window-close", NULL);
}

static void
RunTest(AG_Event *event)
{
	AG_Tlist *tl = AG_TLIST_PTR(1);
	AG_Driver *drv = AGWIDGET(winMain)->drv;
	AG_DriverClass *drvClass = AGDRIVER_CLASS(drv);
	AG_TestCase *tc = AG_TlistSelectedItemPtr(tl);
	AG_TestInstance *ti;

	if (tc == NULL)
		return;

	if (tc->flags & AG_TEST_OPENGL) {
		if (!(drvClass->flags & AG_DRIVER_OPENGL)) {
			AG_TextMsg(AG_MSG_ERROR,
			    _("The `%s' test requires OpenGL.\n"
			      "Current driver (%s) has no GL support"),
			      tc->name, drvClass->name);
			return;
		}
	}
	if (tc->flags & AG_TEST_SDL) {
		if (!(drvClass->flags & AG_DRIVER_SDL)) {
			AG_TextMsg(AG_MSG_ERROR,
			    _("The `%s' test requires SDL.\n"
			      "Current driver (%s) has no SDL support"),
			      tc->name, drvClass->name);
			return;
		}
	}

	if ((ti = CreateTestInstance(tc)) == NULL)
		return;

	if (tc->test != NULL) {
		AG_ConsoleMsg(console, _("Running test: %s..."), tc->name);
		ti->score = 100.0;
		if (tc->test(ti) == 0) {
			if (ti->score != 100.0) {
				AG_ConsoleMsg(console, _("%s: Success (%f%%)"),
				    tc->name, ti->score);
			} else {
				AG_ConsoleMsg(console, _("%s: Success"), tc->name);
			}
		} else {
			AG_ConsoleMsg(console, _("%s: Failed (%s)"), tc->name,
			    AG_GetError());
			AG_LabelTextS(status, AG_GetError());
			goto fail;
		}
	}
	if (tc->bench != NULL) {
		RunBench(event);
	}
	if (tc->testGUI != NULL) {
		AG_Window *win;

		if ((win = AG_WindowNew(0)) == NULL) {
			AG_LabelTextS(status, AG_GetError());
			return;
		}
		AG_WindowSetCaption(win, "agartest: %s", ti->name);
		AG_SetEvent(win, "window-close", TestWindowClose, "%p", ti);
	
		if (tc->testGUI(ti, win) == 0) {
			ti->win = win;
			AG_ConsoleMsg(console, _("%s: Interactive test started"),
			    tc->name);

			AG_SeparatorNewHoriz(win);

			ti->closeBtn = AG_ButtonNewFn(win, AG_BUTTON_HFILL, _("Close this test"),
			    RequestTestClose, "%p", win);

			AG_WindowAttach(winMain, win);
			AG_WindowShow(win);
		} else {
			AG_ConsoleMsg(console, _("%s: Failed to start (%s)"),
			    tc->name, AG_GetError());
			AG_ObjectDetach(win);
			goto fail;
		}
		TAILQ_INSERT_TAIL(&tests, ti, instances);
	} else {
		free(ti);
	}
	return;
fail:
	free(ti);
}

static void
RunBench(AG_Event *event)
{
	AG_Tlist *tl = AG_TLIST_PTR(1);
	AG_TestCase *tc = AG_TlistSelectedItemPtr(tl);
	AG_TestInstance *ti;

	if (tc == NULL || tc->bench == NULL)
		return;
	
	if ((ti = CreateTestInstance(tc)) == NULL)
		return;

	{
#ifdef AG_THREADS
		AG_Thread th;

		AG_ThreadCreate(&th, RunBenchmarks, ti);
#else
		if (tc->bench(ti) == 0) {
			AG_ConsoleMsg(console, _("%s: Success"), tc->name);
		} else {
			AG_ConsoleMsg(console, _("%s: Failed (%s)"), tc->name,
			    AG_GetError());
			AG_LabelTextS(status, AG_GetError());
		}
		free(ti);
#endif
	}
}

static void
TestWindowDetached(AG_Event *event)
{
	AG_TestInstance *ti = AG_PTR(1);
	
	TAILQ_REMOVE(&tests, ti, instances);
	if (ti->tc->destroy != NULL) {
		ti->tc->destroy(ti);
	}
	free(ti);
}

/* Close an interactive test. */
void
TestWindowClose(AG_Event *event)
{
	AG_TestInstance *ti = AG_PTR(1);
	
	AG_ConsoleMsg(console, _("Test %s: terminated"), ti->name);
	AG_SetEvent(ti->win, "window-detached", TestWindowDetached, "%p", ti);
	AG_ObjectDetach(ti->win);
}

/* Write a message to the test console (format string). */
AG_ConsoleLine *
TestMsg(void *obj, const char *fmt, ...)
{
	AG_TestInstance *ti = obj;
	AG_ConsoleLine *ln;
	va_list args;
	char *s;

	va_start(args, fmt);
	AG_Vasprintf(&s, fmt, args);
	ln = AG_ConsoleMsgS(ti->console, s);
	va_end(args);
	free(s);
	return (ln);
}

/* Write a message to the test console (C string). */
AG_ConsoleLine *
TestMsgS(void *obj, const char *s)
{
	AG_TestInstance *ti = obj;

	return AG_ConsoleMsgS(ti->console, s);
}

#if (defined(i386) || defined(__i386__) || defined(__x86_64__))
# define HAVE_RDTSC
static __inline__ Uint64
rdtsc(void)
{
	Uint64 rv;

	__asm __volatile(".byte 0x0f, 0x31" : "=A" (rv));
	return (rv);
}
#else
# undef HAVE_RDTSC
#endif

/* Execute a benchmark module (called from bench() op) */
void
TestExecBenchmark(void *obj, AG_Benchmark *bm)
{
	AG_TestInstance *ti = obj;
	Uint i, j, fIdx;
#if defined(HAVE_64BIT)
	Uint64 t1, t2;
	Uint64 tTot, tRun;
#else
	Uint32 t1, t2;
	Uint32 tTot, tRun;
#endif

	AG_RedrawOnTick(ti->console, 1);
	for (fIdx = 0; fIdx < bm->nFuncs; fIdx++) {
		char pbuf[64];
		AG_BenchmarkFn *bfn = &bm->funcs[fIdx];
		AG_ConsoleLine *cl;

		bfn->clksMax = -1;
		cl = AG_ConsoleMsg(ti->console, "\t%s: ...", bfn->name);
		if (cl == NULL) {
			continue;
		}
#ifdef HAVE_RDTSC
		if (agCPU.ext & AG_EXT_TSC) {
			for (i = 0, tTot = 0; i < bm->runs; i++) {
retry:
				t1 = rdtsc();
				for (j = 0; j < bm->iterations; j++) {
					bfn->run(ti);
				}
				t2 = rdtsc();
				tRun = (t2 - t1) / bm->iterations;
			
				Snprintf(pbuf, sizeof(pbuf),
				    "\t%s: %lu clks [%i/%i]",
				    bfn->name,
				    (Ulong)tRun, i, bm->runs);
				AG_ConsoleMsgEdit(cl, pbuf);

				if (bm->maximum > 0 && tRun > bm->maximum) {
					Snprintf(pbuf, sizeof(pbuf),
					    "\t%s: <preempted>", bfn->name);
					AG_ConsoleMsgEdit(cl, pbuf);
					goto retry;
				}
				bfn->clksMax = AG_MAX(bfn->clksMax,tRun);
				bfn->clksMin = (bfn->clksMin > 0) ?
				               AG_MIN(bfn->clksMin,tRun) :
					       tRun;
				tTot += tRun;
			}
			bfn->clksAvg = (tTot / bm->runs);
			Snprintf(pbuf, sizeof(pbuf), "\t%s: %lu clks [%i]",
			    bfn->name, (Ulong)bfn->clksAvg, bm->runs);
			AG_ConsoleMsgEdit(cl, pbuf);
		} else
#endif /* HAVE_RDTSC */
		{
			for (i = 0, tTot = 0; i < bm->runs; i++) {
				t1 = AG_GetTicks();
				for (j = 0; j < bm->iterations; j++) {
					bfn->run(ti);
				}
				t2 = AG_GetTicks();
				tRun = (t2 - t1);
				Snprintf(pbuf, sizeof(pbuf),
				    "\t%s: %lu ticks [%i/%i]",
				    bfn->name, (Ulong)tRun, i, bm->runs);
				AG_ConsoleMsgEdit(cl, pbuf);
				bfn->clksMax = AG_MAX(bfn->clksMax,tRun);
				bfn->clksMin = (bfn->clksMin > 0) ?
				               AG_MIN(bfn->clksMin,tRun) :
					       tRun;
				tTot += tRun;
			}
			Snprintf(pbuf, sizeof(pbuf), "\t%s: %lu ticks [%i]",
			    bfn->name, (Ulong)bfn->clksAvg, bm->runs);
			AG_ConsoleMsgEdit(cl, pbuf);
		}
	}
	AG_RedrawOnTick(ti->console, -1);
}

#ifdef AG_UNICODE
static void
RunUnicodeBrowser(AG_Event *event)
{
	AG_Window *win;

	if ((win = AG_DEV_UnicodeBrowser()) == NULL) {
		return;
	}
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_RIGHT, 1);
	AG_SetStyle(win, "font-size", "150%");
	AG_WindowShow(win);
}
#endif

#ifdef AG_TIMERS
static void RunDriversBrowser(AG_Event *event) { AG_DEV_Browser(&agDrivers); }
static void RunClassInfo(AG_Event *event) { AG_DEV_ClassInfo(); }
static void RunFontsInfo(AG_Event *event) { AG_DEV_FontInfo(); }

static void
RunTimerInspector(AG_Event *event)
{
	AG_Window *win;

	if ((win = AG_DEV_TimerInspector()) == NULL) {
		return;
	}
	AG_WindowAttach(winMain, win);
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 1);
	AG_SetStyle(win, "font-size", "80%");
	AG_WindowShow(win);
}

# ifdef AG_DEBUG
static void
DoDebugger(void)
{
	AG_Window *win;

	if ((win = AG_GuiDebugger(agWindowFocused)) != NULL)
		AG_WindowShow(win);
}
static void
RunDebugger(AG_Event *event)
{
	DoDebugger();
}
# endif
#endif /* AG_TIMERS */

static void
DoStyleEditor(void)
{
	AG_Window *win;

	if ((win = AG_StyleEditor(agWindowFocused)) != NULL)
		AG_WindowShow(win);
}

static void
RunStyleEditor(AG_Event *event)
{
	DoStyleEditor();
}

/* Redirect AG_Debug() and AG_Verbose() to the AG_Console. */
static int
ConsoleWrite(const char *msg)
{
	if (console == NULL) {
		return (0);
	}
	AG_Strlcat(consoleBuf, msg, sizeof(consoleBuf));
	if (strchr(msg, '\n') != NULL) {
		char *line, *pBuf = consoleBuf;

		while ((line = AG_Strsep(&pBuf, "\n")) != NULL) {
			if (line[0] == '\0') {
				continue;
			}
			AG_ConsoleMsgS(console, line);
		}
		consoleBuf[0] = '\0';
	}
	return (1);
}

static void
ConsoleWindowDetached(AG_Event *event)
{
	AG_SetVerboseCallback(NULL);
	AG_SetDebugCallback(NULL);
	console = NULL;
}

/* Show information about a test module */
static void
TestInfo(AG_Event *event)
{
	const AG_TestCase *tc = AG_CONST_PTR(1);

	AG_TextMsg(AG_MSG_INFO,
	    _("Test: " AGSI_BOLD "%s" AGSI_RST "\n"
	      "Agar >= %s required.\n"
	      AGSI_ITALIC "%s" AGSI_RST),
	    tc->name,
	    tc->minVer ? tc->minVer : "1.0",
	    _(tc->descr));
}

/* Display the source code for a test module. */
static void
TestViewSource(AG_Event *event)
{
	char path[AG_PATHNAME_MAX];
	char file[AG_FILENAME_MAX];
	const AG_TestCase *tc = AG_CONST_PTR(1);
	AG_Textbox *tb;
	AG_Window *win;
	char *s;
	FILE *f;
	AG_Size size;

	Strlcpy(file, tc->name, sizeof(file));
	Strlcat(file, ".c", sizeof(file));

	if (AG_ConfigFind(AG_CONFIG_PATH_DATA, file, path, sizeof(path)) != 0) {
		AG_TextMsgFromError();
		return;
	}
	if ((f = fopen(path, "r")) == NULL) {
		AG_TextError(_("Could not open %s"), file);
		return;
	}

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_SetStyle(win, "font-family", "courier-prime");
	tb = AG_TextboxNew(win, AG_TEXTBOX_MULTILINE | AG_TEXTBOX_EXPAND |
	                        AG_TEXTBOX_READONLY, NULL);
	fseek(f, 0, SEEK_END);
	size = (AG_Size)ftell(f);
	fseek(f, 0, SEEK_SET);
	s = Malloc(size);
	fread(s, size, 1, f);
	fclose(f);
	s[size] = '\0';

	AG_TextboxBindASCII(tb, s, size);
	AG_WindowSetGeometry(win, -1, -1, 800,480);
	AG_WindowShow(win);
}

static void
TestPopupMenu(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	const AG_TestCase *tc = AG_TlistSelectedItemPtr(tl);
	AG_PopupMenu *pm;

	if (tc == NULL) {
		return;
	}
	pm = AG_PopupNew(tl);

	AG_MenuSeparator(pm->root);

	AG_MenuAction(pm->root, _("Test Information"), agIconMagnifier.s,
	    TestInfo, "%Cp", tc);

	AG_MenuAction(pm->root, _("View Source"), agIconDoc.s,
	    TestViewSource, "%Cp", tc);
	
	AG_MenuSeparator(pm->root);

	if (tc->test || tc->testGUI) {
		AG_MenuAction(pm->root,
		    tc->testGUI ? _("Run test") : _("Run test (on console)"),
		    agIconGear.s,
		    RunTest, "%p", tl);
	}
	if (tc->bench) {
		AG_MenuAction(pm->root, _("Run benchmark"), agIconGear.s,
		    RunBench, "%p", tl);
	}

	AG_PopupShow(pm);
}

static void
ZoomIn(AG_Event *event)
{
	AG_ZoomIn();
}

static void
ZoomOut(AG_Event *event)
{
	AG_ZoomOut();
}

static void
EditGuiPrefs(AG_Event *event)
{
	AG_DEV_ConfigShow();
}

int
main(int argc, char *argv[])
{
	char *driverSpec = NULL, *fontSpec = NULL, *styleSheet = NULL, *optArg;
	AG_Window *win;
	AG_Tlist *tl;
	const AG_TestCase **pTest;
	AG_Menu *menu;
	AG_MenuItem *mi;
	AG_Pane *pane;
	AG_Box *hBox;
	int c, i, optInd;
	Uint initFlags = AG_VERBOSE;
	int noConsoleRedir=0;

	TAILQ_INIT(&tests);

	while ((c = AG_Getopt(argc, argv, "CWqd:s:t:v?hp:", &optArg, &optInd)) != -1) {
		switch (c) {
		case 'C':
			noConsoleRedir = 1;
			break;
		case 'W':
			initFlags |= AG_SOFT_TIMERS;
			break;
		case 'q':
			agDebugLvl = 0;
			initFlags &= ~(AG_VERBOSE);
			break;
		case 'd':
			driverSpec = optArg;
			break;
		case 's':
			styleSheet = optArg;
			break;
		case 't':
			fontSpec = optArg;
			break;
		case 'v':
			{
				AG_AgarVersion av;

				AG_GetVersion(&av);
				if (av.release) {
					if (av.rev > 0) {
						printf("agar %d.%d.%d (r%d, \"%s\", %s)\n",
						    av.major, av.minor, av.patch,
						    av.rev, av.release,
						    AG_MEMORY_MODEL_NAME);
					} else {
						printf("agar %d.%d.%d (\"%s\", %s)\n",
						    av.major, av.minor, av.patch,
						    av.release, AG_MEMORY_MODEL_NAME);
					}
				} else {
					printf("agar %d.%d.%d (r%d, %s)\n",
					    av.major, av.minor, av.patch,
					    av.rev,
					    AG_MEMORY_MODEL_NAME);
				}
				return (0);
			}
			break;
		case 'p':
			break;
		case '?':
		case 'h':
		default:
			printf("Usage: agartest [-bCWqv] [-d driver] [-s stylesheet] [-t font] [test1 test2 ...]\n");
			return (1);
		}
	}
	if (AG_InitCore("agartest", initFlags) == -1) {
		goto fail;
	}
#ifdef _WIN32
	AG_ConfigAddPathS(AG_CONFIG_PATH_FONTS, "..\\gui\\fonts");
	AG_ConfigAddPathS(AG_CONFIG_PATH_FONTS, "..\\..\\fonts");
#endif
	if (fontSpec != NULL) {
		AG_TextParseFontSpec(fontSpec);
	}
	if (AG_InitGraphics(driverSpec) == -1)
		goto fail;

	if (styleSheet != NULL &&
	    AG_LoadStyleSheet(NULL, styleSheet) == NULL)
		goto fail;

	/* Redirect AG_Verbose() and AG_Debug() output to the AG_Console. */
	consoleBuf[0] = '\0';
	if (!noConsoleRedir) {
		AG_SetVerboseCallback(ConsoleWrite);
		AG_SetDebugCallback(ConsoleWrite);
	}

	/*
	 * Set application-global keyboard shortcuts.
	 * Map F7 and Ctrl-Shift-D to Debugger.
	 * Map F8 and Ctrl-Shift-C to Style Editor.
	 */
	AG_BindStdGlobalKeys();
#if defined(AG_DEBUG) && defined(AG_TIMERS)
	AG_BindGlobalKey(AG_KEY_F7, AG_KEYMOD_NONE, DoDebugger);
	AG_BindGlobalKey(AG_KEY_D,  AGSI_CMD_MOD,   DoDebugger);
#endif
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_NONE, DoStyleEditor);
	AG_BindGlobalKey(AG_KEY_C,  AGSI_CMD_MOD,   DoStyleEditor);

	/* Check for data files in the agartest install directory. */
#if !defined(_WIN32)
	if (strcmp(DATADIR, "NONE") != 0)
		AG_ConfigAddPathS(AG_CONFIG_PATH_DATA, DATADIR);
#endif
	AG_ConfigAddPathS(AG_CONFIG_PATH_DATA, ".");

	(void)AG_ConfigLoad();

	if ((win = winMain = AG_WindowNew(AG_WINDOW_MAIN)) == NULL) {
		return (1);
	}
	AG_WindowSetCaptionS(win, "agartest");
	
	menu = (agDriverSw) ? AG_MenuNewGlobal(0) :
                              AG_MenuNew(win, AG_MENU_HFILL);

	mi = AG_MenuNode(menu->root, ("File"), NULL);
	{
		AG_MenuActionKb(mi, _("Quit"), agIconClose.s,
		    AG_KEY_W, AG_KEYMOD_CTRL, AGWINCLOSE(win));
	}
	mi = AG_MenuNode(menu->root, ("Edit"), NULL);
	{
		AG_MenuAction(mi, _("GUI Preferences"), agIconGear.s,
		    EditGuiPrefs, NULL);
	}
	mi = AG_MenuNode(menu->root, ("Tools"), NULL);
	{
		AG_MenuAction(mi, _("Style Editor"), NULL, RunStyleEditor, NULL);
#if defined(AG_DEBUG) && defined(AG_TIMERS)
		AG_MenuAction(mi, _("GUI Debugger"), NULL, RunDebugger, NULL);
#endif
		AG_MenuSeparator(mi);
#if defined(AG_TIMERS)
		AG_MenuAction(mi, _("Drivers"), NULL, RunDriversBrowser, NULL);
		AG_MenuAction(mi, _("Classes"), NULL, RunClassInfo, NULL);
		AG_MenuAction(mi, _("Fonts"),   NULL, RunFontsInfo, NULL);
		AG_MenuAction(mi, _("Timers"), NULL,  RunTimerInspector, NULL);
#endif
#if defined(AG_UNICODE)
		AG_MenuAction(mi, _("Unicode"), NULL, RunUnicodeBrowser, NULL);
#endif
	}
	mi = AG_MenuNode(menu->root, ("View"), NULL);
	{
		AG_MenuActionKb(mi, _("Zoom In"),
		    AG_TextRender("\xE2\x8A\x9E"), /* U+229E SQUARED PLUS */
		    AG_KEY_PLUS, AG_KEYMOD_CTRL,
		    ZoomIn, NULL);

		AG_MenuActionKb(mi, _("Zoom Out"),
		    AG_TextRender("\xE2\x8A\x9F"), /* U+229F SQUARED MINUS */
		    AG_KEY_MINUS, AG_KEYMOD_CTRL,
		    ZoomOut, NULL);
#ifdef AG_DEBUG
		AG_MenuSeparator(mi);
		AG_MenuBool(mi, _("Debug Messages"),
		    AG_TextRender("\xE2\x8B\x84"), /* U+22C4 DIAMOND OPERATOR */
		    &agDebugLvl, 0);
#endif
	}
	mi = AG_MenuNode(menu->root, ("Help"), NULL);
	{
		AG_MenuAction(mi, _("About Agar GUI"), NULL, AG_About, NULL);
	}

	pane = AG_PaneNewHoriz(win, AG_PANE_EXPAND);

	AG_LabelNewS(pane->div[0], 0, _("Available tests: "));
	tl = AG_TlistNew(pane->div[0], AG_TLIST_EXPAND);
	AG_TlistSizeHint(tl, "<XXXXXXXXXXXXXX>", 10);
	for (pTest = &testCases[0]; *pTest != NULL; pTest++) {
#if 0
		char path[AG_FILENAME_MAX];                    /* with icon */
		AG_Surface *S;

		Strlcpy(path, (*pTest)->name, sizeof(path));
		Strlcat(path, ".png", sizeof(path));
		if ((S = AG_SurfaceFromPNG(path)) != NULL) {
			AG_TlistAddPtr(tl, S, (*pTest)->name, (void *)*pTest);
			AG_SurfaceFree(S);
		} else
#endif
		{
			AG_TlistAddPtr(tl, agIconDoc.s, (*pTest)->name,
			    (void *)*pTest);
		}
	}
	AG_TlistSort(tl);

	hBox = AG_BoxNewHoriz(pane->div[0], AG_BOX_HFILL | AG_BOX_HOMOGENOUS |
	                                    AG_BOX_NO_SPACING);
	{
		btnTest = AG_ButtonNew(hBox, AG_BUTTON_EXCL, _("Run"));
		AG_SetStyle(btnTest, "font-size", "120%");
		AG_WidgetDisable(btnTest);
	}

	console = AG_ConsoleNew(pane->div[1], AG_CONSOLE_EXPAND);
/*	AG_SetStyle(console, "font-family", "courier-prime"); */
	AG_SetStyle(console, "text-color", "#ddd");
	{
		AG_AgarVersion av;
		AG_DriverClass **pd;
		AG_ConsoleLine *ln;
		const char **bopt;

		AG_GetVersion(&av);
		AG_ConsoleMsgS(console, "");
		if (av.release) {
			if (av.rev > 0) {
				ln = AG_ConsoleMsg(console,
				    _("Agar %d.%d.%d for %s (r%d, " AGSI_FRAK "%s" AGSI_RST ")"),
				    av.major, av.minor, av.patch, agCPU.arch,
				    av.rev, av.release);
			} else {
				ln = AG_ConsoleMsg(console,
				    _("Agar %d.%d.%d for %s (" AGSI_FRAK "%s" AGSI_RST ")"),
				    av.major, av.minor, av.patch, agCPU.arch,
				    av.release);
			}
		} else {
			ln = AG_ConsoleMsg(console,
			    _("Agar %d.%d.%d for %s (r%d)"),
			    av.major, av.minor, av.patch, agCPU.arch, av.rev);
		}
		AG_ColorRGB(&ln->c, 200,240,240);
#if 0
		if (av.rev > 0 && av.rev != AGAR_REVISION) {
			ln = AG_ConsoleMsg(console,
			    _("WARNING: Agartest compiled against SVN r%d, but "
			      "installed libagar is r%d.\n"),
			      AGAR_REVISION, av.rev);
			AG_ColorRGB_8(&ln->c, 255,100,100);
		}
#endif

		AG_ConsoleMsgS(console, "");
		ln = AG_ConsoleMsgS(console, "https://libAgar.org/ | https://PowerfulAgar.com/");
		AG_ColorRGB(&ln->c, 200,240,240);
		AG_ConsoleMsgS(console, "");

		ln = AG_ConsoleMsg(console,
		    _("Memory model: " AGSI_ITALIC "%s" AGSI_RST " "
		      "(%d-bit/component color & %u-bit aligned pixels)"),
		    AG_MEMORY_MODEL_NAME,
		    AG_COMPONENT_BITS, (Uint)(sizeof(AG_Pixel) << 3));
		AG_ColorFromString(&ln->c, "AntiqueWhite", NULL);

		ln = AG_ConsoleMsg(console, _("Build options: " AGSI_FONT10));
		AG_ColorFromString(&ln->c, "AntiqueWhite", NULL);
		for (bopt = &agarBuildOpts[0]; *bopt != NULL; bopt++)
			AG_ConsoleMsgCatS(ln, *bopt);
		
		AG_ConsoleMsgCatS(ln, AGSI_RST);

		AG_ConsoleMsg(console, _("Available drivers:"));
		for (pd = &agDriverList[0]; *pd != NULL; pd++) {
			AG_DriverClass *dc = *pd;

			ln = AG_ConsoleMsg(console,
			    _(" -d %5s  # %s/%s, see %s(3)"),
			    dc->name,
			    agDriverTypeNames[dc->type],
			    agDriverWmTypeNames[dc->wm],
			    AGCLASS(dc)->name);
			if (AGWIDGET(win)->drvOps == dc) {
				AG_ColorRGB_8(&ln->c, 0,255,0);
				AG_ConsoleMsgCatS(ln, _(" [current]"));
			}
		}
		AG_ConsoleMsgS(console, "");
		AG_ConsoleMsg(console,
		    _("Press " AGSI_BOLD AGSI_CMD "[-]" AGSI_RST
		       " and " AGSI_BOLD AGSI_CMD "[=]" AGSI_RST " to zoom"));
# if defined(AG_DEBUG) && defined(AG_TIMERS)
		AG_ConsoleMsg(console,
		    _("Press " AGSI_BOLD "Ctrl-Shift-D or F7" AGSI_RST " to start Debugger"));
#endif
		AG_ConsoleMsg(console,
		    _("Press " AGSI_BOLD "Ctrl-Shift-C or F8" AGSI_RST " to start Style Editor"));
		AG_ConsoleMsgS(console, "");
	}

	AG_TlistSetChangedFn(tl, SelectedTest, NULL);
	AG_TlistSetDblClickFn(tl, RunTest, "%p", tl);
	AG_TlistSetPopupFn(tl, TestPopupMenu, NULL);

	AG_SetEvent(btnTest, "button-pushed", RunTest, "%p", tl);

	statusBar = AG_StatusbarNew(win, AG_STATUSBAR_HFILL);
	status = AG_StatusbarAddLabel(statusBar, _("Please select a test"));
	
	AG_SetEvent(win, "window-detached", ConsoleWindowDetached, NULL);

	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 980, 540);
	AG_WindowShow(win);

#ifdef _WIN32
	optInd++;                                 /* Skip pathname argument */
#endif
	if (optInd == argc &&
	    AG_GetBool(agConfig,"initial-run") == 1) {
		AG_Event ev;

		AG_TlistSelectPtr(tl, (void *)&widgetsTest);
		AG_EventArgs(&ev, "%p,%p", tl, win);
		RunTest(&ev);
		AG_SetBool(agConfig,"initial-run",0);
		if (AG_ConfigSave() == -1)
			AG_Verbose("AG_ConfigSave: %s; ignoring", AG_GetError());
	}
	for (i = optInd; i < argc; i++) {
		AG_Event ev;

		for (pTest = &testCases[0]; *pTest != NULL; pTest++) {
			if (AG_Strcasecmp((*pTest)->name, argv[i]) == 0)
				break;
		}
		if (*pTest == NULL) {
			AG_ConsoleMsg(console, _("No such test: %s"), argv[i]);
			continue;
		}
		AG_TlistSelectPtr(tl, (void *)(*pTest));
		AG_EventArgs(&ev, "%p,%p", tl, win);
		RunTest(&ev);
	}
	if (AG_ConfigSave() == -1)
		AG_Verbose("ConfigSave: %s\n", AG_GetError());

	AG_EventLoop();
	AG_DestroyGraphics();
	AG_Destroy();
	return (0);
fail:
	printf("Agar initialization failed: %s\n", AG_GetError());
	if (AG_GetErrorCode() == AG_ENOENT) {
		AG_DriverClass **pd;

		printf("Available drivers include:\n");
		for (pd = &agDriverList[0]; *pd != NULL; pd++) {
			AG_DriverClass *dc = *pd;

			printf(_("    -d %5s          # %s, %s (see %s(3))\n"),
			    dc->name,
			    agDriverTypeNames[dc->type],
			    agDriverWmTypeNames[dc->wm],
			    AGCLASS(dc)->name);
		}
	}
	return (1);
}
