/*	$Csoft: event.h,v 1.15 2003/03/20 01:17:33 vedge Exp $	*/
/*	Public domain	*/

#include <config/floating_point.h>

#include "begin_code.h"

typedef union evarg {
	void	*p;
	char	*s;
	int	 i;
	char	 c;
	long int li;
#ifdef FLOATING_POINT
	double	 f;
#endif
} *evargs;

#define EVENT_MAX_ARGS	16

struct event {
	char		*name;
	int		 flags;
#define	EVENT_ASYNC	0x01	/* Event handler runs in own thread */
	union evarg	 argv[EVENT_MAX_ARGS];
	int		 argc;
	void		(*handler)(int, union evarg *);
	TAILQ_ENTRY(event) events;
};

extern int	 event_idle;			/* Enable idling? */

__BEGIN_DECLS
extern DECLSPEC void		 event_loop(void);
extern DECLSPEC struct event	*event_new(void *, char *,
				     void (*)(int, union evarg *),
				     const char *, ...);
extern DECLSPEC void		 event_post(void *, char *, const char *, ...);
extern DECLSPEC void		 event_forward(void *, char *, int,
				     union evarg *);
#ifdef DEBUG
extern DECLSPEC struct window	*event_show_fps_counter(void);
#endif
__END_DECLS

#include "close_code.h"
