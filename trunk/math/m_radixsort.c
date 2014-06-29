/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Peter McIlroy and by Dan Bernstein at New York University, 
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
 * Radixsort routines.
 * 
 * For a description of this code, see D. McIlroy, P. McIlroy, K. Bostic,
 * "Engineering Radix Sort".
 */

#include <agar/core/core.h>
#include <agar/math/m.h>

struct sort_stack {
	const Uint8 **sa;
	int sn, si;
};

static __inline__ void InsertionSort(const Uint8 **, int, int, const Uint8 *,
                                     Uint);
static void RadixSortA(const Uint8 **, int, int, const Uint8 *, Uint);
static void RadixSortB(const Uint8 **, const Uint8 **, int, int,
                       const Uint8 *, Uint);

#define	INS_THRESHOLD	20	/* Divert to InsertionSort(). */
#define	STACK_SIZE_INIT	512	/* Default stack size. */

#define SETUP {								\
	if (tab == NULL) {						\
		tr = tr0;						\
		for (c = 0; c < endch; c++)				\
			tr0[c] = c + 1;					\
		tr0[c] = 0;						\
		for (c++; c < 256; c++)					\
			tr0[c] = c;					\
		endch = 0;						\
	} else {							\
		endch = tab[endch];					\
		tr = tab;						\
		if (endch != 0 && endch != 255) {			\
			AG_SetError("Invalid argument");		\
			return (-1);					\
		}							\
	}								\
}

/*
 * Unstable radix sort, uses only O(logN) of extra memory for a stack.
 */
int
M_RadixSort(const Uint8 **a, int n, const Uint8 *tab, Uint endch)
{
	const Uint8 *tr;
	int c;
	Uint8 tr0[256];

	SETUP;
	RadixSortA(a, n, 0, tr, endch);
	return (0);
}

/*
 * Stable radix sort, but uses N extra pointers.
 */
int
M_RadixSortStable(const Uint8 **a, int n, const Uint8 *tab, Uint endch)
{
	const Uint8 *tr, **ta;
	int c;
	Uint8 tr0[256];

	SETUP;
	if (n < INS_THRESHOLD)
		InsertionSort(a, n, 0, tr, endch);
	else {
		if ((ta = TryMalloc(n * sizeof(a))) == NULL) {
			return (-1);
		}
		RadixSortB(a, ta, n, 0, tr, endch);
		Free((Uint8 *)ta);
	}
	return (0);
}

#define empty(s)	(s >= sp)
#define pop(a, n, i)	a = (--sp)->sa, n = sp->sn, i = sp->si
#define push(a, n, i)	sp->sa = a, sp->sn = n, (sp++)->si = i
#define swap(a, b, t)	t = a, a = b, b = t

/* Unstable, in-place sort. */
static void
RadixSortA(const Uint8 **a, int n, int i, const Uint8 *tr, Uint endch)
{
	static int count[256], nc, bmin;
	int c;
	const Uint8 **ak, *r;
	struct sort_stack s[STACK_SIZE_INIT], *sp, *sp0, *sp1, temp;
	int *cp, bigc;
	const Uint8 **an, *t, **aj, **top[256];

	/* Set up stack. */
	sp = s;
	push(a, n, i);
	while (!empty(s)) {
		pop(a, n, i);
		if (n < INS_THRESHOLD) {
			InsertionSort(a, n, i, tr, endch);
			continue;
		}
		an = a + n;

		/* Make character histogram. */
		if (nc == 0) {
			bmin = 255;	/* First occupied bin, excluding eos. */
			for (ak = a; ak < an;) {
				c = tr[(*ak++)[i]];
				if (++count[c] == 1 && c != endch) {
					if (c < bmin)
						bmin = c;
					nc++;
				}
			}
			if (sp + nc > s + STACK_SIZE_INIT) {
				/* Get more stack. */
				RadixSortA(a, n, i, tr, endch);
				continue;
			}
		}

		/*
		 * Set top[]; push incompletely sorted bins onto stack.
		 * top[] = pointers to last out-of-place element in bins.
		 * count[] = counts of elements in bins.
		 * Before permuting: top[c-1] + count[c] = top[c];
		 * during deal: top[c] counts down to top[c-1].
		 */
		sp0 = sp1 = sp;		/* Stack position of biggest bin. */
		bigc = 2;		/* Size of biggest bin. */
		if (endch == 0)		/* Special case: set top[eos]. */
			top[0] = ak = a + count[0];
		else {
			ak = a;
			top[255] = an;
		}
		for (cp = count + bmin; nc > 0; cp++) {
			while (*cp == 0)	/* Find next non-empty pile. */
				cp++;
			if (*cp > 1) {
				if (*cp > bigc) {
					bigc = *cp;
					sp1 = sp;
				}
				push(ak, *cp, i+1);
			}
			top[cp-count] = ak += *cp;
			nc--;
		}
		swap(*sp0, *sp1, temp);	/* Play it safe -- biggest bin last. */

		/*
		 * Permute misplacements home.  Already home: everything
		 * before aj, and in bin[c], items from top[c] on.
		 * Inner loop:
		 *	r = next element to put in place;
		 *	ak = top[r[i]] = location to put the next element.
		 *	aj = bottom of 1st disordered bin.
		 * Outer loop:
		 *	Once the 1st disordered bin is done, ie. aj >= ak,
		 *	aj<-aj + count[c] connects the bins in a linked list;
		 *	reset count[c].
		 */
		for (aj = a; aj < an;  *aj = r, aj += count[c], count[c] = 0)
			for (r = *aj;  aj < (ak = --top[c = tr[r[i]]]);)
				swap(*ak, r, t);
	}
}

/* Stable sort, requiring additional memory. */
static void
RadixSortB(const Uint8 **a, const Uint8 **ta, int n, int i, const Uint8 *tr,
    Uint endch)
{
	static int count[256], nc, bmin;
	int c;
	const Uint8 **ak, **ai;
	struct sort_stack s[512], *sp, *sp0, *sp1, temp;
	const Uint8 **top[256];
	int *cp, bigc;

	sp = s;
	push(a, n, i);
	while (!empty(s)) {
		pop(a, n, i);
		if (n < INS_THRESHOLD) {
			InsertionSort(a, n, i, tr, endch);
			continue;
		}

		if (nc == 0) {
			bmin = 255;
			for (ak = a + n; --ak >= a;) {
				c = tr[(*ak)[i]];
				if (++count[c] == 1 && c != endch) {
					if (c < bmin)
						bmin = c;
					nc++;
				}
			}
			if (sp + nc > s + STACK_SIZE_INIT) {
				RadixSortB(a, ta, n, i, tr, endch);
				continue;
			}
		}

		sp0 = sp1 = sp;
		bigc = 2;
		if (endch == 0) {
			top[0] = ak = a + count[0];
			count[0] = 0;
		} else {
			ak = a;
			top[255] = a + n;
			count[255] = 0;
		}
		for (cp = count + bmin; nc > 0; cp++) {
			while (*cp == 0)
				cp++;
			if ((c = *cp) > 1) {
				if (c > bigc) {
					bigc = c;
					sp1 = sp;
				}
				push(ak, c, i+1);
			}
			top[cp-count] = ak += c;
			*cp = 0;			/* Reset count[]. */
			nc--;
		}
		swap(*sp0, *sp1, temp);

		for (ak = ta + n, ai = a+n; ak > ta;)	/* Copy to temp. */
			*--ak = *--ai;
		for (ak = ta+n; --ak >= ta;)		/* Deal to piles. */
			*--top[tr[(*ak)[i]]] = *ak;
	}
}

static __inline__ void
InsertionSort(const Uint8 **a, int n, int b, const Uint8 *tr, Uint endch)
{
	Uint8 ch;
	const Uint8 **ak, **ai, *s, *t;

	for (ak = a+1; --n >= 1; ak++)
		for (ai = ak; ai > a; ai--) {
			for (s = ai[0] + b, t = ai[-1] + b;
			    (ch = tr[*s]) != endch; s++, t++)
				if (ch != tr[*t])
					break;
			if (ch >= tr[*t])
				break;
			swap(ai[0], ai[-1], s);
		}
}
