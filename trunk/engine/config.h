/*	$Csoft: config.h,v 1.4 2002/06/22 16:05:37 vedge Exp $	*/
/*	Public domain	*/

struct config {
	struct	 object obj;

	/* Read-only */
	struct	 window *settings_win;	/* Settings window */

	/* Read-write, thread-safe */
	Uint32	 flags;	
#define CONFIG_FONT_CACHE	0x01	/* Cache common glyphs */
#define CONFIG_FULLSCREEN	0x02	/* Try full-screen mode */
#define CONFIG_ASYNCBLIT	0x04	/* Asynchronous screen updates */

	struct {
		int	w, h, bpp;
	} view;

	/* Read-write, thread-safe */
	Uint32	 widget_flags;
#define CONFIG_REGION_BORDERS	0x01	/* Region borders always visible */

	pthread_mutex_t lock;
};

extern struct config *config;

struct config	*config_new(void);
void		 config_init(struct config *);
int		 config_load(void *, int);
int		 config_save(void *, int);
void		 config_destroy(void *);

void		 config_window(struct config *);
void		 config_apply(struct config *);

