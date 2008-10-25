/*	Public domain	*/

#ifndef _AGAR_CORE_PUBLIC_H_
#define _AGAR_CORE_PUBLIC_H_
#include <agar/core/core_begin.h>

#include <agar/core/core_init.h>
#include <agar/core/error.h>
#include <agar/core/string_compat.h>

#ifdef _USE_AGAR_STD
# include <agar/core/snprintf.h>
# include <agar/core/vsnprintf.h>
# include <agar/core/vasprintf.h>
#endif

#include <agar/core/data_source.h>
#include <agar/core/load_den.h>
#include <agar/core/load_integral.h>
#include <agar/core/load_real.h>
#include <agar/core/load_string.h>
#include <agar/core/load_version.h>

#include <agar/core/version.h>
#include <agar/core/object.h>
#include <agar/core/config.h>
#include <agar/core/rcs.h>
#include <agar/core/file.h>
#include <agar/core/dir.h>
#include <agar/core/dso.h>

#include <agar/core/core_close.h>
#endif
