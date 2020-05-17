/*
 * Copyright (c) 2006-2009 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <agar/config/enable_gui.h>
#ifdef ENABLE_GUI

/*
 * Utility GUI routines for types defined by the math library.
 */

#include <agar/core/core.h>
#include <agar/gui/widget.h>
#include <agar/gui/box.h>
#include <agar/gui/numerical.h>
#include <agar/math/m.h>
#include <agar/math/m_gui.h>

/* Edit a 3-vector matrix translation. */
void *
M_EditTranslate3(void *parent, const char *label, M_Matrix44 *T)
{
	AG_Box *box;
	AG_Numerical *n;
	int i;

	box = AG_BoxNew(parent, AG_BOX_VERT, AG_BOX_HFILL);
	if (label != NULL) {
		AG_LabelNewS(box, 0, label);
	}
	for (i = 0; i < 3; i++) {
#ifdef HAVE_SSE
		n = AG_NumericalNewFlt(box, 0, NULL, NULL, &T->m[i][3]);
		AG_SetFloat(n, "inc", 0.5f);
#else
		n = M_NumericalNewReal(box, 0, NULL, NULL, &T->m[i][3]);
		M_SetReal(n, "inc", 0.5);
#endif
	}
	return (box);
}

# ifdef AG_THREADS
/* Edit a 3-vector matrix translation (mutex-bound). */
void *
M_EditTranslate3Mp(void *parent, const char *label, M_Matrix44 *T,
    AG_Mutex *lock)
{
	AG_Box *box;
	AG_Numerical *n;
	int i;

	box = AG_BoxNew(parent, AG_BOX_VERT, AG_BOX_HFILL);
	if (label != NULL) {
		AG_LabelNewS(box, 0, label);
	}
	for (i = 0; i < 3; i++) {
		n = AG_NumericalNewS(box, 0, NULL, NULL);
#ifdef HAVE_SSE
		AG_BindFloatMp(n, "value", &T->m[i][3], lock);
		AG_SetFloat(n, "inc", 0.5f);
#else
		M_BindRealMp(n, "value", &T->m[i][3], lock);
		M_SetReal(n, "inc", 0.5);
#endif
	}
	return (box);
}
# endif /* AG_THREADS */

#endif /* ENABLE_GUI */
