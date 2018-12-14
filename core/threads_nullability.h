/*	Public domain	*/
/*
 * Define specific nullability attributes for pthreads types,
 * which may or may not be pointers.
 */
#ifndef __has_feature
#define __has_feature(x) 0
#endif

#if (defined(__clang__) && __has_feature(nullability))
# include <agar/config/have_pthread_mutex_t_pointer.h>
# ifdef HAVE_PTHREAD_MUTEX_T_POINTER
#   define _Nonnull_Mutex          _Nonnull
#   define _Nullable_Mutex         _Nullable
#   define _Null_unspecified_Mutex _Null_unspecified
# else
#  define _Nonnull_Mutex
#  define _Nullable_Mutex
#  define _Null_unspecified_Mutex
# endif
# include <agar/config/have_pthread_cond_t_pointer.h>
# ifdef HAVE_PTHREAD_COND_T_POINTER
#  define _Nonnull_Cond          _Nonnull
#  define _Nullable_Cond         _Nullable
#  define _Null_unspecified_Cond _Null_unspecified
# else
#  define _Nonnull_Cond
#  define _Nullable_Cond
#  define _Null_unspecified_Cond
# endif
# include <agar/config/have_pthread_t_pointer.h>
# ifdef HAVE_PTHREAD_T_POINTER
#  define _Nonnull_Thread          _Nonnull
#  define _Nullable_Thread         _Nullable
#  define _Null_unspecified_Thread _Null_unspecified
# else
#  define _Nonnull_Thread
#  define _Nullable_Thread
#  define _Null_unspecified_Thread
# endif
#else /* !(__clang__ with nullability) */
# define _Nonnull_Mutex
# define _Nonnull_Cond
# define _Nonnull_Thread
# define _Nullable_Mutex
# define _Nullable_Cond
# define _Nullable_Thread
# define _Null_unspecified_Mutex
# define _Null_unspecified_Cond
# define _Null_unspecified_Thread
#endif
