/*	$Csoft: error.h,v 1.13 2003/02/17 11:51:00 vedge Exp $	*/
/*	Public domain	*/

#ifdef __GNUC__
# define warning(fmt, args...) \
	printf("%s: " fmt, __FUNCTION__ , ##args)
# define fatal(fmt, args...)						\
	do {								\
		fprintf(stderr, "%s: " fmt, __FUNCTION__ , ##args);	\
		fprintf(stderr, "\n");					\
		abort();						\
	} while (0)
#else
# define warning	printf
# define fatal		error_fatal
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
void		 error_fatal(const char *, ...);

ssize_t	Write(int, const void *, size_t);
ssize_t	Pwrite(int, const void *, size_t, off_t);
ssize_t	Read(int, void *, size_t);
ssize_t	Pread(int, void *, size_t, off_t);
off_t	Lseek(int, off_t, int);

void	Asprintf(char **, const char *, ...);
#define Vasprintf(msg, fmt, args) do {				\
	va_start((args), (fmt));				\
	if (vasprintf((msg), (fmt), (args)) == -1) {		\
		fatal("vasprintf: %s\n", strerror(errno));	\
	}							\
	va_end((args));						\
} while (0)

