/*	$Csoft$	*/

struct config {
	struct	 object obj;

	Uint32	 flags;	
#define CONFIG_FONT_CACHE	0x0001	/* Cache common glyphs */

	pthread_mutex_t lock;	/* Lock on whole structure */
};

extern struct config *config;

struct config	*config_new(void);
void		 config_init(struct config *);
int		 config_load(void *, int);
int		 config_save(void *, int);
void		 config_destroy(void *);

void		 config_dialog(void);
void		 config_apply(struct config *);

