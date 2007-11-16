/*	Public domain	*/

#ifndef _AGAR_WIDGET_ICON_H_
#define _AGAR_WIDGET_ICON_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#include <gui/text.h>
#else
#include <agar/gui/widget.h>
#include <agar/gui/text.h>
#endif

#include "begin_code.h"

struct ag_window;
struct ag_socket;

typedef struct ag_icon {
	struct ag_widget wid;
	Uint flags;
	int surface;			/* Icon surface */
	struct ag_window *wDND;		/* For drag/drop */
	struct ag_socket *sock;		/* Back pointer to socket */
} AG_Icon;

__BEGIN_DECLS
extern const AG_WidgetClass agIconClass;

AG_Icon *AG_IconNew(void);
AG_Icon *AG_IconFromSurface(SDL_Surface *);
AG_Icon *AG_IconFromBMP(const char *);

void    AG_IconSetPadding(AG_Icon *, int, int, int, int);
#define	AG_IconSetPaddingLeft(b,v)   AG_IconSetPadding((b),(v),-1,-1,-1)
#define	AG_IconSetPaddingRight(b,v)  AG_IconSetPadding((b),-1,(v),-1,-1)
#define AG_IconSetPaddingTop(b,v)    AG_IconSetPadding((b),-1,-1,(v),-1)
#define	AG_IconSetPaddingBottom(b,v) AG_IconSetPadding((b),-1,-1,-1,(v))
void	AG_IconSetSurface(AG_Icon *, SDL_Surface *);
void	AG_IconSetSurfaceNODUP(AG_Icon *, SDL_Surface *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_ICON_H_ */
