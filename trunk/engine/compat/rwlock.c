/*	$Csoft$	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include <pthread.h>
#include <errno.h>

#include "rwlock.h"

int
rwlock_init(rwlock_t *rw, rwlockattr_t *attr)
{
	int res;

	rw->attr = attr;

	res = pthread_mutex_init(&rw->mutex, NULL);
	if (res != 0)
		goto err1;
	res = pthread_cond_init(&rw->condreaders, NULL);
	if (res != 0)
		goto err2;
	res = pthread_cond_init(&rw->condwriters, NULL);
	if (res != 0)
		goto err3;

	rw->nwaitreaders = 0;
	rw->nwaitwriters = 0;
	rw->ref_count = 0;
	rw->magic = RWLOCK_MAGIC;

	return (0);
err3:
	pthread_cond_destroy(&rw->condreaders);
err2:
	pthread_mutex_destroy(&rw->mutex);
err1:
	return (res);
}

int
rwlock_destroy(rwlock_t *rw)
{
	if (rw->magic != RWLOCK_MAGIC) {
		return (EINVAL);
	}
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

int
rwlock_rdlock(rwlock_t *rw)
{
	int res;

	if (rw->magic != RWLOCK_MAGIC) {
		return (EINVAL);
	}

	res = pthread_mutex_lock(&rw->mutex);
	if (res != 0) {
		return (res);
	}

	/* Give preference to waiting writers. */
	while (rw->ref_count < 0 || rw->nwaitwriters > 0) {
		rw->nwaitreaders++;
		res = pthread_cond_wait(&rw->condreaders, &rw->mutex);
		rw->nwaitreaders--;
		if (res != 0) {
			break;
		}
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

	if (rw->magic != RWLOCK_MAGIC) {
		return (EINVAL);
	}

	res = pthread_mutex_lock(&rw->mutex);
	if (res != 0) {
		return (res);
	}

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

	if (rw->magic != RWLOCK_MAGIC) {
		return (EINVAL);
	}

	res = pthread_mutex_lock(&rw->mutex);
	if (res != 0) {
		return (res);
	}

	while (rw->ref_count != 0) {
		rw->nwaitwriters++;
		res = pthread_cond_wait(&rw->condwriters, &rw->mutex);
		rw->nwaitwriters--;
		if (res != 0) {
			break;
		}
	}

	if (res == 0) {
		rw->ref_count = -1;
	}

	pthread_mutex_unlock(&rw->mutex);
	return (res);
}

int
rwlock_trywrlock(rwlock_t *rw)
{
	int res;

	if (rw->magic != RWLOCK_MAGIC) {
		return (EINVAL);
	}

	res = pthread_mutex_lock(&rw->mutex);
	if (res != 0) {
		return (res);
	}

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

	if (rw->magic != RWLOCK_MAGIC) {
		return (EINVAL);
	}

	res = pthread_mutex_lock(&rw->mutex);
	if (res != 0) {
		return (res);
	}

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
