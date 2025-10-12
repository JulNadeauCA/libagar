#ifdef _AGARIB_H_
#error Nested inclusion of <agarib.h>
#endif
#define _AGARIB_H_

#include <agar/core.h>
#include <agar/gui.h>

#if !defined(_)
# include <config/enable_nls.h>
# ifdef ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext("agarib",String)
#  ifdef dgettext_noop
#   define N_(String) dgettext_noop("agarib",String)
#  else
#   define N_(String) (String)
#  endif
#  define _AGARIB_DEFINED_NLS
# else
#  undef _
#  undef N_
#  undef ngettext
#  define _(String) (String)
#  define N_(String) (String)
#  define ngettext(Singular,Plural,Number) ((Number==1)?(Singular):(Plural))
#  define _AGARIB_DEFINED_NLS
# endif
#endif /* !defined(_) */
