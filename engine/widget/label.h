/*	$Csoft: label.h,v 1.22 2003/06/18 00:47:04 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_LABEL_H_
#define _AGAR_WIDGET_LABEL_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

#define LABEL_MAX		128
#define LABEL_FORMAT_MAX	64
#define LABEL_POLL_MAX		32

enum label_type {
	LABEL_STATIC,
	LABEL_POLLED
};

struct label {
	struct widget wid;

	enum label_type	type;
	
	pthread_mutex_t	 lock;
	SDL_Surface	*surface;
	int		 prew, preh;

	struct {
		char		 fmt[LABEL_FORMAT_MAX];
		void		*ptrs[LABEL_POLL_MAX];
		int		 nptrs;
		pthread_mutex_t	*lock;
	} poll;
};

__BEGIN_DECLS
struct label	*label_new(void *, const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 2, 3)
		     NONNULL_ATTRIBUTE(2);
struct label	*label_polled_new(void *, pthread_mutex_t *, const char *, ...)
		     NONNULL_ATTRIBUTE(3);

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
