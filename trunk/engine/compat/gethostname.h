/*	$Csoft$	*/
/*	Public domain	*/

#if defined(__GNUC__) && defined(_SGI_SOURCE)
/* IRIX gcc 2.95 hack. */
extern int gethostname(char *, size_t);
#else
# include <unistd.h>
#endif

