/*	$Csoft: select.h,v 1.6 2003/03/25 13:48:05 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

struct select {
	struct tool	tool;
};

__BEGIN_DECLS
extern DECLSPEC void	 select_init(void *);
__END_DECLS

#include "close_code.h"
