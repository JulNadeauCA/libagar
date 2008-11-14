/*	Public domain	*/

#ifdef AG_DEBUG
# undef AG_DEBUG
#endif
#ifdef AG_LOCKDEBUG
# undef AG_LOCKDEBUG
#endif
#ifdef AG_THREADS
# undef AG_THREADS
#endif

#ifdef _AGAR_HAVE_SYS_TYPES_H_
# undef _AGAR_HAVE_SYS_TYPES_H_
# undef _MK_HAVE_SYS_TYPES_H
#endif
#ifdef _AGAR_HAVE_64BIT_H_
# undef _AGAR_HAVE_64BIT_H_
# undef HAVE_64BIT
#endif
#ifdef _AGAR_HAVE_LONG_DOUBLE_H_
# undef _AGAR_HAVE_LONG_DOUBLE_H_
# undef HAVE_LONG_DOUBLE
#endif
#ifdef _AGAR_HAVE_STDLIB_H_
# undef _AGAR_HAVE_STDLIB_H_
# undef _MK_HAVE_STDLIB_H
#endif
#ifdef _AGAR_HAVE_UNISTD_H_
# undef _AGAR_HAVE_UNISTD_H_
# undef _MK_HAVE_UNISTD_H
#endif
#ifdef _AGAR_HAVE_UNSIGNED_TYPEDEFS_
# undef _AGAR_HAVE_UNSIGNED_TYPEDEFS_
# undef _MK_HAVE_UNSIGNED_TYPEDEFS
#endif
