/*	$Csoft: label.h,v 1.18 2003/04/25 09:47:10 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_LABEL_H_
#define _AGAR_WIDGET_LABEL_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

#define LABEL_MAX_LENGTH	1024
#define LABEL_MAX_POLLITEMS	32

enum label_type {
	LABEL_STATIC,
	LABEL_POLLED
};

struct label {
	struct widget	wid;
	enum label_type	type;
	struct {
		char		*caption;
		SDL_Surface	*surface;
		pthread_mutex_t	 lock;
	} text;
	struct {
		char		*fmt;
		void		*ptrs[LABEL_MAX_POLLITEMS];
		int		 nptrs;
		pthread_mutex_t	*lock;
	} poll;
	enum {
		LABEL_LEFT,
		LABEL_CENTER,
		LABEL_RIGHT
	} justify;
};

__BEGIN_DECLS
extern DECLSPEC struct label	*label_new(struct region *, int, int,
				           const char *, ...);
extern DECLSPEC struct label	*label_polled_new(struct region *, int, int,
				           pthread_mutex_t *,
					   const char *, ...);
extern DECLSPEC void		 label_init(struct label *, enum label_type,
				            const char *, int, int);
extern DECLSPEC void 		 label_destroy(void *);
extern DECLSPEC void		 label_draw(void *);
extern DECLSPEC void		 label_printf(struct label *, const char *,
				              ...);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_LABEL_H_ */
