/*	$Csoft: config.h,v 1.7 2002/08/17 22:17:32 vedge Exp $	*/
/*	Public domain	*/

struct config {
	struct	 object obj;

	/* Read-only */
	struct {
		struct	window *settings;
		struct	window *algorithm_sw;
	} windows;

	/* Read-write, thread-safe */
	Uint32	 flags;	
#define CONFIG_FONT_CACHE	0x01	/* Cache common glyphs */
#define CONFIG_FULLSCREEN	0x02	/* Try full-screen mode */
#define CONFIG_ASYNCBLIT	0x04	/* Asynchronous screen updates */

	struct {
		char	*data_path;		/* Path to data files */
		char	*user_data_dir;		/* User data file directory */
		char	*sys_data_dir;		/* System data directory */
	} path;

	struct {
		int	w, h, bpp;
	} view;

	/* Read-write, thread-safe */
	Uint32	 widget_flags;
#define CONFIG_REGION_BORDERS	0x01	/* Region borders always visible */
#define CONFIG_WINDOW_ANYSIZE	0x02	/* Show resolution on resize */

	pthread_mutex_t lock;
};

extern struct config *config;

struct config	*config_new(void);
void		 config_init(struct config *);
int		 config_load(void *, int);
int		 config_save(void *, int);
void		 config_destroy(void *);

void		 config_init_wins(struct config *);
void		 config_apply(int, union evarg *);

