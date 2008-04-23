/*	$Csoft: agar-bench.h,v 1.3 2005/10/03 17:37:59 vedge Exp $	*/
/*	Public domain	*/

#include <agar/core.h>
#include <agar/gui.h>

struct testfn_ops {
	char *name;
	void (*init)(void);
	void (*destroy)(void);
	void (*run)(void);
	Uint64 clksMin, clksAvg, clksMax;
};

struct test_ops {
	char *name;
	void (*edit)(AG_Window *);
	struct testfn_ops *funcs;
	unsigned nfuncs;
	unsigned flags;
#define TEST_SDL	0x01		/* SDL-only */
#define TEST_GL		0x02		/* OpenGL-only */
	unsigned runs;			/* Number of loop cycles */
	unsigned iterations;		/* Iterations in loop */
	unsigned maximum;		/* If tests exceed value, assume
					   preemption and retry (0=disable) */
					   
};

extern AG_Surface *surface, *surface64, *surface128;

void InitSurface(void);
void FreeSurface(void);
void LockView(void);
void UnlockView(void);

