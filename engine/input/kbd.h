/*	$Csoft$	*/
/*	Public domain	*/

#include "begin_code.h"

struct kbd {
	struct input in;

	int	index;
};

__BEGIN_DECLS
struct kbd	*kbd_new(int);
__END_DECLS

#include "close_code.h"
