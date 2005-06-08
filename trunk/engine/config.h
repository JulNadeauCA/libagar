/*	$Csoft: config.h,v 1.21 2003/10/11 04:31:34 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

typedef struct ag_config {
	struct ag_object obj;
	AG_Window *window;
	pthread_mutex_t	lock;
	char *save_path;			/* Data file save path */
} AG_Config;

extern AG_Config *agConfig;

__BEGIN_DECLS
void AG_ConfigInit(AG_Config *);
int  AG_ConfigLoad(void *, AG_Netbuf *);
int  AG_ConfigSave(void *, AG_Netbuf *);
void AG_ConfigWindow(AG_Config *, u_int);
int  AG_ConfigFile(const char *, const char *, const char *, char *, size_t)
		   BOUNDED_ATTRIBUTE(__string__, 4, 5);
__END_DECLS

#include "close_code.h"

