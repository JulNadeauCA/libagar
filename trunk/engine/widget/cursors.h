/*	$Csoft$	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_CURSORS_H_
#define _AGAR_WIDGET_CURSORS_H_
#include "begin_code.h"

extern SDL_Cursor *fill_cursor, *erase_cursor;

__BEGIN_DECLS
void cursors_init(void);
void cursors_destroy(void);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_CURSORS_H_ */
