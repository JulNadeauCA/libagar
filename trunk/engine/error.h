/*	$Csoft: error.h,v 1.11 2002/12/26 07:11:14 vedge Exp $	*/
/*	Public domain	*/

#ifdef __GNUC__
# define warning(fmt, args...) \
	printf("%s: " fmt, __FUNCTION__ , ##args)
# ifdef DEBUG
#  define fatal(fmt, args...)						\
	do {								\
		fprintf(stderr, "%s: " fmt, __FUNCTION__ , ##args);	\
		abort();						\
	} while (0)
# endif
#else
# define warning	printf
# define fatal		printf
#endif

#define Free(p) do {		\
	if ((p) != NULL) {	\
		free((p));	\
	}			\
} while (0)

char	*Strdup(const char *);
void	*emalloc(size_t);
void	*erealloc(void *, size_t);

const char	*error_get(void);
void		 error_set(const char *, ...);

#define Read(fd, buf, size) do {					\
	ssize_t _Read_rv;						\
									\
	_Read_rv = read((fd), (buf), (size));				\
	if (_Read_rv == -1) { 						\
		fatal("read(%ld): %s\n", (long)size, strerror(errno));	\
	}								\
	if (_Read_rv != size) {						\
		fatal("read(%ld): read %ld bytes\n", (long)size,	\
		    (long)_Read_rv);					\
	}								\
} while (0)

#define Write(fd, buf, size) do {					\
	ssize_t _Write_rv;						\
									\
	_Write_rv = write((fd), (buf), (size));				\
	if (_Write_rv == -1) {						\
		fatal("write(%ld): %s\n", (long)size, strerror(errno));	\
	}								\
	if (_Write_rv != (size)) {					\
		fatal("write(%ld): wrote %ld bytes\n", (long)size,	\
		    (long)_Write_rv);					\
	}								\
} while (0)

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
} while (0)

void	Asprintf(char **, const char *, ...);
#define Vasprintf(msg, fmt, args) do {				\
	va_start((args), (fmt));				\
	if (vasprintf((msg), (fmt), (args)) == -1) {		\
		fatal("vasprintf: %s\n", strerror(errno));	\
	}							\
	va_end((args));						\
} while (0)

