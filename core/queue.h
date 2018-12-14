/*	$OpenBSD: queue.h,v 1.22 2001/06/23 04:39:35 angelos Exp $	*/
/*	$NetBSD: queue.h,v 1.11 1996/05/16 05:17:14 mycroft Exp $	*/
/*
 * Copyright (c) 1991, 1993
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
 *
 *	@(#)queue.h	8.5 (Berkeley) 8/20/94
 */

#ifndef	_AGAR_CORE_QUEUE_H_
#define	_AGAR_CORE_QUEUE_H_

/*
 * This file defines five types of data structures: singly-linked lists, 
 * lists, simple queues, tail queues, and circular queues.
 *
 *
 * A singly-linked list is headed by a single forward pointer. The elements
 * are singly linked for minimum space and pointer manipulation overhead at
 * the expense of O(n) removal for arbitrary elements. New elements can be
 * added to the list after an existing element or at the head of the list.
 * Elements being removed from the head of the list should use the explicit
 * macro for this purpose for optimum efficiency. A singly-linked list may
 * only be traversed in the forward direction.  Singly-linked lists are ideal
 * for applications with large datasets and few or no removals or for
 * implementing a LIFO queue.
 *
 * A list is headed by a single forward pointer (or an array of forward
 * pointers for a hash table header). The elements are doubly linked
 * so that an arbitrary element can be removed without a need to
 * traverse the list. New elements can be added to the list before
 * or after an existing element or at the head of the list. A list
 * may only be traversed in the forward direction.
 *
 * A simple queue is headed by a pair of pointers, one the head of the
 * list and the other to the tail of the list. The elements are singly
 * linked to save space, so elements can only be removed from the
 * head of the list. New elements can be added to the list before or after
 * an existing element, at the head of the list, or at the end of the
 * list. A simple queue may only be traversed in the forward direction.
 *
 * A tail queue is headed by a pair of pointers, one to the head of the
 * list and the other to the tail of the list. The elements are doubly
 * linked so that an arbitrary element can be removed without a need to
 * traverse the list. New elements can be added to the list before or
 * after an existing element, at the head of the list, or at the end of
 * the list. A tail queue may be traversed in either direction.
 *
 * A circle queue is headed by a pair of pointers, one to the head of the
 * list and the other to the tail of the list. The elements are doubly
 * linked so that an arbitrary element can be removed without a need to
 * traverse the list. New elements can be added to the list before or after
 * an existing element, at the head of the list, or at the end of the list.
 * A circle queue may be traversed in either direction, but has a more
 * complex end of list detection.
 *
 * For details on the use of these macros, see the queue(3) manual page.
 */

/*
 * Singly-linked List definitions.
 */
#define AG_SLIST_HEAD(name, t)						\
struct name {								\
	struct t *_Nullable slh_first;					\
}
#define AG_SLIST_HEAD_(t)						\
struct {								\
	struct t *_Nullable slh_first;					\
}
 
#define	AG_SLIST_HEAD_INITIALIZER(head)					\
	{ NULL }
 
#define AG_SLIST_ENTRY(t)						\
struct {								\
	struct t *_Nullable sle_next;					\
}
 
/*
 * Singly-linked List access methods.
 */
#define	AG_SLIST_FIRST(head)	((head)->slh_first)
#define	AG_SLIST_END(head)	NULL
#define	AG_SLIST_EMPTY(head)	(AG_SLIST_FIRST(head) == AG_SLIST_END(head))
#define	AG_SLIST_NEXT(elm, field) ((elm)->field.sle_next)

#define	AG_SLIST_FOREACH(var, head, field)				\
	for((var) = AG_SLIST_FIRST(head);				\
	    (var) != AG_SLIST_END(head);				\
	    (var) = AG_SLIST_NEXT(var, field))

/*
 * Singly-linked List functions.
 */
#define	AG_SLIST_INIT(head) {						\
	AG_SLIST_FIRST(head) = AG_SLIST_END(head);			\
}

#define	AG_SLIST_INSERT_AFTER(slistelm, elm, field) do {		\
	(elm)->field.sle_next = (slistelm)->field.sle_next;		\
	(slistelm)->field.sle_next = (elm);				\
} while (0)

#define	AG_SLIST_INSERT_HEAD(head, elm, field) do {			\
	(elm)->field.sle_next = (head)->slh_first;			\
	(head)->slh_first = (elm);					\
} while (0)

#define	AG_SLIST_REMOVE_HEAD(head, field) do {				\
	(head)->slh_first = (head)->slh_first->field.sle_next;		\
} while (0)

#define AG_SLIST_REMOVE(head, elm, t, field) do {			\
	if ((head)->slh_first == (elm)) {				\
		AG_SLIST_REMOVE_HEAD((head), field);			\
	}								\
	else {								\
		struct t *curelm = (head)->slh_first;			\
		while( curelm->field.sle_next != (elm) )		\
			curelm = curelm->field.sle_next;		\
		curelm->field.sle_next =				\
		    curelm->field.sle_next->field.sle_next;		\
	}								\
} while (0)

/*
 * List definitions.
 */
#define AG_LIST_HEAD(name, t)						\
struct name {								\
	struct t *_Nullable lh_first;					\
}
#define AG_LIST_HEAD_(t)						\
struct {								\
	struct t *_Nullable lh_first;					\
}

#define AG_LIST_HEAD_INITIALIZER(head)					\
	{ NULL }

#define AG_LIST_ENTRY(t)						\
struct {								\
	struct t *_Nullable le_next;					\
	struct t *_Nullable *_Nullable le_prev;				\
}

/*
 * List access methods
 */
#define	AG_LIST_FIRST(head)	((head)->lh_first)
#define	AG_LIST_END(head)	NULL
#define	AG_LIST_EMPTY(head)	(AG_LIST_FIRST(head) == AG_LIST_END(head))
#define	AG_LIST_NEXT(elm, field) ((elm)->field.le_next)

#define AG_LIST_FOREACH(var, head, field)				\
	for((var) = AG_LIST_FIRST(head);				\
	    (var)!= AG_LIST_END(head);					\
	    (var) = AG_LIST_NEXT(var, field))

/*
 * List functions.
 */
#define	AG_LIST_INIT(head) do {						\
	AG_LIST_FIRST(head) = AG_LIST_END(head);			\
} while (0)

#define AG_LIST_INSERT_AFTER(listelm, elm, field) do {			\
	if (((elm)->field.le_next = (listelm)->field.le_next) != NULL)	\
		(listelm)->field.le_next->field.le_prev =		\
		    &(elm)->field.le_next;				\
	(listelm)->field.le_next = (elm);				\
	(elm)->field.le_prev = &(listelm)->field.le_next;		\
} while (0)

#define	AG_LIST_INSERT_BEFORE(listelm, elm, field) do {			\
	(elm)->field.le_prev = (listelm)->field.le_prev;		\
	(elm)->field.le_next = (listelm);				\
	*(listelm)->field.le_prev = (elm);				\
	(listelm)->field.le_prev = &(elm)->field.le_next;		\
} while (0)

#define AG_LIST_INSERT_HEAD(head, elm, field) do {			\
	if (((elm)->field.le_next = (head)->lh_first) != NULL)		\
		(head)->lh_first->field.le_prev = &(elm)->field.le_next;\
	(head)->lh_first = (elm);					\
	(elm)->field.le_prev = &(head)->lh_first;			\
} while (0)

#define AG_LIST_REMOVE(elm, field) do {					\
	if ((elm)->field.le_next != NULL)				\
		(elm)->field.le_next->field.le_prev =			\
		    (elm)->field.le_prev;				\
	*(elm)->field.le_prev = (elm)->field.le_next;			\
} while (0)

#define AG_LIST_REPLACE(elm, elm2, field) do {				\
	if (((elm2)->field.le_next = (elm)->field.le_next) != NULL)	\
		(elm2)->field.le_next->field.le_prev =			\
		    &(elm2)->field.le_next;				\
	(elm2)->field.le_prev = (elm)->field.le_prev;			\
	*(elm2)->field.le_prev = (elm2);				\
} while (0)

/*
 * Simple queue definitions.
 */
#define AG_SIMPLEQ_HEAD(name, t)					\
struct name {								\
	struct t *_Nullable sqh_first;					\
	struct t *_Nullable *_Nullable sqh_last;			\
}
#define AG_SIMPLEQ_HEAD_(t)						\
struct {								\
	struct t *_Nullable sqh_first;					\
	struct t *_Nullable *_Nullable sqh_last;			\
}

#define AG_SIMPLEQ_HEAD_INITIALIZER(head)				\
	{ NULL, &(head).sqh_first }

#define AG_SIMPLEQ_ENTRY(t)						\
struct {								\
	struct t *_Nullable sqe_next;					\
}

/*
 * Simple queue access methods.
 */
#define	AG_SIMPLEQ_FIRST(head)  ((head)->sqh_first)
#define	AG_SIMPLEQ_END(head)    NULL
#define	AG_SIMPLEQ_EMPTY(head)  (AG_SIMPLEQ_FIRST(head) == AG_SIMPLEQ_END(head))
#define	AG_SIMPLEQ_NEXT(elm, field) ((elm)->field.sqe_next)

#define AG_SIMPLEQ_FOREACH(var, head, field)				\
	for((var) = AG_SIMPLEQ_FIRST(head);				\
	    (var) != AG_SIMPLEQ_END(head);				\
	    (var) = AG_SIMPLEQ_NEXT(var, field))

/*
 * Simple queue functions.
 */
#define	AG_SIMPLEQ_INIT(head) do {					\
	(head)->sqh_first = NULL;					\
	(head)->sqh_last = &(head)->sqh_first;				\
} while (0)

#define AG_SIMPLEQ_INSERT_HEAD(head, elm, field) do {			\
	if (((elm)->field.sqe_next = (head)->sqh_first) == NULL)	\
		(head)->sqh_last = &(elm)->field.sqe_next;		\
	(head)->sqh_first = (elm);					\
} while (0)

#define AG_SIMPLEQ_INSERT_TAIL(head, elm, field) do {			\
	(elm)->field.sqe_next = NULL;					\
	*(head)->sqh_last = (elm);					\
	(head)->sqh_last = &(elm)->field.sqe_next;			\
} while (0)

#define AG_SIMPLEQ_INSERT_AFTER(head, listelm, elm, field) do {		\
	if (((elm)->field.sqe_next = (listelm)->field.sqe_next) == NULL)\
		(head)->sqh_last = &(elm)->field.sqe_next;		\
	(listelm)->field.sqe_next = (elm);				\
} while (0)

#define AG_SIMPLEQ_REMOVE_HEAD(head, elm, field) do {			\
	if (((head)->sqh_first = (elm)->field.sqe_next) == NULL)	\
		(head)->sqh_last = &(head)->sqh_first;			\
} while (0)

/*
 * Tail queue definitions.
 */
#define AG_TAILQ_HEAD(name, t)						\
struct name {								\
	struct t *_Nullable tqh_first;	/* first element */			\
	struct t *_Nullable *_Nullable tqh_last; /* addr of last next element */		\
}
#define AG_TAILQ_HEAD_(t)						\
struct {								\
	struct t *_Nullable tqh_first;	/* first element */			\
	struct t *_Nullable *_Nullable tqh_last; /* addr of last next element */		\
}

#define AG_TAILQ_HEAD_INITIALIZER(head)					\
	{ NULL, &(head).tqh_first }

#define AG_TAILQ_ENTRY(t)						\
struct {								\
	struct t *_Nullable tqe_next;	         /* next element */			\
	struct t *_Nullable *_Nullable tqe_prev; /* address of previous next element */	\
}
#define AG_TAILQ_ENTRY_INITIALIZER { NULL, NULL }

/* 
 * tail queue access methods 
 */
#define	AG_TAILQ_FIRST(head)		((head)->tqh_first)
#define	AG_TAILQ_END(head)		NULL
#define	AG_TAILQ_NEXT(elm, field)	((elm)->field.tqe_next)

#define AG_TAILQ_LAST(head, headname)					\
	(*(((struct headname *)((head)->tqh_last))->tqh_last))

#define AG_TAILQ_PREV(elm, headname, field)				\
	(*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))

#define	AG_TAILQ_EMPTY(head)						\
	(AG_TAILQ_FIRST(head) == AG_TAILQ_END(head))

#define AG_TAILQ_FOREACH(var, head, field)				\
	for((var) = AG_TAILQ_FIRST(head);				\
	    (var) != AG_TAILQ_END(head);				\
	    (var) = AG_TAILQ_NEXT(var, field))

#define AG_TAILQ_FOREACH_REVERSE(var, head, headname, field)		\
	for((var) = AG_TAILQ_LAST(head, headname);			\
	    (var) != AG_TAILQ_END(head);				\
	    (var) = AG_TAILQ_PREV(var, headname, field))

/*
 * Tail queue functions.
 */
#define	AG_TAILQ_INIT(head) do {					\
	(head)->tqh_first = NULL;					\
	(head)->tqh_last = &(head)->tqh_first;				\
} while (0)

#define AG_TAILQ_INSERT_HEAD(head, elm, field) do {			\
	if (((elm)->field.tqe_next = (head)->tqh_first) != NULL)	\
		(head)->tqh_first->field.tqe_prev =			\
		    &(elm)->field.tqe_next;				\
	else								\
		(head)->tqh_last = &(elm)->field.tqe_next;		\
	(head)->tqh_first = (elm);					\
	(elm)->field.tqe_prev = &(head)->tqh_first;			\
} while (0)

#define AG_TAILQ_INSERT_TAIL(head, elm, field) do {			\
	(elm)->field.tqe_next = NULL;					\
	(elm)->field.tqe_prev = (head)->tqh_last;			\
	*(head)->tqh_last = (elm);					\
	(head)->tqh_last = &(elm)->field.tqe_next;			\
} while (0)

#define AG_TAILQ_INSERT_AFTER(head, listelm, elm, field) do {		\
	if (((elm)->field.tqe_next = (listelm)->field.tqe_next) != NULL)\
		(elm)->field.tqe_next->field.tqe_prev =			\
		    &(elm)->field.tqe_next;				\
	else								\
		(head)->tqh_last = &(elm)->field.tqe_next;		\
	(listelm)->field.tqe_next = (elm);				\
	(elm)->field.tqe_prev = &(listelm)->field.tqe_next;		\
} while (0)

#define	AG_TAILQ_INSERT_BEFORE(listelm, elm, field) do {		\
	(elm)->field.tqe_prev = (listelm)->field.tqe_prev;		\
	(elm)->field.tqe_next = (listelm);				\
	*(listelm)->field.tqe_prev = (elm);				\
	(listelm)->field.tqe_prev = &(elm)->field.tqe_next;		\
} while (0)

#define AG_TAILQ_REMOVE(head, elm, field) do {				\
	if (((elm)->field.tqe_next) != NULL)				\
		(elm)->field.tqe_next->field.tqe_prev =			\
		    (elm)->field.tqe_prev;				\
	else								\
		(head)->tqh_last = (elm)->field.tqe_prev;		\
	*(elm)->field.tqe_prev = (elm)->field.tqe_next;			\
} while (0)

#define AG_TAILQ_REPLACE(head, elm, elm2, field) do {			\
	if (((elm2)->field.tqe_next = (elm)->field.tqe_next) != NULL)	\
		(elm2)->field.tqe_next->field.tqe_prev =		\
		    &(elm2)->field.tqe_next;				\
	else								\
		(head)->tqh_last = &(elm2)->field.tqe_next;		\
	(elm2)->field.tqe_prev = (elm)->field.tqe_prev;			\
	*(elm2)->field.tqe_prev = (elm2);				\
} while (0)

/*
 * Circular queue definitions.
 */
#define AG_CIRCLEQ_HEAD(name, t)					\
struct name {								\
	struct t *_Nullable cqh_first;					\
	struct t *_Nullable cqh_last;					\
}
#define AG_CIRCLEQ_HEAD_(t)						\
struct {								\
	struct t *_Nullable cqh_first;					\
	struct t *_Nullable cqh_last;					\
}

#define AG_CIRCLEQ_HEAD_INITIALIZER(head)				\
	{ AG_CIRCLEQ_END(&head), AG_CIRCLEQ_END(&head) }

#define AG_CIRCLEQ_ENTRY(t)						\
struct {								\
	struct t *_Nullable cqe_next;					\
	struct t *_Nullable cqe_prev;					\
}

/*
 * Circular queue access methods 
 */
#define	AG_CIRCLEQ_FIRST(head)		((head)->cqh_first)
#define	AG_CIRCLEQ_LAST(head)		((head)->cqh_last)
#define	AG_CIRCLEQ_END(head)		((void *)(head))
#define	AG_CIRCLEQ_NEXT(elm, field)	((elm)->field.cqe_next)
#define	AG_CIRCLEQ_PREV(elm, field)	((elm)->field.cqe_prev)
#define	AG_CIRCLEQ_EMPTY(head)						\
	(AG_CIRCLEQ_FIRST(head) == AG_CIRCLEQ_END(head))

#define AG_CIRCLEQ_FOREACH(var, head, field)				\
	for((var) = AG_CIRCLEQ_FIRST(head);				\
	    (var) != AG_CIRCLEQ_END(head);				\
	    (var) = AG_CIRCLEQ_NEXT(var, field))

#define AG_CIRCLEQ_FOREACH_REVERSE(var, head, field)			\
	for((var) = AG_CIRCLEQ_LAST(head);				\
	    (var) != AG_CIRCLEQ_END(head);				\
	    (var) = AG_CIRCLEQ_PREV(var, field))

/*
 * Circular queue functions.
 */
#define	AG_CIRCLEQ_INIT(head) do {					\
	(head)->cqh_first = AG_CIRCLEQ_END(head);			\
	(head)->cqh_last = AG_CIRCLEQ_END(head);			\
} while (0)

#define AG_CIRCLEQ_INSERT_AFTER(head, listelm, elm, field) do {		\
	(elm)->field.cqe_next = (listelm)->field.cqe_next;		\
	(elm)->field.cqe_prev = (listelm);				\
	if ((listelm)->field.cqe_next == AG_CIRCLEQ_END(head))		\
		(head)->cqh_last = (elm);				\
	else								\
		(listelm)->field.cqe_next->field.cqe_prev = (elm);	\
	(listelm)->field.cqe_next = (elm);				\
} while (0)

#define AG_CIRCLEQ_INSERT_BEFORE(head, listelm, elm, field) do {	\
	(elm)->field.cqe_next = (listelm);				\
	(elm)->field.cqe_prev = (listelm)->field.cqe_prev;		\
	if ((listelm)->field.cqe_prev == AG_CIRCLEQ_END(head))		\
		(head)->cqh_first = (elm);				\
	else								\
		(listelm)->field.cqe_prev->field.cqe_next = (elm);	\
	(listelm)->field.cqe_prev = (elm);				\
} while (0)

#define AG_CIRCLEQ_INSERT_HEAD(head, elm, field) do {			\
	(elm)->field.cqe_next = (head)->cqh_first;			\
	(elm)->field.cqe_prev = AG_CIRCLEQ_END(head);			\
	if ((head)->cqh_last == AG_CIRCLEQ_END(head))			\
		(head)->cqh_last = (elm);				\
	else								\
		(head)->cqh_first->field.cqe_prev = (elm);		\
	(head)->cqh_first = (elm);					\
} while (0)

#define AG_CIRCLEQ_INSERT_TAIL(head, elm, field) do {			\
	(elm)->field.cqe_next = AG_CIRCLEQ_END(head);			\
	(elm)->field.cqe_prev = (head)->cqh_last;			\
	if ((head)->cqh_first == AG_CIRCLEQ_END(head))			\
		(head)->cqh_first = (elm);				\
	else								\
		(head)->cqh_last->field.cqe_next = (elm);		\
	(head)->cqh_last = (elm);					\
} while (0)

#define	AG_CIRCLEQ_REMOVE(head, elm, field) do {			\
	if ((elm)->field.cqe_next == AG_CIRCLEQ_END(head))		\
		(head)->cqh_last = (elm)->field.cqe_prev;		\
	else								\
		(elm)->field.cqe_next->field.cqe_prev =			\
		    (elm)->field.cqe_prev;				\
	if ((elm)->field.cqe_prev == AG_CIRCLEQ_END(head))		\
		(head)->cqh_first = (elm)->field.cqe_next;		\
	else								\
		(elm)->field.cqe_prev->field.cqe_next =			\
		    (elm)->field.cqe_next;				\
} while (0)

#define AG_CIRCLEQ_REPLACE(head, elm, elm2, field) do {			\
	if (((elm2)->field.cqe_next = (elm)->field.cqe_next) ==		\
	    AG_CIRCLEQ_END(head))					\
		(head).cqh_last = (elm2);				\
	else								\
		(elm2)->field.cqe_next->field.cqe_prev = (elm2);	\
	if (((elm2)->field.cqe_prev = (elm)->field.cqe_prev) ==		\
	    AG_CIRCLEQ_END(head))					\
		(head).cqh_first = (elm2);				\
	else								\
		(elm2)->field.cqe_prev->field.cqe_next = (elm2);	\
} while (0)

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_QUEUE)

#define SLIST_HEAD(name,t)			AG_SLIST_HEAD(name,t)
#define SLIST_HEAD_(t)				AG_SLIST_HEAD_(t)
#define SLIST_HEAD_INITIALIZER(head)		AG_SLIST_HEAD_INITIALIZER(head)
#define SLIST_ENTRY(t)				AG_SLIST_ENTRY(t)
#define SLIST_FIRST(head)			AG_SLIST_FIRST(head)
#define SLIST_END(head)				AG_SLIST_END(head)
#define SLIST_EMPTY(head)			AG_SLIST_EMPTY(head)
#define SLIST_NEXT(elm,field)			AG_SLIST_NEXT(elm,field)
#define SLIST_FOREACH(var,head,field)		AG_SLIST_FOREACH(var,head,field)
#define SLIST_INIT(head)			AG_SLIST_INIT(head)
#define SLIST_INSERT_AFTER(slistelm,elm,field)	AG_SLIST_INSERT_AFTER(slistelm,elm,field)
#define SLIST_INSERT_HEAD(head,elm,field)	AG_SLIST_INSERT_HEAD(head,elm,field)
#define SLIST_REMOVE_HEAD(head,field)		AG_SLIST_REMOVE_HEAD(head,field)
#define SLIST_REMOVE(head,elm,t,field)		AG_SLIST_REMOVE(head,elm,t,field)

#define LIST_HEAD(name,t)			AG_LIST_HEAD(name,t)
#define LIST_HEAD_(t)				AG_LIST_HEAD_(t)
#define LIST_HEAD_INITIALIZER(head)		AG_LIST_HEAD_INITIALIZER(head)
#define LIST_ENTRY(t)				AG_LIST_ENTRY(t)
#define LIST_FIRST(head)			AG_LIST_FIRST(head)
#define LIST_END(head)				AG_LIST_END(head)
#define LIST_EMPTY(head)			AG_LIST_EMPTY(head)
#define LIST_NEXT(elm,field)			AG_LIST_NEXT(elm,field)
#define LIST_FOREACH(var,head,field)		AG_LIST_FOREACH(var,head,field)
#define LIST_INIT(head)				AG_LIST_INIT(head)
#define LIST_INSERT_AFTER(listelm,elm,field)	AG_LIST_INSERT_AFTER(listelm,elm,field)
#define LIST_INSERT_BEFORE(listelm,elm,field)	AG_LIST_INSERT_BEFORE(listelm,elm,field)
#define LIST_INSERT_HEAD(head,elm,field)	AG_LIST_INSERT_HEAD(head,elm,field)
#define LIST_REMOVE(elm,field)			AG_LIST_REMOVE(elm,field)
#define LIST_REPLACE(elm,elm2,field)		AG_LIST_REPLACE(elm,elm2,field)

#define SIMPLEQ_HEAD(name,t)				AG_SIMPLEQ_HEAD(name,t)
#define SIMPLEQ_HEAD_(t)				AG_SIMPLEQ_HEAD_(t)
#define SIMPLEQ_HEAD_INITIALIZER(head)			AG_SIMPLEQ_HEAD_INITIALIZER(head)
#define SIMPLEQ_ENTRY(t)				AG_SIMPLEQ_ENTRY(t)
#define SIMPLEQ_FIRST(head)				AG_SIMPLEQ_FIRST(head)
#define SIMPLEQ_END(head)				AG_SIMPLEQ_END(head)
#define SIMPLEQ_EMPTY(head)				AG_SIMPLEQ_EMPTY(head)
#define SIMPLEQ_NEXT(elm,field)				AG_SIMPLEQ_NEXT(elm,field)
#define SIMPLEQ_FOREACH(var,head,field)			AG_SIMPLEQ_FOREACH(var,head,field)
#define SIMPLEQ_INIT(head)				AG_SIMPLEQ_INIT(head)
#define SIMPLEQ_INSERT_HEAD(head,elm,field)		AG_SIMPLEQ_INSERT_HEAD(head,elm,field)
#define SIMPLEQ_INSERT_TAIL(head,elm,field)		AG_SIMPLEQ_INSERT_TAIL(head,elm,field)
#define SIMPLEQ_INSERT_AFTER(head,listelm,elm,field)	AG_SIMPLEQ_INSERT_AFTER(head,listelm,elm,field)
#define SIMPLEQ_REMOVE_HEAD(head,elm,field)		AG_SIMPLEQ_REMOVE_HEAD(head,elm,field)

#define TAILQ_HEAD(name,t)				AG_TAILQ_HEAD(name,t)
#define TAILQ_HEAD_(t)					AG_TAILQ_HEAD_(t)
#define TAILQ_HEAD_INITIALIZER(head)			AG_TAILQ_HEAD_INITIALIZER(head)
#define TAILQ_ENTRY(t)					AG_TAILQ_ENTRY(t)
#define TAILQ_FIRST(head)				AG_TAILQ_FIRST(head)
#define TAILQ_END(head)					AG_TAILQ_END(head)
#define TAILQ_NEXT(elm,field)				AG_TAILQ_NEXT(elm,field)
#define TAILQ_LAST(head,headname)			AG_TAILQ_LAST(head,headname)
#define TAILQ_PREV(elm,headname,field)			AG_TAILQ_PREV(elm,headname,field)
#define TAILQ_EMPTY(head)				AG_TAILQ_EMPTY(head)
#define TAILQ_INIT(head)				AG_TAILQ_INIT(head)
#define TAILQ_FOREACH(var,head,field)			AG_TAILQ_FOREACH(var,head,field)
#define TAILQ_FOREACH_REVERSE(var,head,headname,field)	AG_TAILQ_FOREACH_REVERSE(var,head,headname,field)
#define TAILQ_INSERT_HEAD(head,elm,field)		AG_TAILQ_INSERT_HEAD(head,elm,field)
#define TAILQ_INSERT_TAIL(head,elm,field)		AG_TAILQ_INSERT_TAIL(head,elm,field)
#define TAILQ_INSERT_AFTER(head,listelm,elm,field)	AG_TAILQ_INSERT_AFTER(head,listelm,elm,field)
#define TAILQ_INSERT_BEFORE(listelm,elm,field)		AG_TAILQ_INSERT_BEFORE(listelm,elm,field)
#define TAILQ_REMOVE(head,elm,field)			AG_TAILQ_REMOVE(head,elm,field)
#define TAILQ_REPLACE(head,elm,elm2,field)		AG_TAILQ_REPLACE(head,elm,elm2,field)

#define CIRCLEQ_HEAD(name,t)				AG_CIRCLEQ_HEAD(name,t)
#define CIRCLEQ_HEAD_(t)				AG_CIRCLEQ_HEAD_(t)
#define CIRCLEQ_HEAD_INITIALIZER(head)			AG_CIRCLEQ_HEAD_INITIALIZER(head)
#define CIRCLEQ_ENTRY(t)				AG_CIRCLEQ_ENTRY(t)
#define CIRCLEQ_FIRST(head)				AG_CIRCLEQ_FIRST(head)
#define CIRCLEQ_LAST(head)				AG_CIRCLEQ_LAST(head)
#define CIRCLEQ_END(head)				AG_CIRCLEQ_END(head)
#define CIRCLEQ_NEXT(elm,field)				AG_CIRCLEQ_NEXT(elm,field)
#define CIRCLEQ_PREV(elm,field)				AG_CIRCLEQ_PREV(elm,field)
#define CIRCLEQ_EMPTY(head)				AG_CIRCLEQ_EMPTY(head)
#define CIRCLEQ_INIT(head)				AG_CIRCLEQ_INIT(head)
#define CIRCLEQ_FOREACH(var,head,field)			AG_CIRCLEQ_FOREACH(var,head,field)
#define CIRCLEQ_FOREACH_REVERSE(var,head,field)		AG_CIRCLEQ_FOREACH_REVERSE
#define CIRCLEQ_INSERT_AFTER(head,listelm,elm,field)	AG_CIRCLEQ_INSERT_AFTER(head,listelm,elm,field)
#define CIRCLEQ_INSERT_BEFORE(head,listelm,elm,field)	AG_CIRCLEQ_INSERT_BEFORE(head,listelm,elm,field)
#define CIRCLEQ_INSERT_HEAD(head,elm,field)		AG_CIRCLEQ_INSERT_HEAD(head,elm,field)
#define CIRCLEQ_INSERT_TAIL(head,elm,field)		AG_CIRCLEQ_INSERT_TAIL(head,elm,field)
#define CIRCLEQ_REMOVE(head,elm,field)			AG_CIRCLEQ_REMOVE(head,elm,field)
#define CIRCLEQ_REPLACE(head,elm,elm2,field)		AG_CIRCLEQ_REPLACE(head,elm,elm2,field)

#endif /* _AGAR_INTERNAL or _USE_AGAR_QUEUE */

#endif	/* !_AGAR_CORE_QUEUE_H_ */
