/*	Public domain	*/

#ifndef _AGARTEST_H_
#define _AGARTEST_H_

#include <agar/core.h>
#include <agar/gui.h>

#include <agar/config/ag_debug.h>
#include <agar/config/have_64bit.h>
#include <agar/config/have_opengl.h>

/* Test case definition */
typedef struct ag_test_case {
	const char *_Nonnull name;		/* Test name */
	const char *_Nonnull descr;		/* Short description */
	const char *_Nullable minVer;		/* Minimum Agar version */
	Uint flags;
#define AG_TEST_THREADS	0x01			/* Require multithreading */
#define AG_TEST_OPENGL	0x02			/* Require OpenGL support */
#define AG_TEST_SDL	0x04			/* Require SDL 1.x support */
#define AG_TEST_AUDIO	0x08			/* Require the Agar-AU library */
	AG_Size size;				/* Instance structure size */
	int  (*_Nullable init)(void *_Nonnull);		/* Initialize test */
	void (*_Nullable destroy)(void *_Nonnull);	/* Free test instance */
	int  (*_Nullable test)(void *_Nonnull);		/* Non-interactive test */
	int  (*_Nullable testGUI)(void *_Nonnull,
	                          AG_Window *_Nonnull);	/* Interactive test */
	int  (*_Nullable bench)(void *_Nonnull);	/* Run benchmark */
} AG_TestCase;

/* Test case instance */
typedef struct ag_test_instance {
	const AG_TestCase *_Nonnull tc;
	const char *_Nonnull name;
	Uint flags;
	float score;				/* Numerical result */
	AG_Console *_Nullable console;		/* Output console */
	AG_Window *_Nullable win;		/* Main (control) window */
	AG_Button *_Nullable closeBtn;		/* "Close this test" */
	AG_TAILQ_ENTRY(ag_test_instance) instances;
} AG_TestInstance;

typedef struct ag_benchmark_fn {
	char *_Nonnull name;
	void (*_Nonnull run)(void *_Nonnull ti);
#if defined(AG_HAVE_64BIT)
	Uint64 clksMin, clksAvg, clksMax;
#else
	Uint32 clksMin, clksAvg, clksMax;
#endif
} AG_BenchmarkFn;

typedef struct ag_benchmark {
	char *_Nonnull name;		/* Benchmark display name */

	AG_BenchmarkFn *_Nonnull funcs;	/* Subroutine list */
	Uint                    nFuncs;

	Uint runs;			/* Number of loop cycles */
	Uint iterations;		/* Iterations in loop */
	Uint maximum;			/* If tests exceed value, assume
					   preemption and retry (0=disable) */
} AG_Benchmark;

AG_ConsoleLine *_Nonnull TestMsg(void *_Nonnull, const char *_Nonnull, ...);
AG_ConsoleLine *_Nonnull TestMsgS(void *_Nonnull, const char *_Nonnull);

void TestExecBenchmark(void *_Nonnull, AG_Benchmark *_Nonnull);
void TestWindowClose(AG_Event *_Nonnull);

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
