/*	$Csoft: event.h,v 1.17 2003/04/26 04:42:44 vedge Exp $	*/
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

#define EVENT_ARGS_MAX	16

struct event {
	char		*name;
	int		 flags;
#define	EVENT_ASYNC		0x01	/* Event handler runs in own thread */
#define EVENT_FORWARD_CHILDREN	0x02	/* Forward to all descendents */
	union evarg	 argv[EVENT_ARGS_MAX];
	int		 argc;
	void		(*handler)(int, union evarg *);
	TAILQ_ENTRY(event) events;
};

extern int event_idle;		/* Enable idling? */

__BEGIN_DECLS
extern DECLSPEC struct event	*event_new(void *, const char *,
				     void (*)(int, union evarg *),
				     const char *, ...);

extern DECLSPEC void	event_loop(void);
extern DECLSPEC int	event_post(void *, const char *, const char *, ...);
extern DECLSPEC void	event_forward(void *, const char *, int, union evarg *);

#ifdef DEBUG
extern DECLSPEC struct window	*event_show_fps_counter(void);
#endif
__END_DECLS

#include "close_code.h"
