/*	$Csoft$	*/
/*	Public domain	*/

typedef struct {
	int	type;
#define RWLOCK_RECURSIVE	0x01	 /* Can relock from the same thread */
} rwlockattr_t;

typedef struct {
	pthread_mutex_t	mutex;		/* Basic lock on this struct */
	pthread_cond_t	condreaders;	/* For reader threads waiting */
	pthread_cond_t	condwriters;	/* For writer threads waiting */
	int		 magic;		/* Error check */
	int		 nwaitreaders;	/* The number waiting */
	int		 nwaitwriters;	/* The number waiting */
	int		 ref_count;	/* -1 if writer has the lock, else
					   the # readers holding the lock. */
	rwlockattr_t	*attr;
} rwlock_t;

#define RWLOCK_MAGIC	0x25732573
#define RWLOCK_INITIALIZER { \
	PTHREAD_MUTEX_INITIALIZER, \
	PTHREAD_COND_INITIALIZER, \
	PTHREAD_COND_INITIALIZER, \
	RWLOCK_MAGIC, 0, 0, 0, NULL }

int	rwlock_destroy(rwlock_t *);
int	rwlock_init(rwlock_t *, rwlockattr_t *);
int	rwlock_rdlock(rwlock_t *);
int	rwlock_tryrdlock(rwlock_t *);
int	rwlock_wrlock(rwlock_t *);
int	rwlock_trywrlock(rwlock_t *);
int	rwlock_unlock(rwlock_t *);

