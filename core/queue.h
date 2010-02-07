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
	struct t *slh_first;	/* first element */			\
}
#define AG_SLIST_HEAD_(t)						\
struct {								\
	struct t *slh_first;	/* first element */			\
}
 
#define	AG_SLIST_HEAD_INITIALIZER(head)					\
	{ NULL }
 
#define AG_SLIST_ENTRY(t)						\
struct {								\
	struct t *sle_next;	/* next element */			\
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
	struct t *lh_first;	/* first element */			\
}
#define AG_LIST_HEAD_(t)						\
struct {								\
	struct t *lh_first;	/* first element */			\
}

#define AG_LIST_HEAD_INITIALIZER(head)					\
	{ NULL }

#define AG_LIST_ENTRY(t)						\
struct {								\
	struct t *le_next;	/* next element */			\
	struct t **le_prev;	/* address of previous next element */	\
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
	struct t *sqh_first;	/* first element */			\
	struct t **sqh_last;	/* addr of last next element */		\
}
#define AG_SIMPLEQ_HEAD_(t)						\
struct {								\
	struct t *sqh_first;	/* first element */			\
	struct t **sqh_last;	/* addr of last next element */		\
}

#define AG_SIMPLEQ_HEAD_INITIALIZER(head)				\
	{ NULL, &(head).sqh_first }

#define AG_SIMPLEQ_ENTRY(t)						\
struct {								\
	struct t *sqe_next;	/* next element */			\
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
	struct t *tqh_first;	/* first element */			\
	struct t **tqh_last;	/* addr of last next element */		\
}
#define AG_TAILQ_HEAD_(t)						\
struct {								\
	struct t *tqh_first;	/* first element */			\
	struct t **tqh_last;	/* addr of last next element */		\
}

#define AG_TAILQ_HEAD_INITIALIZER(head)					\
	{ NULL, &(head).tqh_first }

#define AG_TAILQ_ENTRY(t)						\
struct {								\
	struct t *tqe_next;	/* next element */			\
	struct t **tqe_prev;	/* address of previous next element */	\
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
/* XXX */
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
	struct t *cqh_first;		/* first element */		\
	struct t *cqh_last;		/* last element */		\
}
#define AG_CIRCLEQ_HEAD_(t)						\
struct {								\
	struct t *cqh_first;		/* first element */		\
	struct t *cqh_last;		/* last element */		\
}

#define AG_CIRCLEQ_HEAD_INITIALIZER(head)				\
	{ AG_CIRCLEQ_END(&head), AG_CIRCLEQ_END(&head) }

#define AG_CIRCLEQ_ENTRY(t)						\
struct {								\
	struct t *cqe_next;		/* next element */		\
	struct t *cqe_prev;		/* previous element */		\
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

#define SLIST_HEAD			AG_SLIST_HEAD
#define SLIST_HEAD_			AG_SLIST_HEAD_
#define SLIST_HEAD_INITIALIZER		AG_SLIST_HEAD_INITIALIZER
#define SLIST_ENTRY			AG_SLIST_ENTRY
#define SLIST_FIRST			AG_SLIST_FIRST
#define SLIST_END			AG_SLIST_END
#define SLIST_EMPTY			AG_SLIST_EMPTY
#define SLIST_NEXT			AG_SLIST_NEXT
#define SLIST_FOREACH			AG_SLIST_FOREACH
#define SLIST_INIT			AG_SLIST_INIT
#define SLIST_INSERT_AFTER		AG_SLIST_INSERT_AFTER
#define SLIST_INSERT_HEAD		AG_SLIST_INSERT_HEAD
#define SLIST_REMOVE_HEAD		AG_SLIST_REMOVE_HEAD
#define SLIST_REMOVE			AG_SLIST_REMOVE

#define LIST_HEAD			AG_LIST_HEAD
#define LIST_HEAD_			AG_LIST_HEAD_
#define LIST_HEAD_INITIALIZER		AG_LIST_HEAD_INITIALIZER
#define LIST_ENTRY			AG_LIST_ENTRY
#define LIST_FIRST			AG_LIST_FIRST
#define LIST_END			AG_LIST_END
#define LIST_EMPTY			AG_LIST_EMPTY
#define LIST_NEXT			AG_LIST_NEXT
#define LIST_FOREACH			AG_LIST_FOREACH
#define LIST_INIT			AG_LIST_INIT
#define LIST_INSERT_AFTER		AG_LIST_INSERT_AFTER
#define LIST_INSERT_BEFORE		AG_LIST_INSERT_BEFORE
#define LIST_INSERT_HEAD		AG_LIST_INSERT_HEAD
#define LIST_REMOVE			AG_LIST_REMOVE
#define LIST_REPLACE			AG_LIST_REPLACE

#define SIMPLEQ_HEAD			AG_SIMPLEQ_HEAD
#define SIMPLEQ_HEAD_			AG_SIMPLEQ_HEAD_
#define SIMPLEQ_HEAD_INITIALIZER	AG_SIMPLEQ_HEAD_INITIALIZER
#define SIMPLEQ_ENTRY			AG_SIMPLEQ_ENTRY
#define SIMPLEQ_FIRST			AG_SIMPLEQ_FIRST
#define SIMPLEQ_END			AG_SIMPLEQ_END
#define SIMPLEQ_EMPTY			AG_SIMPLEQ_EMPTY
#define SIMPLEQ_NEXT			AG_SIMPLEQ_NEXT
#define SIMPLEQ_FOREACH			AG_SIMPLEQ_FOREACH
#define SIMPLEQ_INIT			AG_SIMPLEQ_INIT
#define SIMPLEQ_INSERT_HEAD		AG_SIMPLEQ_INSERT_HEAD
#define SIMPLEQ_INSERT_TAIL		AG_SIMPLEQ_INSERT_TAIL
#define SIMPLEQ_INSERT_AFTER		AG_SIMPLEQ_INSERT_AFTER
#define SIMPLEQ_REMOVE_HEAD		AG_SIMPLEQ_REMOVE_HEAD

#define TAILQ_HEAD			AG_TAILQ_HEAD
#define TAILQ_HEAD_			AG_TAILQ_HEAD_
#define TAILQ_HEAD_INITIALIZER		AG_TAILQ_HEAD_INITIALIZER
#define TAILQ_ENTRY			AG_TAILQ_ENTRY
#define TAILQ_FIRST			AG_TAILQ_FIRST
#define TAILQ_END			AG_TAILQ_END
#define TAILQ_NEXT			AG_TAILQ_NEXT
#define TAILQ_LAST			AG_TAILQ_LAST
#define TAILQ_PREV			AG_TAILQ_PREV
#define TAILQ_EMPTY			AG_TAILQ_EMPTY
#define TAILQ_FOREACH			AG_TAILQ_FOREACH
#define TAILQ_FOREACH_REVERSE		AG_TAILQ_FOREACH_REVERSE
#define TAILQ_INIT			AG_TAILQ_INIT
#define TAILQ_INSERT_HEAD		AG_TAILQ_INSERT_HEAD
#define TAILQ_INSERT_TAIL		AG_TAILQ_INSERT_TAIL
#define TAILQ_INSERT_AFTER		AG_TAILQ_INSERT_AFTER
#define TAILQ_INSERT_BEFORE		AG_TAILQ_INSERT_BEFORE
#define TAILQ_REMOVE			AG_TAILQ_REMOVE
#define TAILQ_REPLACE			AG_TAILQ_REPLACE

#define CIRCLEQ_HEAD			AG_CIRCLEQ_HEAD
#define CIRCLEQ_HEAD_			AG_CIRCLEQ_HEAD_
#define CIRCLEQ_HEAD_INITIALIZER	AG_CIRCLEQ_HEAD_INITIALIZER
#define CIRCLEQ_ENTRY			AG_CIRCLEQ_ENTRY
#define CIRCLEQ_FIRST			AG_CIRCLEQ_FIRST
#define CIRCLEQ_LAST			AG_CIRCLEQ_LAST
#define CIRCLEQ_END			AG_CIRCLEQ_END
#define CIRCLEQ_NEXT			AG_CIRCLEQ_NEXT
#define CIRCLEQ_PREV			AG_CIRCLEQ_PREV
#define CIRCLEQ_EMPTY			AG_CIRCLEQ_EMPTY
#define CIRCLEQ_FOREACH			AG_CIRCLEQ_FOREACH
#define CIRCLEQ_FOREACH_REVERSE		AG_CIRCLEQ_FOREACH_REVERSE
#define CIRCLEQ_INIT			AG_CIRCLEQ_INIT
#define CIRCLEQ_INSERT_AFTER		AG_CIRCLEQ_INSERT_AFTER
#define CIRCLEQ_INSERT_BEFORE		AG_CIRCLEQ_INSERT_BEFORE
#define CIRCLEQ_INSERT_HEAD		AG_CIRCLEQ_INSERT_HEAD
#define CIRCLEQ_INSERT_TAIL		AG_CIRCLEQ_INSERT_TAIL
#define CIRCLEQ_REMOVE			AG_CIRCLEQ_REMOVE
#define CIRCLEQ_REPLACE			AG_CIRCLEQ_REPLACE

#endif /* _AGAR_INTERNAL or _USE_AGAR_QUEUE */

#endif	/* !_AGAR_CORE_QUEUE_H_ */
