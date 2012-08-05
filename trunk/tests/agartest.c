/*	Public domain	*/
/*
 * Execute the Agar test suite.
 */

#include "agartest.h"

#include <agar/dev.h>

#include <string.h>

#include <agar/config/have_opengl.h>
#include "config/have_agar_au.h"
#include "config/datadir.h"

extern const AG_TestCase audioTest;
extern const AG_TestCase charsetsTest;
extern const AG_TestCase chineseTest;
extern const AG_TestCase compositingTest;
extern const AG_TestCase configSettingsTest;
extern const AG_TestCase consoleTest;
extern const AG_TestCase customWidgetTest;
extern const AG_TestCase fixedResTest;
extern const AG_TestCase focusingTest;
extern const AG_TestCase glviewTest;
extern const AG_TestCase imageLoadingTest;
extern const AG_TestCase keyEventsTest;
extern const AG_TestCase loaderTest;
extern const AG_TestCase maximizedTest;
extern const AG_TestCase minimalTest;
extern const AG_TestCase modalWindowHandlerTest;
extern const AG_TestCase networkTest;
extern const AG_TestCase objSystemTest;
extern const AG_TestCase paneTest;
extern const AG_TestCase plottingTest;
extern const AG_TestCase renderToSurfaceTest;
extern const AG_TestCase scrollbarTest;
extern const AG_TestCase scrollviewTest;
extern const AG_TestCase socketsTest;
extern const AG_TestCase tableTest;
extern const AG_TestCase textboxTest;
extern const AG_TestCase textDlgTest;
extern const AG_TestCase themesTest;
extern const AG_TestCase threadsTest;
extern const AG_TestCase timeoutsTest;
extern const AG_TestCase unitconvTest;
extern const AG_TestCase windowsTest;

const AG_TestCase *testCases[] = {
#ifdef HAVE_AGAR_AU
	&audioTest,
#endif
	&charsetsTest,
	&chineseTest,
	&compositingTest,
	&configSettingsTest,
	&consoleTest,
	&customWidgetTest,
	&fixedResTest,
	&focusingTest,
#ifdef HAVE_OPENGL
	&glviewTest,
#endif
	&imageLoadingTest,
	&keyEventsTest,
	&loaderTest,
	&maximizedTest,
	&minimalTest,
	&modalWindowHandlerTest,
	&networkTest,
	&objSystemTest,
	&paneTest,
	&plottingTest,
	&renderToSurfaceTest,
	&scrollbarTest,
	&scrollviewTest,
	&socketsTest,
	&tableTest,
	&textboxTest,
	&textDlgTest,
	&themesTest,
	&threadsTest,
	&timeoutsTest,
	&unitconvTest,
	&windowsTest,
	NULL
};

TAILQ_HEAD_(ag_test_instance) tests;		/* Running tests */
AG_Statusbar *statusBar;
AG_Label *status;

static void
SelectedTest(AG_Event *event)
{
	AG_TlistItem *it = AG_PTR(1);
	AG_TestCase *tc = it->p1;

	AG_LabelText(status, "%s: %s", tc->name, tc->descr);
}

static void
RunTest(AG_Event *event)
{
	AG_Tlist *tl = AG_PTR(1);
	AG_Console *cons = AG_PTR(2);
	AG_TestCase *tc = AG_TlistSelectedItemPtr(tl);
	AG_TestInstance *ti;
	
	if ((ti = TryMalloc(tc->size)) == NULL) {
		AG_LabelTextS(status, AG_GetError());
		return;
	}
	ti->name = tc->name;
	ti->tc = tc;
	ti->flags = 0;
	ti->score = 1.0;
	ti->console = cons;
	ti->win = NULL;

	if (tc->init != NULL &&
	    tc->init(ti) == -1) {
		AG_ConsoleMsg(cons, _("%s: Failed: %s"), tc->name, AG_GetError());
		AG_LabelTextS(status, AG_GetError());
		goto fail;
	}
	if (tc->test != NULL) {
		AG_ConsoleMsg(cons, _("Running test: %s..."), tc->name);
		ti->score = 100.0;
		if (tc->test(ti) == 0) {
			if (ti->score != 100.0) {
				AG_ConsoleMsg(cons, _("%s: Success (%f%%)"),
				    tc->name, ti->score);
			} else {
				AG_ConsoleMsg(cons, _("%s: Success"), tc->name);
			}
		} else {
			AG_ConsoleMsg(cons, _("%s: Failed (%s)"), tc->name,
			    AG_GetError());
			AG_LabelTextS(status, AG_GetError());
			goto fail;
		}
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
			AG_ConsoleMsg(cons, _("%s: Interactive test started"),
			    tc->name);
			AG_WindowShow(win);
		} else {
			AG_ConsoleMsg(cons, _("%s: Failed to start (%s)"),
			    tc->name, AG_GetError());
			AG_ObjectDetach(win);
			goto fail;
		}
	}
	TAILQ_INSERT_TAIL(&tests, ti, instances);
	return;
fail:
	Free(ti);
}

/* Close an interactive test. */
void
TestWindowClose(AG_Event *event)
{
	AG_TestInstance *ti = AG_PTR(1);
	
	AG_ConsoleMsg(ti->console, _("Test %s: terminated"), ti->name);
	AG_ObjectDetach(ti->win);
	TAILQ_REMOVE(&tests, ti, instances);
	free(ti);
}

/* Write a message to the test console. */
void
TestMsg(void *obj, const char *fmt, ...)
{
	AG_TestInstance *ti = obj;
	va_list args;
	char *s;

	va_start(args, fmt);
	if (TryVasprintf(&s, fmt, args) == -1) {
		return;
	}
	va_end(args);
	AG_ConsoleMsg(ti->console, "%s: %s", ti->name, s);
	free(s);
}

#ifdef AG_DEBUG
static void
StartDebugger(void)
{
	AG_Window *win;

	if ((win = AG_GuiDebugger(agWindowFocused)) != NULL)
		AG_WindowShow(win);
}
#endif

int
main(int argc, char *argv[])
{
	char *driverSpec = NULL, *optArg;
	AG_Window *win;
	AG_Tlist *tl;
	const AG_TestCase **pTest;
	AG_Console *cons;
	AG_Pane *pane;
	AG_Button *btn;
	int c;

	TAILQ_INIT(&tests);

	while ((c = AG_Getopt(argc, argv, "?hd:", &optArg, NULL)) != -1) {
		switch (c) {
		case 'd':
			driverSpec = optArg;
			break;
		case '?':
		case 'h':
		default:
			printf("Usage: agartest [-d agar-driver] [test1 test2 ...]\n");
			return (1);
		}
	}
	if (AG_InitCore("agartest", AG_VERBOSE) == -1 ||
	    AG_InitGraphics(driverSpec) == -1) {
		printf("Agar initialization failed: %s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);
#ifdef AG_DEBUG
	AG_BindGlobalKey(AG_KEY_F12, AG_KEYMOD_ANY, StartDebugger);
#endif

	if (strcmp(DATADIR, "NONE") != 0) {
		AG_PrtString(agConfig, "load-path", "%s:.", DATADIR);
	} else {
		AG_SetString(agConfig, "load-path", ".");
	}
	(void)AG_ConfigLoad();

	if ((win = AG_WindowNew(0)) == NULL) {
		return (1);
	}
	AG_WindowSetCaptionS(win, "agartest");

	pane = AG_PaneNewVert(win, AG_PANE_EXPAND);
	AG_PaneMoveDividerPct(pane, 60);

	AG_LabelNewS(pane->div[0], 0, _("Available tests: "));
	tl = AG_TlistNew(pane->div[0], AG_TLIST_EXPAND);
	AG_TlistSizeHint(tl, "XXXXXXXXXXXXXXXXXX", 5);
	for (pTest = &testCases[0]; *pTest != NULL; pTest++) {
		AG_TlistAddPtr(tl, NULL, (*pTest)->name, (void *)*pTest);
	}
	btn = AG_ButtonNew(pane->div[0], AG_BUTTON_HFILL, _("Run test"));
	cons = AG_ConsoleNew(pane->div[1], AG_CONSOLE_EXPAND);
	
	AG_TlistSetChangedFn(tl, SelectedTest, NULL);
	AG_SetEvent(btn, "button-pushed", RunTest, "%p,%p", tl, cons);

	statusBar = AG_StatusbarNew(win, AG_STATUSBAR_HFILL);
	status = AG_StatusbarAddLabel(statusBar, AG_LABEL_STATIC,
	    _("Please select a test"));

	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 520, 440);
	AG_WindowShow(win);

	AG_EventLoop();
	AG_Destroy();
	return (0);
}
