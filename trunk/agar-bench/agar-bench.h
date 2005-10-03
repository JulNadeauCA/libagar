/*	$Csoft: test.h,v 1.2 2005/10/02 16:06:31 vedge Exp $	*/
/*	Public domain	*/

#include <engine/engine.h>
#include <engine/view.h>
#include <engine/map/map.h>
#include <engine/widget/gui.h>

struct testfn_ops {
	char *name;
	u_int runs;
	u_int iterations;
	int flags;
#define TEST_SDL	0x01		/* SDL-only */
#define TEST_GL		0x02		/* OpenGL-only */

	void (*init)(void);
	void (*destroy)(void);
	void (*run)(void);
	Uint64 clks;
};

struct test_ops {
	char *name;
	void (*edit)(AG_Window *);
	struct testfn_ops *funcs;
	u_int nfuncs;
};

extern SDL_Surface *surface;

void InitSurface(void);
void FreeSurface(void);
void LockView(void);
void UnlockView(void);

