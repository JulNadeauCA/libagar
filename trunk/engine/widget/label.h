/*	$Csoft: label.h,v 1.25 2004/09/18 06:37:43 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_LABEL_H_
#define _AGAR_WIDGET_LABEL_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

#define LABEL_MAX		128
#define LABEL_MAX_POLLPTRS	32

enum label_type {
	LABEL_STATIC,		/* Static text */
	LABEL_POLLED,		/* Polling (thread unsafe) */
	LABEL_POLLED_MT		/* Polling (thread safe) */
};

struct label {
	struct widget wid;
	enum label_type	type;
	pthread_mutex_t	lock;
	int surface;
	int prew, preh;
	struct {
		char fmt[LABEL_MAX];
		void *ptrs[LABEL_MAX_POLLPTRS];
		int nptrs;
		pthread_mutex_t	*lock;
	} poll;
};

__BEGIN_DECLS
struct label *label_new(void *, enum label_type, const char *, ...);
__inline__ void label_static(void *, const char *);
void		label_staticf(void *, const char *, ...);

void	 label_init(struct label *, enum label_type, const char *);
void 	 label_destroy(void *);
void	 label_draw(void *);
void	 label_scale(void *, int, int);
void	 label_printf(struct label *, const char *, ...)
	     FORMAT_ATTRIBUTE(printf, 2, 3)
	     NONNULL_ATTRIBUTE(2);
void	 label_set_surface(struct label *, SDL_Surface *);
void	 label_prescale(struct label *, const char *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_LABEL_H_ */
