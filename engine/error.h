/*	$Csoft: error.h,v 1.3 2002/06/09 10:08:04 vedge Exp $	*/
/*	Public domain	*/

#define AGAR_GetError()    ((char *)pthread_getspecific(engine_errorkey))
#define AGAR_SetError(msg) pthread_setspecific(engine_errorkey, (char *)(msg))

extern pthread_key_t engine_errorkey;	/* engine.c */

#ifdef __GNUC__
# define warning(fmt, args...) \
	printf("%s: " fmt, __FUNCTION__ , ##args)
# ifdef DEBUG
#  define fatal(fmt, args...)					\
	do {							\
		printf("%s: " fmt, __FUNCTION__ , ##args);	\
		abort();					\
	} while (/*CONSTCOND*/0)
# else
#  define fatal(fmt, args...)					\
	do {							\
		printf("%s: " fmt, __FUNCTION__ , ##args);	\
		engine_stop();					\
	} while (/*CONSTCOND*/0)
# endif
#else
# define warning	printf
# define fatal		printf
#endif

void	*emalloc(size_t);
void	*erealloc(void *, size_t);

#define eread(fd, buf, size) do {					\
	ssize_t _eread_rv;						\
									\
	_eread_rv = read((fd), (buf), (size));				\
	if (_eread_rv == -1) { 						\
		fatal("read(%ld): %s\n", (long)size, strerror(errno));	\
	}								\
	if (_eread_rv != size) {					\
		fatal("read(%ld): read %ld bytes\n", (long)size,	\
		    (long)_eread_rv);					\
	}								\
} while (/*CONSTCOND*/0)

#define ewrite(fd, buf, size) do {					\
	ssize_t _ewrite_rv;						\
									\
	_ewrite_rv = write((fd), (buf), (size));			\
	if (_ewrite_rv == -1) {						\
		fatal("write(%ld): %s\n", (long)size, strerror(errno));	\
	}								\
	if (_ewrite_rv != (size)) {					\
		fatal("write(%ld): wrote %ld bytes\n", (long)size,	\
		    (long)_ewrite_rv);					\
	}								\
} while (/*CONSTCOND*/0)

#define elseek(fd, off, whence) do {					\
	off_t _elseek_rv;						\
									\
	_elseek_rv = lseek((fd), (off), (whence));			\
	if (_elseek_rv == -1) {						\
		fatal("lseek(%ld, %s): %s\n", (long)off,		\
		    (whence == SEEK_SET) ? "SEEK_SET" :			\
		    (whence == SEEK_CUR) ? "SEEK_CUR" :			\
		    (whence == SEEK_END) ? "SEEK_END" :			\
		     "???",						\
		    strerror(errno));					\
	}								\
} while (/*CONSTCOND*/0)

