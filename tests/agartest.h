/*	Public domain	*/

#ifndef _AGARTEST_H_
#define _AGARTEST_H_

#include <agar/core.h>
#include <agar/gui.h>

#include <agar/config/ag_debug.h>
#include <agar/config/have_opengl.h>
#include <agar/config/enable_au.h>
#include <agar/config/enable_vg.h>
#include <agar/config/enable_rg.h>

/* Test case definition */
typedef struct ag_test_case {
	const char *name;			/* Test name */
	const char *descr;			/* Short description */
	const char *minVer;			/* Minimum Agar version */
	Uint flags;
#define AG_TEST_THREADS	0x01			/* Require multithreading */
#define AG_TEST_OPENGL	0x02			/* Require OpenGL support */
#define AG_TEST_SDL	0x04			/* Require SDL 1.x support */
#define AG_TEST_AUDIO	0x08			/* Require the Agar-AU library */
	size_t size;				/* Instance structure size */
	int  (*init)(void *);			/* Initialize instance */
	void (*destroy)(void *);		/* Free instance */
	int  (*test)(void *);			/* Non-interactive test */
	int  (*testGUI)(void *, AG_Window *);	/* Interactive test */
} AG_TestCase;

/* Test case instance */
typedef struct ag_test_instance {
	const AG_TestCase *tc;
	const char *name;
	Uint flags;
	float score;				/* Numerical result */
	AG_Console *console;			/* Output console */
	AG_Window *win;				/* Main (control) window */
	AG_TAILQ_ENTRY(ag_test_instance) instances;
} AG_TestInstance;

void       TestWindowClose(AG_Event *);
void       TestMsg(void *, const char *, ...);

#include "config/enable_nls.h"
#ifdef ENABLE_NLS
# include <libintl.h>
# define _(String) gettext(String)
# define gettext_noop(String) (String)
# define N_(String) gettext_noop(String)
#else
# undef _
# undef N_
# define _(s) (s)
# define N_(s) (s)
#endif

#endif /* _AGARTEST_H_ */
