/*	$Csoft: rwlock.c,v 1.1 2004/02/26 09:19:38 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 * All rights reserved.
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

#include <config/threads.h>
#include <config/debug.h>

#ifdef THREADS

#define _XOPEN_SOURCE 500	/* Require recursive mutexes */
#include <pthread.h>
#include <signal.h>
#undef _XOPEN_SOURCE

#include <errno.h>

#include "rwlock.h"

static rwlockattr_t default_rwlockattr = {
	0, RWLOCKATTR_MAGIC
};

int
rwlockattr_init(rwlockattr_t *attr)
{
	attr->type = 0;
	attr->magic = RWLOCKATTR_MAGIC;
	return (0);
}

int
rwlockattr_destroy(rwlockattr_t *attr)
{
#ifdef DEBUG
	if (attr->magic != RWLOCKATTR_MAGIC)
		return (EINVAL);
#endif
	attr->type = 0;
	attr->magic = 0;
	return (0);
}

int
rwlockattr_settype(rwlockattr_t *attr, int type)
{
#ifdef DEBUG
	if (attr->magic != RWLOCKATTR_MAGIC)
		return (EINVAL);
#endif
	attr->type = type;
	return (0);
}

int
rwlockattr_gettype(rwlockattr_t *attr, int *type)
{
#ifdef DEBUG
	if (attr->magic != RWLOCKATTR_MAGIC)
		return (EINVAL);
#endif
	*type = attr->type;
	return (0);
}

int
rwlock_init(rwlock_t *rw, rwlockattr_t *attr)
{
	int res;

	rw->attr = (attr == NULL) ? &default_rwlockattr : attr;

	res = pthread_mutexattr_init(&rw->mutexattr);
	if (res != 0) {
		return (res);
	}

	if (rw->attr->type == RWLOCK_RECURSIVE) {
		res = pthread_mutexattr_settype(&rw->mutexattr,
		    PTHREAD_MUTEX_RECURSIVE);
		if (res != 0) {
			pthread_mutexattr_destroy(&rw->mutexattr);
			return (res);
		}
	}

	res = pthread_mutex_init(&rw->mutex, &rw->mutexattr);
	if (res != 0) {
		pthread_mutexattr_destroy(&rw->mutexattr);
		return (res);
	}

	res = pthread_cond_init(&rw->condreaders, NULL);
	if (res != 0) {
		pthread_mutexattr_destroy(&rw->mutexattr);
		pthread_mutex_destroy(&rw->mutex);
		return (res);
	}

	res = pthread_cond_init(&rw->condwriters, NULL);
	if (res != 0) {
		pthread_mutexattr_destroy(&rw->mutexattr);
		pthread_mutex_destroy(&rw->mutex);
		pthread_cond_destroy(&rw->condreaders);
		return (res);
	}

	rw->nwaitreaders = 0;
	rw->nwaitwriters = 0;
	rw->ref_count = 0;
	rw->magic = RWLOCK_MAGIC;

	return (0);
}

int
rwlock_destroy(rwlock_t *rw)
{
#ifdef DEBUG
	if (rw->magic != RWLOCK_MAGIC)
		return (EINVAL);
#endif
	if (rw->ref_count != 0 ||
	    rw->nwaitreaders != 0 || rw->nwaitwriters != 0) {
		return (EBUSY);
	}

	pthread_mutex_destroy(&rw->mutex);
	pthread_cond_destroy(&rw->condreaders);
	pthread_cond_destroy(&rw->condwriters);
	
	rw->magic = 0;
	return (0);
}

static void
rwlock_cancel_rdwait(void *p)
{
	rwlock_t *rw = p;

	rw->nwaitreaders--;
	pthread_mutex_unlock(&rw->mutex);
}

static void
rwlock_cancel_wrwait(void *p)
{
	rwlock_t *rw = p;

	rw->nwaitwriters--;
	pthread_mutex_unlock(&rw->mutex);
}

int
rwlock_rdlock(rwlock_t *rw)
{
	int res;

#ifdef DEBUG
	if (rw->magic != RWLOCK_MAGIC)
		return (EINVAL);
#endif

	if ((res = pthread_mutex_lock(&rw->mutex)) != 0)
		return (res);

	/* Give preference to waiting writers. */
	while (rw->ref_count < 0 || rw->nwaitwriters > 0) {
		rw->nwaitreaders++;
		pthread_cleanup_push(rwlock_cancel_rdwait, (void *)rw);
		res = pthread_cond_wait(&rw->condreaders, &rw->mutex);
		pthread_cleanup_pop(0);
		rw->nwaitreaders--;
		if (res != 0)
			break;
	}
	
	if (res == 0) {
		/* Another reader has a read lock. */
		rw->ref_count++;
	}

	pthread_mutex_unlock(&rw->mutex);
	return (res);
}

int
rwlock_tryrdlock(rwlock_t *rw)
{
	int res;

#ifdef DEBUG
	if (rw->magic != RWLOCK_MAGIC)
		return (EINVAL);
#endif

	if ((res = pthread_mutex_lock(&rw->mutex)) != 0)
		return (res);

	if (rw->ref_count < 0 || rw->nwaitwriters > 0) {
		/* Held by a writer or waiting writers */
		res = EBUSY;
	} else {
		/* Increment count of reader locks. */
		rw->ref_count++;
	}

	pthread_mutex_unlock(&rw->mutex);
	return (res);
}

int
rwlock_wrlock(rwlock_t *rw)
{
	int res;

#ifdef DEBUG
	if (rw->magic != RWLOCK_MAGIC)
		return (EINVAL);
#endif
	if ((res = pthread_mutex_lock(&rw->mutex)) != 0)
		return (res);

	while (rw->ref_count != 0) {
		rw->nwaitwriters++;
		pthread_cleanup_push(rwlock_cancel_wrwait, (void *)rw);
		res = pthread_cond_wait(&rw->condwriters, &rw->mutex);
		pthread_cleanup_pop(0);
		rw->nwaitwriters--;
		if (res != 0)
			break;
	}

	if (res == 0)
		rw->ref_count = -1;

	pthread_mutex_unlock(&rw->mutex);
	return (res);
}

int
rwlock_trywrlock(rwlock_t *rw)
{
	int res;

#ifdef DEBUG
	if (rw->magic != RWLOCK_MAGIC)
		return (EINVAL);
#endif

	if ((res = pthread_mutex_lock(&rw->mutex)) != 0)
		return (res);

	if (rw->ref_count != 0) {
		/* Held by either writer or reader(s) */
		res = EBUSY;
	} else {
		/* Available, indicate a writer has it */
		rw->ref_count = -1;
	}
	return (res);
}

int
rwlock_unlock(rwlock_t *rw)
{
	int res;

#ifdef DEBUG
	if (rw->magic != RWLOCK_MAGIC)
		return (EINVAL);
#endif

	if ((res = pthread_mutex_lock(&rw->mutex)) != 0)
		return (res);

	if (rw->ref_count > 0) {
		/* Releasing a reader */
		rw->ref_count--;
	} else if (rw->ref_count == -1) {
		/* Releasing a writer */
		rw->ref_count = 0;
	} else {
		return (EINVAL);
	}

	/* Give preference to waiting writers over waiting readers. */
	if (rw->nwaitwriters > 0) {
		if (rw->ref_count == 0) {
			res = pthread_cond_signal(&rw->condwriters);
		}
	} else if (rw->nwaitreaders > 0) {
		res = pthread_cond_broadcast(&rw->condreaders);
	}

	pthread_mutex_unlock(&rw->mutex);
	return (res);
}

#endif	/* THREADS */
