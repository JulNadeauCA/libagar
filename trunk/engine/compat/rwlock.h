/*	$Csoft: rwlock.h,v 1.1 2002/09/16 09:36:39 vedge Exp $	*/
/*	Public domain	*/

typedef struct {
	int	type;
#define RWLOCK_DEFAULT		0x01	 /* Error checking */
#define RWLOCK_RECURSIVE	0x02	 /* Can relock from the same thread */
	int	magic;			 /* Error check */
} rwlockattr_t;

typedef struct {
	pthread_mutex_t	mutex;		/* Basic lock on this struct */
	pthread_mutexattr_t mutexattr;	/* Recursive mutex prop */
	pthread_cond_t	condreaders;	/* For reader threads waiting */
	pthread_cond_t	condwriters;	/* For writer threads waiting */
	int		 magic;		/* Error check */
	int		 nwaitreaders;	/* The number waiting */
	int		 nwaitwriters;	/* The number waiting */
	int		 ref_count;	/* -1 if writer has the lock, else
					   the # readers holding the lock. */
	rwlockattr_t	*attr;
} rwlock_t;

#define RWLOCK_MAGIC		0x25732573
#define RWLOCKATTR_MAGIC	0x50255025

#define RWLOCK_INITIALIZER { 		\
	PTHREAD_MUTEX_INITIALIZER,	\
	PTHREAD_COND_INITIALIZER,	\
	PTHREAD_COND_INITIALIZER,	\
	RWLOCK_MAGIC, 0, 0, 0, NULL }

#define RWLOCKATTR_INITIALIZER { 0, RWLOCKATTR_MAGIC }

int	rwlock_destroy(rwlock_t *);
int	rwlock_init(rwlock_t *, rwlockattr_t *);
int	rwlock_rdlock(rwlock_t *);
int	rwlock_tryrdlock(rwlock_t *);
int	rwlock_wrlock(rwlock_t *);
int	rwlock_trywrlock(rwlock_t *);
int	rwlock_unlock(rwlock_t *);

int	rwlockattr_init(rwlockattr_t *);
int	rwlockattr_destroy(rwlockattr_t *);
int	rwlockattr_settype(rwlockattr_t *, int);
int	rwlockattr_gettype(rwlockattr_t *, int *);

