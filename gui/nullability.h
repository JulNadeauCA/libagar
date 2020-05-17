#ifndef __has_feature
#define __has_feature(x) 0
#endif
#if !(defined(__clang__) && __has_feature(nullability))
# define _Nonnull
# define _Nullable
# define _Null_unspecified
# define _AGAR_GUI_DEFINED_NULLABILITY
#endif
