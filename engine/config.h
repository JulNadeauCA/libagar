/*	$Csoft: config.h,v 1.8 2002/09/02 08:12:14 vedge Exp $	*/
/*	Public domain	*/

/* flags */
#define CONFIG_FONT_CACHE	0x01	/* Cache common glyphs */
#define CONFIG_FULLSCREEN	0x02	/* Try full-screen mode */
#define CONFIG_ASYNCBLIT	0x04	/* Asynchronous screen updates */

/* widgets.flags */
#define CONFIG_REGION_BORDERS	0x01	/* Region borders always visible */
#define CONFIG_WINDOW_ANYSIZE	0x02	/* Show resolution on resize */

struct config {
	struct	 object obj;
	struct {
		struct	window *settings;
		struct	window *algorithm_sw;
	} windows;
};

extern struct config *config;

struct config	*config_new(void);
void		 config_init(struct config *);
int		 config_load(void *, int);
int		 config_save(void *, int);
void		 config_destroy(void *);

void		 config_init_wins(struct config *);
void		 config_apply(int, union evarg *);

int		 config_int(const char *);
char		*config_string(const char *);

