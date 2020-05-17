/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
 */

#include <agar/core/core.h>
#include <agar/math/m.h>

#define SwapCode(TYPE, parmi, parmj, n) { 		\
	long i = (n) / sizeof (TYPE); 			\
	TYPE *pi = (TYPE *) (parmi); 			\
	TYPE *pj = (TYPE *) (parmj); 			\
	do { 						\
		TYPE	t = *pi;			\
		*pi++ = *pj;				\
		*pj++ = t;				\
        } while (--i > 0);				\
}

static __inline__ void
SwapFunc(char *_Nonnull a, char *_Nonnull b, int n, int swaptype)
{
	if (swaptype <= 1) 
		SwapCode(long, a, b, n)
	else
		SwapCode(char, a, b, n)
}

#define Swap(a, b)					\
	if (swaptype == 0) {				\
		long t = *(long *)(a);			\
		*(long *)(a) = *(long *)(b);		\
		*(long *)(b) = t;			\
	} else						\
		SwapFunc(a, b, es, swaptype)

#define VecSwap(a, b, n) 	if ((n) > 0) SwapFunc(a, b, n, swaptype)

static __inline__ char *
Median3(char *_Nonnull a, char *_Nonnull b, char *_Nonnull c,
    M_Real (*_Nonnull cmp)(const void *_Nonnull, const void *_Nonnull))
{
	return (cmp(a, b) < -M_MACHEP) ?
	       (cmp(b, c) < -M_MACHEP ? b : (cmp(a, c) < -M_MACHEP ? c : a )) :
               (cmp(b, c) > +M_MACHEP ? b : (cmp(a, c) < -M_MACHEP ? a : c ));
}

void
M_QSort(void *aa, AG_Size n, AG_Size es,
    M_Real (*cmp)(const void *, const void *))
{
	char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
	int i, d, swaptype, swap_cnt;
	char *a = aa;
	M_Real r;

loop:
	swaptype = ((char *)a - (char *)0) % sizeof(long) ||
	           es % sizeof(long) ? 2 : es == sizeof(long) ? 0 : 1;

	swap_cnt = 0;
	if (n < 7) {
		for (pm = &a[es];
		     pm < &a[n*es];
		     pm += es) {
			for (pl = pm;
			     pl > &a[0] && (cmp(pl-es, pl) > +M_MACHEP);
			     pl -= es) {
				Swap(pl, pl - es);
			}
		}
		return;
	}
	pm = &a[(n/2)*es];
	if (n > 7) {
		pl = &a[0];
		pn = &a[(n-1)*es];
		if (n > 40) {
			d = (n / 8) * es;
			pl = Median3(pl,     pl+d, pl+2*d, cmp);
			pm = Median3(pm-d,   pm,   pm+d,   cmp);
			pn = Median3(pn-2*d, pn-d, pn,     cmp);
		}
		pm = Median3(pl, pm, pn, cmp);
	}
	Swap(a, pm);
	pa = pb = &a[es];
	pc = pd = &a[(n-1)*es];
	for (;;) {
		while (pb <= pc && (r = cmp(pb, a)) <= +M_MACHEP) {
			if (M_MACHZERO(r)) {
				swap_cnt = 1;
				Swap(pa, pb);
				pa += es;
			}
			pb += es;
		}
		while (pb <= pc && (r = cmp(pc, a)) >= -M_MACHEP) {
			if (M_MACHZERO(r)) {
				swap_cnt = 1;
				Swap(pc, pd);
				pd -= es;
			}
			pc -= es;
		}
		if (pb > pc) {
			break;
		}
		Swap(pb, pc);
		swap_cnt = 1;
		pb += es;
		pc -= es;
	}

	/* Switch to insertion sort */
	if (swap_cnt == 0) {
		for (pm = &a[es]; pm < &a[n*es]; pm += es) {
			for (pl = pm;
			     pl > &a[0] && (cmp(pl-es, pl) > M_MACHEP);
			     pl -= es) {
				Swap(pl, pl - es);
			}
		}
		return;
	}

	pn = &a[n*es];
	i = MIN(pa-&a[0], pb-pa);
	VecSwap(a, pb-i, i);
	i = MIN(pd-pc, pn-pd-es);
	VecSwap(pb, pn-i, i);
	if ((i = pb - pa) > es) {
		M_QSort(a, i/es, es, cmp);
	}
	if ((i = pd - pc) > es) { 
		/* Iterate rather than recurse to save stack space */
		a = pn-i;
		n = i/es;
		goto loop;
	}
}
