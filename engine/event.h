/*	$Csoft: event.h,v 1.9 2002/09/11 08:51:10 vedge Exp $	*/
/*	Public domain	*/

typedef union evarg {
	void	*p;
	char	*s;
	char	 c;
	int	 i;
	long int li;
	double	 f;
} *evargs;

#define EVENT_MAXFPS	100
#define EVENT_MAXARGS	16

struct event {
	char	*name;
	int	 flags;
#define	EVENT_ASYNC	0x01	/* Event handler runs in own thread */
	union	 evarg argv[EVENT_MAXARGS];
	int	 argc;
	void	(*handler)(int, union evarg *);
	TAILQ_ENTRY(event) events;
};

/* SDL user events */
enum {
	USER_WINDOW_EVENT
};

void	 event_loop(void);
void	 event_new(void *, char *, void (*)(int, union evarg *),
	     const char *, ...);
void	 event_post(void *, char *, const char *fmt, ...);
void	 event_forward(void *, char *, int, union evarg *);

