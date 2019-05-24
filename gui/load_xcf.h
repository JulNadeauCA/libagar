/*	Public domain	*/

#include <agar/config/ag_serialization.h>
#ifdef AG_SERIALIZATION

#include <agar/gui/begin.h>
__BEGIN_DECLS
int AG_XCFLoad(AG_DataSource *_Nonnull, AG_Offset,
               void (*_Nonnull)(AG_Surface *_Nonnull, const char *_Nonnull,
	                        void *_Nullable),
	       void *_Nullable);
__END_DECLS
#include <agar/gui/close.h>

#endif /* AG_SERIALIZATION */
