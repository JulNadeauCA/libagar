/*	$Csoft: position.h,v 1.2 2003/10/13 23:49:00 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

struct position {
	struct tool tool;

	int	 speed;			/* Movement/scrolling increment */
	int	 center_view;		/* Center view around? */
	int	 soft_motion;		/* Soft-scrolling */
	int	 pass_through;		/* Ignore node movement restrictions */

	void	*obj;			/* Object to position */
	void	*submap;		/* Object submap to display */
	void	*input_dev;		/* Input device to control object */
};

__BEGIN_DECLS
void	 position_init(void *);
__END_DECLS

#include "close_code.h"
