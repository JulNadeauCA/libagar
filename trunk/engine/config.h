/*	$Csoft: config.h,v 1.2 2002/06/01 02:38:42 vedge Exp $	*/
/*	Public domain	*/

struct config {
	struct	 object obj;

	/* Read-only */
	struct	 window *settings_win;	/* Settings window */

	/* Read-write, thread-safe */
	Uint32	 flags;	
#define CONFIG_FONT_CACHE	0x0001	/* Cache common glyphs */
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

