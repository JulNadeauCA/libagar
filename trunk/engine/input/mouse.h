/*	$Csoft$	*/
/*	Public domain	*/

#include "begin_code.h"

struct mouse {
	struct input in;

	int	index;
};

__BEGIN_DECLS
struct mouse	*mouse_new(int);
__END_DECLS

#include "close_code.h"
