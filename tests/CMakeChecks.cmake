# Public domain
#
# Do not edit!
# This file was generated from configure.in by BSDBuild 3.2.
#
# To regenerate this file, get the latest BSDBuild release from
# https://bsdbuild.hypertriton.com/, and use the command:
#
#    $ mkconfigure --output-cmake=CMakeChecks.cmake < configure.in > /dev/null
#
# or alternatively:
#
#    $ make configure
#

# Save a C definition (boolean) to ${CONFIG_DIR}.
macro(BB_Save_Define arg)
	string(TOLOWER "${arg}" arg_lower)
	file(WRITE "${CONFIG_DIR}/${arg_lower}.h" "#ifndef ${arg}
#define ${arg} \"yes\"
#endif
")
	file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/Makefile.config" "${arg}=yes
")
endmacro()

# Save a C definition (with a string literal value) to ${CONFIG_DIR}.
macro(BB_Save_Define_Value arg val)
	string(TOLOWER "${arg}" arg_lower)
	file(WRITE "${CONFIG_DIR}/${arg_lower}.h" "#ifndef ${arg}
#define ${arg} \"${val}\"
#endif
")
	file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/Makefile.config" "${arg}=\"${val}\"
")
endmacro()

# Save a C definition (with an unquoted literal value) to ${CONFIG_DIR}.
macro(BB_Save_Define_Value_Bare arg val)
	string(TOLOWER "${arg}" arg_lower)
	file(WRITE "${CONFIG_DIR}/${arg_lower}.h" "#ifndef ${arg}
#define ${arg} ${val}
#endif
")
	file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/Makefile.config" "${arg}=\"${val}\"
")
endmacro()

# Save a C undefinition to ${CONFIG_DIR}.
macro(BB_Save_Undef arg)
	string(TOLOWER "${arg}" arg_lower)
	file(WRITE "${CONFIG_DIR}/${arg_lower}.h" "#undef ${arg}
")
	file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/Makefile.config" "${arg}=no
")
endmacro()

# Save the value of a make variable to Makefile.config.
macro(BB_Save_MakeVar arg val)
	file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/Makefile.config" "${arg}=${val}
")
endmacro()

# Set the per-platform definition expected by BSDBuild modules.
macro(BB_Detect_Platform)
	if(WIN32)
		if(NOT WINDOWS)
			set(WINDOWS TRUE)
		endif()
	elseif(UNIX AND NOT APPLE)
		if(CMAKE_SYSTEM_NAME MATCHES ".*Linux")
			set(LINUX TRUE)
		elseif(CMAKE_SYSTEM_NAME MATCHES "kFreeBSD.*")
			set(FREEBSD TRUE)
		elseif(CMAKE_SYSTEM_NAME MATCHES "kNetBSD.*|NetBSD.*")
			set(NETBSD TRUE)
		elseif(CMAKE_SYSTEM_NAME MATCHES "kOpenBSD.*|OpenBSD.*")
			set(OPENBSD TRUE)
		elseif(CMAKE_SYSTEM_NAME MATCHES ".*GNU.*")
			set(GNU TRUE)
		elseif(CMAKE_SYSTEM_NAME MATCHES ".*BSDI.*")
			set(BSDI TRUE)
		elseif(CMAKE_SYSTEM_NAME MATCHES "DragonFly.*|FreeBSD")
			set(FREEBSD TRUE)
		elseif(CMAKE_SYSTEM_NAME MATCHES "SYSV5.*")
			set(SYSV5 TRUE)
		elseif(CMAKE_SYSTEM_NAME MATCHES "Solaris.*")
			set(SOLARIS TRUE)
		elseif(CMAKE_SYSTEM_NAME MATCHES "HP-UX.*")
			set(HPUX TRUE)
		elseif(CMAKE_SYSTEM_NAME MATCHES "AIX.*")
			set(AIX TRUE)
		elseif(CMAKE_SYSTEM_NAME MATCHES "Minix.*")
			set(MINIX TRUE)
		endif()
	elseif(APPLE)
		if(CMAKE_SYSTEM_NAME MATCHES ".*Darwin.*")
			set(DARWIN TRUE)
		elseif(CMAKE_SYSTEM_NAME MATCHES ".*MacOS.*")
			set(MACOSX TRUE)
		elseif(CMAKE_SYSTEM_NAME MATCHES ".*tvOS.*")
			set(TVOS TRUE)
		elseif(CMAKE_SYSTEM_NAME MATCHES ".*iOS.*")
			set(IOS TRUE)
		endif()
	elseif(CMAKE_SYSTEM_NAME MATCHES "BeOS.*")
		set(BEOS TRUE)
	elseif(CMAKE_SYSTEM_NAME MATCHES "Haiku.*")
		set(HAIKU TRUE)
	endif()

	if(UNIX AND NOT APPLE AND NOT RISCOS)
		set(UNIX_SYS ON)
	else()
		set(UNIX_SYS OFF)
	endif()

	if(UNIX OR APPLE)
		set(UNIX_OR_MAC_SYS ON)
	else()
		set(UNIX_OR_MAC_SYS OFF)
	endif()
endmacro()

#
# From BSDBuild/cocoa.pm:
#
macro(Check_Cocoa)
	set(COCOA_CFLAGS "-DTARGET_API_MAC_CARBON -DTARGET_API_MAC_OSX -force_cpusubtype_ALL -fpascal-strings")
	set(COCOA_LIBS "-lobjc -Wl,-framework,Cocoa -Wl,-framework,OpenGL -Wl,-framework,IOKit")

	set(ORIG_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
	set(ORIG_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
	set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} ${COCOA_CFLAGS}")
	set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES} ${COCOA_LIBS}")
		
	check_objc_source_compiles("
#import <Cocoa/Cocoa.h>

int main(int argc, char *argv[]) { return (0); }
" HAVE_COCOA)
	if (HAVE_COCOA)
		BB_Save_Define(HAVE_COCOA)
	else()
		set(COCOA_CFLAGS "")
		set(COCOA_LIBS "")
		BB_Save_Undef(HAVE_COCOA)
	endif()

	set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
	set(CMAKE_REQUIRED_LIBRARIES ${ORIG_CMAKE_REQUIRED_LIBRARIES})

	BB_Save_MakeVar(COCOA_CFLAGS "${COCOA_CFLAGS}")
	BB_Save_MakeVar(COCOA_LIBS "${COCOA_LIBS}")
endmacro()

macro(Disable_Cocoa)
	set(HAVE_COCOA OFF)
	BB_Save_Undef(HAVE_COCOA)
	BB_Save_MakeVar(COCOA_CFLAGS "")
	BB_Save_MakeVar(COCOA_LIBS "")
endmacro()

#
# From BSDBuild/gettimeofday.pm:
#
macro(Check_Gettimeofday)
	check_c_source_compiles("
#include <sys/time.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
	struct timeval tv;
	int rv = gettimeofday(&tv, NULL);
	return (rv);
}
" HAVE_GETTIMEOFDAY)
	if (HAVE_GETTIMEOFDDAY)
		BB_Save_Define(HAVE_GETTIMEOFDAY)
	else()
		BB_Save_Undef(HAVE_GETTIMEOFDAY)
	endif()
endmacro()

#
# From BSDBuild/byte_order.pm:
#
include(TestBigEndian)
macro(Check_ByteOrder)
	TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
	if (IS_BIG_ENDIAN)
		BB_Save_Define(_MK_BIG_ENDIAN)
		BB_Save_Undef(_MK_LITTLE_ENDIAN)
	else()
		BB_Save_Define(_MK_LITTLE_ENDIAN)
		BB_Save_Undef(_MK_BIG_ENDIAN)
	endif()
endmacro()

#
# From BSDBuild/getuid.pm:
#
macro(Check_Getuid)
	check_c_source_compiles("
#include <sys/types.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
	uid_t uid = getuid();
	return (uid != 0);
}
" HAVE_GETUID)
	if (HAVE_GETUID)
		BB_Save_Define(HAVE_GETUID)
	else()
		BB_Save_Undef(HAVE_GETUID)
	endif()
endmacro()

#
# From BSDBuild/getpwuid.pm:
#
macro(Check_Getpwuid)
	check_c_source_compiles("
#include <string.h>
#include <sys/types.h>
#include <pwd.h>

int
main(int argc, char *argv[])
{
	struct passwd *pwd;
	uid_t uid = 0;

	pwd = getpwuid(uid);
	return (pwd != NULL && pwd->pw_dir != NULL);
}
" HAVE_GETPWUID)
	if (HAVE_GETPWUID)
		BB_Save_Define(HAVE_GETPWUID)
	else()
		BB_Save_Undef(HAVE_GETPWUID)
	endif()
endmacro()

#
# From BSDBuild/xbox.pm:
#
macro(Check_Xbox)
	check_c_source_compiles("
#include <xtl.h>
#ifndef _XBOX
# error undefined
#endif

int
main(int argc, char *argv[])
{
	return (0);
}
" HAVE_XBOX)
	if (HAVE_XBOX)
		BB_Save_Define(HAVE_XBOX)
	else()
		BB_Save_Undef(HAVE_XBOX)
	endif()
endmacro()

macro(Disable_Xbox)
	set(HAVE_XBOX OFF)
	BB_Save_Undef(HAVE_XBOX)
endmacro()

#
# From BSDBuild/limits_h.pm:
#
macro(Check_Limits_h)
	check_c_source_compiles("
#include <limits.h>

int main(int argc, char *argv[]) {
	int i = INT_MIN;
	unsigned u = 0;
	long l = LONG_MIN;
	unsigned long ul = 0;
	i = INT_MAX;
	u = UINT_MAX;
	l = LONG_MAX;
	ul = ULONG_MAX;
	return (i != INT_MAX || u != UINT_MAX || l != LONG_MAX || ul != LONG_MAX);
}
" _MK_HAVE_LIMITS_H)
	if (_MK_HAVE_LIMITS_H)
		BB_Save_Define(_MK_HAVE_LIMITS_H)
	else()
		BB_Save_Undef(_MK_HAVE_LIMITS_H)
	endif()
endmacro()

#
# From BSDBuild/snprintf.pm:
#
macro(Check_Snprintf)
	check_c_source_compiles("
#include <stdio.h>

int main(int argc, char *argv[])
{
	char buf[16];
	(void)snprintf(buf, sizeof(buf), \"foo\");
	return (0);
}
" HAVE_SNPRINTF)
	if (HAVE_SNPRINTF)
		BB_Save_Define(HAVE_SNPRINTF)
	else()
		BB_Save_Undef(HAVE_SNPRINTF)
	endif()
endmacro()

#
# From BSDBuild/portaudio.pm:
#
macro(Check_Portaudio)
	set(PORTAUDIO_CFLAGS "")
	set(PORTAUDIO_LIBS "")

	set(ORIG_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
	set(ORIG_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
	set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -I/usr/local/include")
	set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES} -L/usr/local/lib -lm -lpthread -lportaudio")

	CHECK_INCLUDE_FILE(portaudio.h HAVE_PORTAUDIO_H)
	if(HAVE_PORTAUDIO_H)
		check_c_source_compiles("
#include <stdio.h>
#include <portaudio.h>

int
main(int argc, char *argv[])
{
	int rv;

	if ((rv = Pa_Initialize()) != paNoError) {
		if (Pa_IsFormatSupported(NULL, NULL, 48000.0) != 0) {
			return (0);
		} else {
			return (rv);
		}
	} else {
		Pa_Terminate();
		return (0);
	}
}
" HAVE_PORTAUDIO)
		if(HAVE_PORTAUDIO)
			set(PORTAUDIO_CFLAGS "-I/usr/local/include")
			set(PORTAUDIO_LIBS "-L/usr/local/lib" "-lm" "-lpthread" "-lportaudio")
			BB_Save_Define(HAVE_PORTAUDIO)
		else()
			BB_Save_Undef(HAVE_PORTAUDIO)
		endif()
	else()
		set(HAVE_PORTAUDIO OFF)
		BB_Save_Undef(HAVE_PORTAUDIO)
	endif()

	set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
	set(CMAKE_REQUIRED_LIBRARIES ${ORIG_CMAKE_REQUIRED_LIBRARIES})

	BB_Save_MakeVar(PORTAUDIO_CFLAGS "${PORTAUDIO_CFLAGS}")
	BB_Save_MakeVar(PORTAUDIO_LIBS "${PORTAUDIO_LIBS}")
endmacro()

macro(Disable_Portaudio)
	set(HAVE_PORTAUDIO OFF)
	BB_Save_MakeVar(PORTAUDIO_CFLAGS "")
	BB_Save_MakeVar(PORTAUDIO_LIBS "")
	BB_Save_Undef(HAVE_PORTAUDIO)
endmacro()

#
# From BSDBuild/unistd_h.pm:
#
macro(Check_Unistd_h)
	check_c_source_compiles("
#include <sys/types.h>
#include <unistd.h>
int main(int argc, char *argv[]) {
	char buf;
	int rv, fdout=1;

	if ((rv = write(fdout, (void *)&buf, 1)) < 1) { return (1); }
	if ((rv = read(0, (void *)&buf, 1)) < 1) { return (1); }
	if (unlink(\"/tmp/foo\") != 0) { return (1); }
	return (0);
}
" _MK_HAVE_UNISTD_H)
	if (_MK_HAVE_UNISTD_H)
		BB_Save_Define(_MK_HAVE_UNISTD_H)
	else()
		BB_Save_Undef(_MK_HAVE_UNISTD_H)
	endif()
endmacro()

#
# From BSDBuild/sse.pm:
#
macro(Check_SSE)
	set(ORIG_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})

	set(CMAKE_REQUIRED_FLAGS "${ORIG_CMAKE_REQUIRED_FLAGS} -msse")
	check_c_source_compiles("
#include <xmmintrin.h>
#include <stdio.h>

#define MAXERR 1e-4

typedef union vec {
	float v[4];
	__m128 m128;
	struct { float x, y, z, pad; };
} MyVector __attribute__ ((aligned(16)));

int
main(int argc, char *argv[])
{
	MyVector a;
	__m128 v;

	a.x = 1.0f;
	a.y = 2.0f;
	a.z = 3.0f;
	v = _mm_set1_ps(1.0f);
	a.m128 = _mm_mul_ps(a.m128, v);
	return (0);
}
" HAVE_SSE)
	if (HAVE_SSE)
		set(SSE_CFLAGS "-msse")
		BB_Save_MakeVar(SSE_CFLAGS "${SSE_CFLAGS}")
		BB_Save_Define(HAVE_SSE)
	else()
		set(SSE_CFLAGS "")
		BB_Save_MakeVar(SSE_CFLAGS "")
		BB_Save_Undef(HAVE_SSE)
	endif()

	set(CMAKE_REQUIRED_FLAGS "${ORIG_CMAKE_REQUIRED_FLAGS} -msse2")
	check_c_source_compiles("
#include <emmintrin.h>

int
main(int argc, char *argv[])
{
	double a[4] __attribute__ ((aligned(16)));
	double b[4] __attribute__ ((aligned(16)));
	double rv;
	__m128d vec1, vec2;
	a[0] = 1.0f; a[1] = 2.0f; a[2] = 3.0f; a[3] = 4.0f;
	b[0] = 1.0f; b[1] = 2.0f; b[2] = 3.0f; b[3] = 4.0f;
	vec1 = _mm_load_pd(a);
	vec2 = _mm_load_pd(b);
	vec1 = _mm_xor_pd(vec1, vec2);
	_mm_store_sd(&rv, vec1);
	return (0);
}
" HAVE_SSE2)
	if (HAVE_SSE2)
		set(SSE2_CFLAGS "-msse2")
		BB_Save_MakeVar(SSE2_CFLAGS "${SSE2_CFLAGS}")
		BB_Save_Define(HAVE_SSE2)
	else()
		set(SSE2_CFLAGS "")
		BB_Save_MakeVar(SSE2_CFLAGS "")
		BB_Save_Undef(HAVE_SSE2)
	endif()

	set(CMAKE_REQUIRED_FLAGS "${ORIG_CMAKE_REQUIRED_FLAGS} -msse3")
	check_c_source_compiles("
#include <pmmintrin.h>

int
main(int argc, char *argv[])
{
	float a[4] __attribute__ ((aligned(16)));
	float b[4] __attribute__ ((aligned(16)));
	__m128 vec1, vec2;
	float rv;
	a[0] = 1.0f; a[1] = 2.0f; a[2] = 3.0f; a[3] = 4.0f;
	b[0] = 1.0f; b[1] = 2.0f; b[2] = 3.0f; b[3] = 4.0f;
	vec1 = _mm_load_ps(a);
	vec2 = _mm_load_ps(b);
	vec1 = _mm_mul_ps(vec1, vec2);
	vec1 = _mm_hadd_ps(vec1, vec1);
	vec1 = _mm_hadd_ps(vec1, vec1);
	_mm_store_ss(&rv, vec1);
	return (0);
}
" HAVE_SSE3)
	if (HAVE_SSE3)
		set(SSE3_CFLAGS "-msse3")
		BB_Save_MakeVar(SSE3_CFLAGS "${SSE3_CFLAGS}")
		BB_Save_Define(HAVE_SSE3)
	else()
		set(SSE3_CFLAGS "")
		BB_Save_MakeVar(SSE3_CFLAGS "")
		BB_Save_Undef(HAVE_SSE3)
	endif()

	set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
endmacro()

macro(Disable_SSE)
	set(SSE_CFLAGS "")
	set(SSE2_CFLAGS "")
	set(SSE3_CFLAGS "")
	BB_Save_MakeVar(SSE_CFLAGS "")
	BB_Save_MakeVar(SSE2_CFLAGS "")
	BB_Save_MakeVar(SSE3_CFLAGS "")
	BB_Save_Undef(HAVE_SSE)
	BB_Save_Undef(HAVE_SSE2)
	BB_Save_Undef(HAVE_SSE3)
endmacro()

#
# From BSDBuild/timerfd.pm:
#
macro(Check_Timerfd)
	check_c_source_compiles("
#include <sys/timerfd.h>

int
main(int argc, char *argv[])
{
	struct itimerspec its;
	int fd;

	if ((fd = timerfd_create(CLOCK_MONOTONIC, TFD_TIMER_ABSTIME)) != -1) {
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_nsec = 0L;
		its.it_value.tv_sec = 0;
		its.it_value.tv_nsec = 0L;
		return (timerfd_settime(fd, 0, &its, NULL) == -1);
	}
	return (1);
}
" HAVE_TIMERFD)
	if (HAVE_TIMERFD)
		BB_Save_Define(HAVE_TIMERFD)
	else()
		BB_Save_Undef(HAVE_TIMERFD)
	endif()
endmacro()

macro(Disable_Timerfd)
	BB_Save_Undef(HAVE_TIMERFD)
endmacro()

#
# From BSDBuild/getenv.pm:
#
macro(Check_Getenv)
	check_c_source_compiles("
#include <stdlib.h>

int
main(int argc, char *argv[])
{
	(void)getenv(\"PATH\");
	return (0);
}
" HAVE_GETENV)
	if (HAVE_GETENV)
		BB_Save_Define(HAVE_GETENV)
	else()
		BB_Save_Undef(HAVE_GETENV)
	endif()
endmacro()

#
# From BSDBuild/float_h.pm:
#
macro(Check_Float_h)
	check_c_source_compiles("
#include <float.h>

int main(int argc, char *argv[]) {
	float flt = 0.0f;
	double dbl = 0.0;

	flt += FLT_EPSILON;
	dbl += DBL_EPSILON;
	return (0);
}
" _MK_HAVE_FLOAT_H)
	if (_MK_HAVE_FLOAT_H)
		BB_Save_Define(_MK_HAVE_FLOAT_H)
	else()
		BB_Save_Undef(_MK_HAVE_FLOAT_H)
	endif()
endmacro()

#
# From BSDBuild/vsnprintf.pm:
#
macro(Check_Vsnprintf)
	check_c_source_compiles("
#include <stdio.h>
#include <stdarg.h>

static void
testfmt(const char *fmt, ...)
{
	char buf[16];
	va_list ap;
	va_start(ap, fmt);
	(void)vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
}
int
main(int argc, char *argv[])
{
	testfmt(\"foo\", 1, 2, 3);
	return (0);
}
" HAVE_VSNPRINTF)
	if (HAVE_VSNPRINTF)
		BB_Save_Define(HAVE_VSNPRINTF)
	else()
		BB_Save_Undef(HAVE_VSNPRINTF)
	endif()
endmacro()

#
# From BSDBuild/clock_win32.pm:
#
macro(Check_Clock_win32)
	set(ORIG_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})

	set(CLOCK_CFLAGS "")
	set(CLOCK_LIBS "")

	set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES} -lwinmm")
	check_c_source_compiles("
#ifdef _XBOX
#include <xtl.h>
#else
#include <windows.h>
#include <mmsystem.h>
#endif

int
main(int argc, char *argv[])
{
	DWORD t0;
#ifndef _XBOX
	timeBeginPeriod(1);
#endif
	t0 = timeGetTime();
	Sleep(1);
	return (t0 != 0) ? 0 : 1;
}
" HAVE_CLOCK_WIN32)
	if(HAVE_CLOCK_WIN32)
		BB_Save_Define(HAVE_CLOCK_WIN32)
		set(CLOCK_LIBS "-lwinmm")
	else()
		BB_Save_Undef(HAVE_CLOCK_WIN32)
	endif()

	BB_Save_MakeVar(CLOCK_CFLAGS "${CLOCK_CFLAGS}")
	BB_Save_MakeVar(CLOCK_LIBS "${CLOCK_LIBS}")

	set(CMAKE_REQUIRED_LIBRARIES ${ORIG_CMAKE_REQUIRED_LIBRARIES})
endmacro()

macro(Disable_Clock_win32)
	BB_Save_Undef(HAVE_CLOCK_WIN32)
endmacro()

#
# From BSDBuild/setenv.pm:
#
macro(Check_Setenv)
	check_c_source_compiles("
#include <stdlib.h>

int
main(int argc, char *argv[])
{
	(void)setenv(\"BSDBUILD_SETENV_TEST\", \"foo\", 1);
	unsetenv(\"BSDBUILD_SETENV_TEST\");
	return (0);
}
" HAVE_SETENV)
	if (HAVE_SETENV)
		BB_Save_Define(HAVE_SETENV)
	else()
		BB_Save_Undef(HAVE_SETENV)
	endif()
endmacro()

#
# From BSDBuild/getopt.pm:
#
macro(Check_Getopt)
	check_c_source_compiles("
#include <string.h>
#include <getopt.h>

int
main(int argc, char *argv[])
{
	int c, x = 0;
	while ((c = getopt(argc, argv, \"foo\")) != -1) {
		extern char *optarg;
		extern int optind, opterr, optopt;
		if (optarg != NULL) { x = 1; }
		if (optind > 0) { x = 2; }
		if (opterr > 0) { x = 3; }
		if (optopt > 0) { x = 4; }
	}
	return (x != 0);
}
" HAVE_GETOPT)
	if (HAVE_GETOPT)
		BB_Save_Define(HAVE_GETOPT)
	else()
		BB_Save_Undef(HAVE_GETOPT)
	endif()
endmacro()

#
# From BSDBuild/nanosleep.pm:
#
macro(Check_Nanosleep)
	check_c_source_compiles("
#include <time.h>

int
main(int argc, char *argv[])
{
	struct timespec rqtp, rmtp;
	int rv;

	rqtp.tv_sec = 1;
	rqtp.tv_nsec = 1000000;
	rv = nanosleep(&rqtp, &rmtp);
	return (rv == -1);
}
" HAVE_NANOSLEEP)
	if (HAVE_NANOSLEEP)
		BB_Save_Define(HAVE_NANOSLEEP)
	else()
		BB_Save_Undef(HAVE_NANOSLEEP)
	endif()
endmacro()

#
# From BSDBuild/jpeg.pm:
#
macro(Check_Jpeg)
	set(JPEG_CFLAGS "")
	set(JPEG_LIBS "")

	include(FindJPEG)
	if(JPEG_FOUND)
		set(HAVE_JPEG ON)

		foreach(jpegincdir ${JPEG_INCLUDE_DIRS})
			list(APPEND JPEG_CFLAGS "-I${jpegincdir}")
		endforeach()
		foreach(jpeglib ${JPEG_LIBRARIES})
			list(APPEND JPEG_LIBS "${jpeglib}")
		endforeach()
		BB_Save_Define(HAVE_JPEG)
	else()
		set(HAVE_JPEG OFF)
		BB_Save_Undef(HAVE_JPEG)
	endif()

	BB_Save_MakeVar(JPEG_CFLAGS "${JPEG_CFLAGS}")
	BB_Save_MakeVar(JPEG_LIBS "${JPEG_LIBS}")
endmacro()

macro(Disable_Jpeg)
	set(HAVE_JPEG OFF)
	BB_Save_Undef(HAVE_JPEG)
	BB_Save_MakeVar(JPEG_CFLAGS "")
	BB_Save_MakeVar(JPEG_LIBS "")
endmacro()

#
# From BSDBuild/vasprintf.pm:
#
macro(Check_Vasprintf)
	check_c_source_compiles("
#include <stdio.h>
#include <stdarg.h>

int
testprintf(const char *fmt, ...)
{
	va_list args;
	char *buf;

	va_start(args, fmt);
	if (vasprintf(&buf, \"%s\", args) == -1) {
		return (-1);
	}
	va_end(args);
	return (0);
}
int
main(int argc, char *argv[])
{
	return (testprintf(\"foo %s\", \"bar\"));
}
" HAVE_VASPRINTF)
	if (HAVE_VASPRINTF)
		BB_Save_Define(HAVE_VASPRINTF)
	else()
		BB_Save_Undef(HAVE_VASPRINTF)
	endif()
endmacro()

#
# From BSDBuild/stdlib_h.pm:
#
macro(Check_Stdlib_h)
	check_c_source_compiles("
#include <stdlib.h>
int main(int argc, char *argv[]) {
	void *foo = malloc(1);
	free(foo);
	return (0);
}
" _MK_HAVE_STDLIB_H)
	if (_MK_HAVE_STDLIB_H)
		BB_Save_Define(_MK_HAVE_STDLIB_H)
	else()
		BB_Save_Undef(_MK_HAVE_STDLIB_H)
	endif()
endmacro()

#
# From BSDBuild/mprotect.pm:
#
macro(Check_Mprotect)
	check_c_source_compiles("
#include <sys/mman.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
	void *p;
	int psz;
	char *buffer;

	psz = sysconf(_SC_PAGE_SIZE);
	if (psz == -1) {
		return (1);
	}

	posix_memalign(&buffer, psz, psz*4);
	if (buffer == NULL)
		return (1);

	mprotect(buffer + psz*2, psz, PROT_READ);
	return (0);
}
" HAVE_MPROTECT)
	if (HAVE_MPROTECT)
		BB_Save_Define(HAVE_MPROTECT)
	else()
		BB_Save_Undef(HAVE_MPROTECT)
	endif()
endmacro()

#
# From BSDBuild/csidl.pm:
#
macro(Check_Csidl)
	check_c_source_compiles("
#include <shlobj.h>

int
main(int argc, char *argv[])
{
	WCHAR path[MAX_PATH];

	if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path))) {
		return (0);
	} else {
		return (1);
	}
}
" HAVE_CSIDL)
	if (HAVE_CSIDL)
		BB_Save_Define(HAVE_CSIDL)
	else()
		BB_Save_Undef(HAVE_CSIDL)
	endif()
endmacro()

#
# From BSDBuild/siocgifconf.pm:
#
macro(Check_Siocgifconf)
	check_c_source_compiles("
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
int
main(int argc, char *argv[])
{
	char buf[4096];
	struct ifconf conf;
	struct ifreq *ifr;
	int sock;
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		return (1);
	}
	conf.ifc_len = sizeof(buf);
	conf.ifc_buf = (caddr_t)buf;
	if (ioctl(sock, SIOCGIFCONF, &conf) < 0) {
		return (1);
	}
#if !defined(_SIZEOF_ADDR_IFREQ)
#define _SIZEOF_ADDR_IFREQ sizeof
#endif
	for (ifr = (struct ifreq *)buf;
	     (char *)ifr < &buf[conf.ifc_len];
	     ifr = (struct ifreq *)((char *)ifr + _SIZEOF_ADDR_IFREQ(*ifr))) {
		if (ifr->ifr_addr.sa_family == AF_INET)
			return (1);
	}
	close(sock);
	return (0);
}
" HAVE_SIOCGIFCONF)
	if (HAVE_SIOCGIFCONF)
		BB_Save_Define(HAVE_SIOCGIFCONF)
	else()
		BB_Save_Undef(HAVE_SIOCGIFCONF)
	endif()
endmacro()

#
# From BSDBuild/signal.pm:
#
macro(Check_Signal)
	check_c_source_compiles("
#include <signal.h>

void sighandler(int sig) { }
int
main(int argc, char *argv[])
{
	signal(SIGTERM, sighandler);
	signal(SIGILL, sighandler);
	return (0);
}
" _MK_HAVE_SIGNAL)
	if (_MK_HAVE_SIGNAL)
		BB_Save_Define(_MK_HAVE_SIGNAL)
	else()
		BB_Save_Undef(_MK_HAVE_SIGNAL)
	endif()
endmacro()

#
# From BSDBuild/asprintf.pm:
#
macro(Check_Asprintf)
	check_c_source_compiles("
#include <stdio.h>

int
main(int argc, char *argv[])
{
	char *buf;
	if (asprintf(&buf, \"foo %s\", \"bar\") == 0) {
	    return (0);
	}
	return (1);
}
" HAVE_ASPRINTF)
	if (HAVE_ASPRINTF)
		BB_Save_Define(HAVE_ASPRINTF)
	else()
		BB_Save_Undef(HAVE_ASPRINTF)
	endif()
endmacro()

#
# From BSDBuild/strtoll.pm:
#
macro(Check_Strtoll)
	check_c_source_compiles("
#include <stdlib.h>

int
main(int argc, char *argv[])
{
	long long int lli;
	char *ep = NULL;
	char *foo = \"1234\";

	lli = strtoll(foo, &ep, 10);
	return (lli != 0);
}
" _MK_HAVE_STRTOLL)
	if (_MK_HAVE_STRTOLL)
		BB_Save_Define(_MK_HAVE_STRTOLL)
	else()
		BB_Save_Undef(_MK_HAVE_STRTOLL)
	endif()
endmacro()

#
# From BSDBuild/zlib.pm:
#
macro(Check_Zlib)
	set(ZLIB_CFLAGS "")
	set(ZLIB_LIBS "")

	include(FindZLIB)
	if(ZLIB_FOUND)
		set(HAVE_ZLIB ON)
		BB_Save_Define(HAVE_ZLIB)
		if(${ZLIB_INCLUDE_DIRS})
			set(ZLIB_CFLAGS "-I${ZLIB_INCLUDE_DIRS}")
		endif()
		set(ZLIB_LIBS "${ZLIB_LIBRARIES}")
	else()
		set(HAVE_ZLIB OFF)
		BB_Save_Undef(HAVE_ZLIB)
	endif()

	BB_Save_MakeVar(ZLIB_CFLAGS "${ZLIB_CFLAGS}")
	BB_Save_MakeVar(ZLIB_LIBS "${ZLIB_LIBS}")
endmacro()

macro(Disable_Zlib)
	set(HAVE_ZLIB OFF)
	BB_Save_Undef(HAVE_ZLIB)
	BB_Save_MakeVar(ZLIB_CFLAGS "")
	BB_Save_MakeVar(ZLIB_LIBS "")
endmacro()

#
# From BSDBuild/dyld.pm:
#
macro(Check_Dyld)
	set(ORIG_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})

	CHECK_INCLUDE_FILE(mach-o/dyld.h HAVE_MACH_O_DYLD_H)
	if(HAVE_MACH_O_DYLD_H)
		set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -DHAVE_MACH_O_DYLD_H")
		BB_Save_Define(HAVE_MACH_O_DYLD_H)
	else()
		BB_Save_Undef(HAVE_MACH_O_DYLD_H)
	endif()

	check_c_source_compiles("
#ifdef __APPLE__
# include <Availability.h>
# ifdef __MAC_OS_X_VERSION_MIN_REQUIRED
#  if __MAC_OS_X_VERSION_MIN_REQUIRED >= 1050
#   error \"deprecated in Leopard and later\"
#  endif
# endif
#endif

#ifdef HAVE_MACH_O_DYLD_H
#include <mach-o/dyld.h>
#endif

int
main(int argc, char *argv[])
{
	NSObjectFileImage img;
	NSObjectFileImageReturnCode rv;

	rv = NSCreateObjectFileImageFromFile(\"foo\", &img);
	return (rv == NSObjectFileImageSuccess);
}
" HAVE_DYLD)
	if (HAVE_DYLD)
		BB_Save_Define(HAVE_DYLD)

		check_c_source_compiles("
#ifdef HAVE_MACH_O_DYLD_H
#include <mach-o/dyld.h>
#endif
int
main(int argc, char *argv[])
{
	NSObjectFileImage img;
	NSObjectFileImageReturnCode rv;
	void *handle;

	rv = NSCreateObjectFileImageFromFile(\"foo\", &img);
	handle = (void *)NSLinkModule(img, \"foo\",
	    NSLINKMODULE_OPTION_RETURN_ON_ERROR|
		NSLINKMODULE_OPTION_NONE);
	if (handle == NULL) {
		NSLinkEditErrors errs;
		int n;
		const char *f, *s = NULL;
		NSLinkEditError(&errs, &n, &f, &s);
	}
	return (0);
}
" HAVE_DYLD_RETURN_ON_ERROR)
		if(HAVE_DYLD_RETURN_ON_ERROR)
			BB_Save_Define(HAVE_DYLD_RETURN_ON_ERROR)
		else()
			BB_Save_Undef(HAVE_DYLD_RETURN_ON_ERROR)
		endif()
	else()
		BB_Save_Undef(HAVE_DYLD)
		BB_Save_Undef(HAVE_DYLD_RETURN_ON_ERROR)
	endif()

	set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
endmacro()

macro(Disable_Dyld)
	BB_Save_Undef(HAVE_DYLD)
	BB_Save_Undef(HAVE_MACH_O_DYLD_H)
	BB_Save_Undef(HAVE_DYLD_RETURN_ON_ERROR)
endmacro()

#
# From BSDBuild/fontconfig.pm:
#
macro(Check_Fontconfig)
	set(FONTCONFIG_CFLAGS "")
	set(FONTCONFIG_LIBS "")

	include(FindFontconfig)
	if(Fontconfig_FOUND)
		set(HAVE_FONTCONFIG ON)

		if(Fontconfig_COMPILE_OPTIONS)
			foreach(fontconfigopt ${Fontconfig_COMPILE_OPTIONS})
				list(APPEND FONTCONFIG_CFLAGS ${fontconfigopt})
			endforeach()
		endif()
		foreach(fontconfigincdir ${Fontconfig_INCLUDE_DIRS})
			list(APPEND FONTCONFIG_CFLAGS "-I${fontconfigincdir}")
		endforeach()
		foreach(fontconfiglib ${Fontconfig_LIBRARIES})
			list(APPEND FONTCONFIG_LIBS "${fontconfiglib}")
		endforeach()
		BB_Save_Define(HAVE_FONTCONFIG)
	else()
		set(HAVE_FONTCONFIG OFF)
		BB_Save_Undef(HAVE_FONTCONFIG)
	endif()

	BB_Save_MakeVar(FONTCONFIG_CFLAGS "${FONTCONFIG_CFLAGS}")
	BB_Save_MakeVar(FONTCONFIG_LIBS "${FONTCONFIG_LIBS}")
endmacro()

macro(Disable_Fontconfig)
	set(HAVE_FONTCONFIG OFF)
	BB_Save_Undef(HAVE_FONTCONFIG)
	BB_Save_MakeVar(FONTCONFIG_CFLAGS "")
	BB_Save_MakeVar(FONTCONFIG_LIBS "")
endmacro()

#
# From BSDBuild/sdl.pm:
#
macro(Check_Sdl)
	set(SDL_CFLAGS "")
	set(SDL_LIBS "")

	set(SDL_BUILDING_LIBRARY ON)
	include(FindSDL)
	if(SDL_FOUND)
		set(HAVE_SDL ON)
		foreach(sdlincdir ${SDL_INCLUDE_DIRS})
			list(APPEND SDL_CFLAGS "-I${sdlincdir}")
		endforeach()

		foreach(sdllib ${SDL_LIBRARIES})
			list(APPEND SDL_LIBS "${sdllib}")
		endforeach()
		BB_Save_Define(HAVE_SDL)
	else()
		set(HAVE_SDL OFF)
		BB_Save_Undef(HAVE_SDL)
	endif()

	BB_Save_MakeVar(SDL_CFLAGS "${SDL_CFLAGS}")
	BB_Save_MakeVar(SDL_LIBS "${SDL_LIBS}")
endmacro()

macro(Disable_Sdl)
	set(HAVE_SDL OFF)
	BB_Save_Undef(HAVE_SDL)
	BB_Save_MakeVar(SDL_CFLAGS "")
	BB_Save_MakeVar(SDL_LIBS "")
endmacro()

#
# From BSDBuild/getpwnam_r.pm:
#
macro(Check_Getpwnam_r)
	check_c_source_compiles("
#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int
main(int argc, char *argv[])
{
	struct passwd pw, *res;
	char *buf;
	size_t bufSize;
	int rv;

	bufSize = sysconf(_SC_GETPW_R_SIZE_MAX);
	if (bufSize == -1) { bufSize = 16384; }
	if ((buf = malloc(bufSize)) == NULL) { return (1); }

	rv = getpwnam_r(\"foo\", &pw, buf, bufSize, &res);
	if (res == NULL) {
		return (rv == 0);
	}
	return (pw.pw_dir != NULL);
}
" HAVE_GETPWNAM_R)
	if (HAVE_GETPWNAM_R)
		BB_Save_Define(HAVE_GETPWNAM_R)
	else()
		BB_Save_Undef(HAVE_GETPWNAM_R)
	endif()
endmacro()

#
# From BSDBuild/shl_load.pm:
#
macro(Check_Shl_load)
	set(ORIG_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
	set(ORIG_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})

	set(SHL_LOAD_LIBS "-ldld")
	set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES} ${SHL_LOAD_LIBS}")

	CHECK_INCLUDE_FILE(dl.h HAVE_DL_H)
	if(HAVE_DL_H)
		set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -DHAVE_DL_H")
		BB_Save_Define(HAVE_DL_H)
	else()
		BB_Save_Undef(HAVE_DL_H)
	endif()

	check_c_source_compiles("
#include <string.h>
#ifdef HAVE_DL_H
#include <dl.h>
#endif

int
main(int argc, char *argv[])
{
	void *handle;
	void **p;

	handle = shl_load(\"foo.so\", BIND_IMMEDIATE, 0);
	(void)shl_findsym((shl_t *)&handle, \"foo\", TYPE_PROCEDURE, p);
	(void)shl_findsym((shl_t *)&handle, \"foo\", TYPE_DATA, p);
	shl_unload((shl_t)handle);
	return (handle != NULL);
}
" HAVE_SHL_LOAD)
	if(HAVE_SHL_LOAD)
		BB_Save_Define(HAVE_SHL_LOAD)

		set(DSO_LIBS "${DSO_LIBS} ${SHL_LOAD_LIBS}")
		BB_Save_MakeVar(DSO_LIBS "${DSO_LIBS}")
	else()
		BB_Save_Undef(HAVE_SHL_LOAD)
	endif()

	BB_Save_MakeVar(SHL_LOAD_LIBS "${SHL_LOAD_LIBS}")

	set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
	set(CMAKE_REQUIRED_LIBRARIES ${ORIG_CMAKE_REQUIRED_LIBRARIES})
endmacro()

macro(Disable_Shl_load)
	BB_Save_Undef(HAVE_SHL_LOAD)
	BB_Save_Undef(HAVE_DL_H)
	BB_Save_MakeVar(SHL_LOAD_LIBS "")
endmacro()

#
# From BSDBuild/winsock.pm:
#
macro(Check_Winsock)
	if(WINDOWS)
		BB_Save_Define(HAVE_WINSOCK1)
		BB_Save_Define(HAVE_WINSOCK2)
	else()
		BB_Save_Undef(HAVE_WINSOCK1)
		BB_Save_Undef(HAVE_WINSOCK2)
	endif()
endmacro()

macro(Disable_Winsock)
	BB_Save_Undef(HAVE_WINSOCK1)
	BB_Save_Undef(HAVE_WINSOCK2)
endmacro()

#
# From BSDBuild/sockopts.pm:
#
macro(Check_Setsockopts)

	check_c_source_compiles("
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>
int
main(int argc, char *argv[])
{
	int fd = 0, rv;
	struct timeval tv;
	socklen_t tvLen = sizeof(tv);
	tv.tv_sec = 1; tv.tv_usec = 0;
	rv = setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, tvLen);
	return (rv != 0);
}
" HAVE_SETSOCKOPT)
	if (HAVE_SETSOCKOPT)
		BB_Save_Define(HAVE_SETSOCKOPT)
	else()
		BB_Save_Undef(HAVE_SETSOCKOPT)
	endif()

	check_c_source_compiles("
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
int
main(int argc, char *argv[])
{
	int fd = 0, val = 1, rv;
	socklen_t valLen = sizeof(val);
	rv = setsockopt(fd, SOL_SOCKET, SO_OOBINLINE, &val, valLen);
	return (rv != 0);
}
" HAVE_SO_OOBINLINE)
	if (HAVE_SO_OOBINLINE)
		BB_Save_Define(HAVE_SO_OOBINLINE)
	else()
		BB_Save_Undef(HAVE_SO_OOBINLINE)
	endif()

	check_c_source_compiles("
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
int
main(int argc, char *argv[])
{
	int fd = 0, val = 1, rv;
	socklen_t valLen = sizeof(val);
	rv = setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &val, valLen);
	return (rv != 0);
}
" HAVE_SO_REUSEPORT)
	if (HAVE_SO_REUSEPORT)
		BB_Save_Define(HAVE_SO_REUSEPORT)
	else()
		BB_Save_Undef(HAVE_SO_REUSEPORT)
	endif()

	check_c_source_compiles("
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
int
main(int argc, char *argv[])
{
	int fd = 0, val = 1, rv;
	socklen_t valLen = sizeof(val);
	rv = setsockopt(fd, SOL_SOCKET, SO_TIMESTAMP, &val, valLen);
	return (rv != 0);
}
" HAVE_SO_TIMESTAMP)
	if (HAVE_SO_TIMESTAMP)
		BB_Save_Define(HAVE_SO_TIMESTAMP)
	else()
		BB_Save_Undef(HAVE_SO_TIMESTAMP)
	endif()

	check_c_source_compiles("
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
int
main(int argc, char *argv[])
{
	int fd = 0, val = 1, rv;
	socklen_t valLen = sizeof(val);
	rv = setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &val, valLen);
	return (rv != 0);
}
" HAVE_SO_NOSIGPIPE)
	if (HAVE_SO_NOSIGPIPE)
		BB_Save_Define(HAVE_SO_NOSIGPIPE)
	else()
		BB_Save_Undef(HAVE_SO_NOSIGPIPE)
	endif()

	check_c_source_compiles("
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
int
main(int argc, char *argv[])
{
	int fd = 0, rv;
	struct linger ling;
	socklen_t lingLen = sizeof(ling);
	ling.l_onoff = 1; ling.l_linger = 1;
	rv = setsockopt(fd, SOL_SOCKET, SO_LINGER, &ling, lingLen);
	return (rv != 0);
}
" HAVE_SO_LINGER)
	if (HAVE_SO_LINGER)
		BB_Save_Define(HAVE_SO_LINGER)
	else()
		BB_Save_Undef(HAVE_SO_LINGER)
	endif()

	check_c_source_compiles("
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
int
main(int argc, char *argv[])
{
	int fd = 0, rv;
	struct accept_filter_arg afa;
	socklen_t afaLen = sizeof(afa);
	afa.af_name[0] = 'A';
	afa.af_arg[0] = 'A';
	rv = setsockopt(fd, SOL_SOCKET, SO_ACCEPTFILTER, &afa, afaLen);
	return (rv != 0);
}
" HAVE_SO_ACCEPTFILTER)
	if (HAVE_SO_ACCEPTFILTER)
		BB_Save_Define(HAVE_SO_ACCEPTFILTER)
	else()
		BB_Save_Undef(HAVE_SO_ACCEPTFILTER)
	endif()


endmacro()

macro(Disable_Setsockopts)
	BB_Save_Undef(HAVE_SETSOCKOPT)
	BB_Save_Undef(HAVE_SO_OOBINLINE)
	BB_Save_Undef(HAVE_SO_REUSEPORT)
	BB_Save_Undef(HAVE_SO_TIMESTAMP)
	BB_Save_Undef(HAVE_SO_NOSIGPIPE)
	BB_Save_Undef(HAVE_SO_LINGER)
	BB_Save_Undef(HAVE_SO_ACCEPTFILTER)
endmacro()

#
# From BSDBuild/x11.pm:
#
macro(Check_X11)
	set(X11_CFLAGS "")
	set(X11_LIBS "")
	set(XINERAMA_CFLAGS "")
	set(XINERAMA_LIBS "")

	include(FindX11)
	if(X11_FOUND)
		set(HAVE_X11 ON)

		if(X11_INCLUDE_DIR)
			list(APPEND X11_CFLAGS "-I${X11_INCLUDE_DIR}")
		endif()
		foreach(x11lib ${X11_LIBRARIES})
			list(APPEND X11_LIBS "${x11lib}")
		endforeach()

		BB_Save_Define(HAVE_X11)

		if(X11_Xinerama_FOUND)
			if(X11_Xinerama_INCLUDE_PATH)
				list(APPEND XINERAMA_CFLAGS "-I${X11_Xinerama_INCLUDE_PATH}")
			endif()
			if(X11_Xinerama_LIB)
				list(APPEND XINERAMA_LIBS ${X11_Xinerama_LIB})
			endif()

			set(HAVE_XINERAMA ON)
			BB_Save_Define(HAVE_XINERAMA)
		else()
			set(HAVE_XINERAMA OFF)
			BB_Save_Undef(HAVE_XINERAMA)
		endif()

		set(ORIG_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
		set(ORIG_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
		set(CMAKE_REQUIRED_FLAGS ${X11_CFLAGS})
		set(CMAKE_REQUIRED_LIBRARIES ${X11_LIBS})

		check_c_source_compiles("
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
int main(int argc, char *argv[])
{
	Display *disp = XOpenDisplay(NULL);
	KeyCode kc = 0;
	KeySym ks = XkbKeycodeToKeysym(disp, kc, 0, 0);
	XCloseDisplay(disp);
	return (ks != NoSymbol);
}
" HAVE_XKB)
		if (HAVE_XKB)
			BB_Save_Define(HAVE_XKB)
		else()
			BB_Save_Undef(HAVE_XKB)
		endif()

		set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES} -lXxf86misc")
		check_c_source_compiles("
#include <X11/Xlib.h>
#include <X11/extensions/xf86misc.h>
int main(int argc, char *argv[])
{
	Display *disp = XOpenDisplay(NULL);
	int dummy, rv;
	rv = XF86MiscQueryExtension(disp, &dummy, &dummy);
	XCloseDisplay(disp);
	return (rv != 0);
}
" HAVE_XF86MISC)
		if (HAVE_XF86MISC)
			BB_Save_Define(HAVE_XF86MISC)
			set(X11_LIBS "${X11_LIBS} -lXxf86misc")
		else()
			BB_Save_Undef(HAVE_XF86MISC)
		endif()

		set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
		set(CMAKE_REQUIRED_LIBRARIES ${ORIG_CMAKE_REQUIRED_LIBRARIES})
	else()
		set(HAVE_X11 OFF)
		set(HAVE_XKB OFF)
		set(HAVE_XF86MISC OFF)
		set(HAVE_XINERAMA OFF)
		BB_Save_Undef(HAVE_X11)
		BB_Save_Undef(HAVE_XKB)
		BB_Save_Undef(HAVE_XF86MISC)
		BB_Save_Undef(HAVE_XINERAMA)
	endif()

	BB_Save_MakeVar(X11_CFLAGS "${X11_CFLAGS}")
	BB_Save_MakeVar(X11_LIBS "${X11_LIBS}")

	BB_Save_MakeVar(XINERAMA_CFLAGS "${XINERAMA_CFLAGS}")
	BB_Save_MakeVar(XINERAMA_LIBS "${XINERAMA_LIBS}")
endmacro()

macro(Disable_X11)
	set(HAVE_X11 OFF)
	set(HAVE_XKB OFF)
	set(HAVE_XF86MISC OFF)
	set(HAVE_XINERAMA OFF)
	BB_Save_Undef(HAVE_X11)
	BB_Save_Undef(HAVE_XKB)
	BB_Save_Undef(HAVE_XF86MISC)
	BB_Save_Undef(HAVE_XINERAMA)
	BB_Save_MakeVar(X11_CFLAGS "")
	BB_Save_MakeVar(X11_LIBS "")
	BB_Save_MakeVar(XINERAMA_CFLAGS "")
	BB_Save_MakeVar(XINERAMA_LIBS "")
endmacro()

#
# From BSDBuild/kqueue.pm:
#
macro(Check_Kqueue)
	check_c_source_compiles("
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
	struct kevent kev, chg;
	int kq, fd = -1, nev;

	if ((kq = kqueue()) == -1) { return (1); }
#if defined(__NetBSD__)
	EV_SET(&kev, (uintptr_t)fd, EVFILT_READ, EV_ADD|EV_ENABLE|EV_ONESHOT, 0, 0, (intptr_t)NULL);
	EV_SET(&kev, (uintptr_t)1, EVFILT_TIMER, EV_ADD|EV_ENABLE, 0, 0, (intptr_t)NULL);
#else
	EV_SET(&kev, fd, EVFILT_READ, EV_ADD|EV_ENABLE|EV_ONESHOT, 0, 0, NULL);
	EV_SET(&kev, 1, EVFILT_TIMER, EV_ADD|EV_ENABLE, 0, 0, NULL);
#endif
	nev = kevent(kq, &kev, 1, &chg, 1, NULL);
	return (chg.flags & EV_ERROR);
}
" HAVE_KQUEUE)
	if (HAVE_KQUEUE)
		BB_Save_Define(HAVE_KQUEUE)
	else()
		BB_Save_Undef(HAVE_KQUEUE)
	endif()
endmacro()

#
# From BSDBuild/execvp.pm:
#
macro(Check_Execvp)
	check_c_source_compiles("
#include <unistd.h>

int
main(int argc, char *argv[])
{
	char *args[3] = { \"foo\", NULL, NULL };
	int rv;

	rv = execvp(args[0], args);
	return (rv);
}
" HAVE_EXECVP)
	if (HAVE_EXECVP)
		BB_Save_Define(HAVE_EXECVP)
	else()
		BB_Save_Undef(HAVE_EXECVP)
	endif()
endmacro()

#
# From BSDBuild/agar.pm:
#
macro(Check_Agar)
	set(AGAR_CFLAGS "")
	set(AGAR_LIBS "")
	set(AGAR_CORE_CFLAGS "")
	set(AGAR_CORE_LIBS "")
	set(AGAR_GUI_CFLAGS "")
	set(AGAR_GUI_LIBS "")
	set(AGAR_AU_CFLAGS "")
	set(AGAR_AU_LIBS "")
	set(AGAR_MAP_CFLAGS "")
	set(AGAR_MAP_LIBS "")
	set(AGAR_MATH_CFLAGS "")
	set(AGAR_MATH_LIBS "")

	find_package(agar)
	if(agar_FOUND)
		set(HAVE_AGAR ON)
		foreach(agarincdir ${AGAR_INCLUDE_DIRS})
			list(APPEND AGAR_CFLAGS "-I${agarincdir}")
			list(APPEND AGAR_CORE_CFLAGS "-I${agarincdir}")
		endforeach()
		foreach(agarlib ${AGAR_CORE_LIBRARIES})
			list(APPEND AGAR_CORE_LIBS "${agarlib}")
		endforeach()
		foreach(agarlib ${AGAR_GUI_LIBRARIES} ${AGAR_CORE_LIBRARIES})
			list(APPEND AGAR_LIBS "${agarlib}")
		endforeach()
		list(REMOVE_DUPLICATES AGAR_CFLAGS)
		list(REMOVE_DUPLICATES AGAR_LIBS)
		list(REMOVE_DUPLICATES AGAR_CORE_CFLAGS)
		list(REMOVE_DUPLICATES AGAR_CORE_LIBS)
		list(REMOVE_DUPLICATES AGAR_INCLUDE_DIRS)
		BB_Save_Define(HAVE_AGAR)
	else()
		set(HAVE_AGAR OFF)
		BB_Save_Undef(HAVE_AGAR)
	endif()

	if(HAVE_AGAR_GUI)
		BB_Save_Define(HAVE_AGAR_GUI)
		set(AGAR_GUI_CFLAGS ${AGAR_CFLAGS})
		foreach(agarlib ${AGAR_GUI_LIBRARIES})
			list(APPEND AGAR_GUI_LIBS "${agarlib}")
		endforeach()
	else()
		BB_Save_Undef(HAVE_AGAR_GUI)
	endif()
	if(HAVE_AGAR_AU)
		BB_Save_Define(HAVE_AGAR_AU)
		set(AGAR_AU_CFLAGS ${AGAR_CFLAGS})
		foreach(agarlib ${AGAR_AU_LIBRARIES})
			list(APPEND AGAR_AU_LIBS "${agarlib}")
		endforeach()
	else()
		BB_Save_Undef(HAVE_AGAR_AU)
	endif()
	if(HAVE_AGAR_MAP)
		BB_Save_Define(HAVE_AGAR_MAP)
		set(AGAR_MAP_CFLAGS ${AGAR_CFLAGS})
		foreach(agarlib ${AGAR_MAP_LIBRARIES})
			list(APPEND AGAR_MAP_LIBS "${agarlib}")
		endforeach()
	else()
		BB_Save_Undef(HAVE_AGAR_MAP)
	endif()
	if(HAVE_AGAR_MATH)
		BB_Save_Define(HAVE_AGAR_MATH)
		set(AGAR_MATH_CFLAGS ${AGAR_CFLAGS})
		foreach(agarlib ${AGAR_MATH_LIBRARIES})
			list(APPEND AGAR_MATH_LIBS "${agarlib}")
		endforeach()
	else()
		BB_Save_Undef(HAVE_AGAR_MATH)
	endif()

	BB_Save_MakeVar(AGAR_CFLAGS "${AGAR_CFLAGS}")
	BB_Save_MakeVar(AGAR_LIBS "${AGAR_LIBS}")
	BB_Save_MakeVar(AGAR_CORE_CFLAGS "${AGAR_CORE_CFLAGS}")
	BB_Save_MakeVar(AGAR_CORE_LIBS "${AGAR_CORE_LIBS}")
	BB_Save_MakeVar(AGAR_GUI_CFLAGS "${AGAR_GUI_CFLAGS}")
	BB_Save_MakeVar(AGAR_GUI_LIBS "${AGAR_GUI_LIBS}")
	BB_Save_MakeVar(AGAR_AU_CFLAGS "${AGAR_AU_CFLAGS}")
	BB_Save_MakeVar(AGAR_AU_LIBS "${AGAR_AU_LIBS}")
	BB_Save_MakeVar(AGAR_MAP_CFLAGS "${AGAR_MAP_CFLAGS}")
	BB_Save_MakeVar(AGAR_MAP_LIBS "${AGAR_MAP_LIBS}")
	BB_Save_MakeVar(AGAR_MATH_CFLAGS "${AGAR_MATH_CFLAGS}")
	BB_Save_MakeVar(AGAR_MATH_LIBS "${AGAR_MATH_LIBS}")
endmacro()

macro(Disable_Agar)
	set(HAVE_AGAR OFF)
	BB_Save_Undef(HAVE_AGAR)
	BB_Save_MakeVar(AGAR_CFLAGS "")
	BB_Save_MakeVar(AGAR_LIBS "")
endmacro()

#
# From BSDBuild/glob.pm:
#
macro(Check_Glob)
	check_c_source_compiles("
#include <string.h>
#include <glob.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
	glob_t gl;
	int rv, i;
	char *s = NULL;

	rv = glob(\"~/foo\", GLOB_TILDE, NULL, &gl);
	for (i = 0; i < gl.gl_pathc; i++) { s = gl.gl_pathv[i]; }
	return (rv != 0 && s != NULL);
}
" HAVE_GLOB)
	if (HAVE_GLOB)
		BB_Save_Define(HAVE_GLOB)
	else()
		BB_Save_Undef(HAVE_GLOB)
	endif()
endmacro()

#
# From BSDBuild/dlopen.pm:
#
macro(Check_Dlopen)
	set(ORIG_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
	set(ORIG_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})

	set(DSO_CFLAGS "")
	set(DSO_LIBS "")

	CHECK_INCLUDE_FILE(dlfcn.h HAVE_DLFCN_H)

	if(HAVE_DLFCN_H)
		set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -DHAVE_DLFCN_H")
		BB_Save_Define(HAVE_DLFCN_H)
	else()
		BB_Save_Undef(HAVE_DLFCN_H)
	endif()

	check_c_source_compiles("
#include <string.h>
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

int
main(int argc, char *argv[])
{
	void *handle;
	char *error;
	handle = dlopen(\"foo.so\", 0);
	error = dlerror();
	(void)dlsym(handle, \"foo\");
	return (error != NULL);
}
" HAVE_DLOPEN)
	if(HAVE_DLOPEN)
		BB_Save_Define(HAVE_DLOPEN)
	else()
		check_library_exists(dl dlopen "" HAVE_LIBDL_DLOPEN)
		if(HAVE_LIBDL_DLOPEN)
			set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES} -ldl")
			check_c_source_compiles("
#include <string.h>
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

int
main(int argc, char *argv[])
{
	void *handle;
	char *error;
	handle = dlopen(\"foo.so\", 0);
	error = dlerror();
	(void)dlsym(handle, \"foo\");
	return (error != NULL);
}
" HAVE_DLOPEN_IN_LIBDL)
			if(HAVE_DLOPEN_IN_LIBDL)
				BB_Save_Define(HAVE_DLOPEN)
				set(DSO_LIBS "-ldl")
			else()
				BB_Save_Undef(HAVE_DLOPEN)
			endif()
		else()
			BB_Save_Undef(HAVE_DLOPEN)
		endif()
	endif()

	BB_Save_MakeVar(DSO_CFLAGS "${DSO_CFLAGS}")
	BB_Save_MakeVar(DSO_LIBS "${DSO_LIBS}")

	set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
	set(CMAKE_REQUIRED_LIBRARIES ${ORIG_CMAKE_REQUIRED_LIBRARIES})
endmacro()

macro(Disable_Dlopen)
	BB_Save_Undef(HAVE_DLOPEN)
	BB_Save_Undef(HAVE_DLFCN_H)
endmacro()

#
# From BSDBuild/sndfile.pm:
#
macro(Check_Sndfile)
	set(SNDFILE_CFLAGS "")
	set(SNDFILE_LIBS "")

	set(ORIG_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
	set(ORIG_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
	set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -I/usr/local/include")
	set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES} -L/usr/local/lib -lsndfile")

	CHECK_INCLUDE_FILE(sndfile.h HAVE_SNDFILE_H)
	if(HAVE_SNDFILE_H)
		check_c_source_compiles("
#include <stdio.h>
#include <sndfile.h>

int main(int argc, char *argv[]) {
	SNDFILE *sf;
	SF_INFO sfi;

	sfi.format = 0;
	sf = sf_open(\"foo\", 0, &sfi);
	sf_close(sf);
	return (0);
}
" HAVE_SNDFILE)
		if(HAVE_SNDFILE)
			set(SNDFILE_CFLAGS "-I/usr/local/include")
			set(SNDFILE_LIBS "-L/usr/local/lib" "-lsndfile")
			BB_Save_Define(HAVE_SNDFILE)
		else()
			BB_Save_Undef(HAVE_SNDFILE)
		endif()
	else()
		set(HAVE_SNDFILE OFF)
		BB_Save_Undef(HAVE_SNDFILE)
	endif()

	set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
	set(CMAKE_REQUIRED_LIBRARIES ${ORIG_CMAKE_REQUIRED_LIBRARIES})

	BB_Save_MakeVar(SNDFILE_CFLAGS "${SNDFILE_CFLAGS}")
	BB_Save_MakeVar(SNDFILE_LIBS "${SNDFILE_LIBS}")
endmacro()

macro(Disable_Sndfile)
	set(HAVE_SNDFILE OFF)
	BB_Save_MakeVar(SNDFILE_CFLAGS "")
	BB_Save_MakeVar(SNDFILE_LIBS "")
	BB_Save_Undef(HAVE_SNDFILE)
endmacro()

#
# From BSDBuild/opengl.pm:
#
macro(Check_OpenGL)
	set(OPENGL_CFLAGS "")
	set(OPENGL_LIBS "")

	include(FindOpenGL)
	if(OPENGL_FOUND)
		set(HAVE_OPENGL ON)

		if(OPENGL_INCLUDE_DIR)
			list(APPEND OPENGL_CFLAGS "-I${OPENGL_INCLUDE_DIR}")
		endif()
		foreach(opengllib ${OPENGL_LIBRARIES})
			list(APPEND OPENGL_LIBS "${opengllib}")
		endforeach()

		BB_Save_Define(HAVE_OPENGL)
	else()
		set(HAVE_OPENGL OFF)
		BB_Save_Undef(HAVE_OPENGL)
	endif()

	if(OpenGL_GLU_FOUND)
		set(HAVE_GLU ON)
		BB_Save_Define(HAVE_GLU)
	else()
		set(HAVE_GLU OFF)
		BB_Save_Undef(HAVE_GLU)
	endif()

	if(OpenGL_GLX_FOUND)
		set(HAVE_GLX ON)
		BB_Save_Define(HAVE_GLX)
	else()
		set(HAVE_GLX OFF)
		BB_Save_Undef(HAVE_GLX)
	endif()

	set(ORIG_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
	set(ORIG_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})

	set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} ${OPENGL_CFLAGS}")
	set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES} ${OPENGL_LIBS}")

	check_c_source_compiles("
#define GL_GLEXT_PROTOTYPES
#ifdef _USE_OPENGL_FRAMEWORK
# include <OpenGL/gl.h>
# include <OpenGL/glext.h>
#else
# include <GL/gl.h>
# include <GL/glext.h>
#endif

static void
DebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar *message, const void *userParam)
{ }

int main(int argc, char *argv[]) {
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(DebugMessageCallback, 0);
	return (0);
}
" HAVE_GLEXT)
	if(HAVE_GLEXT)
		BB_Save_Define(HAVE_GLEXT)
	else()
		BB_Save_Undef(HAVE_GLEXT)
	endif()

	set(CMAKE_REQUIRED_LIBRARIES "${ORIG_CMAKE_REQUIRED_LIBRARIES} ${OPENGL_LIBS} -lgdi32")

	check_c_source_compiles("
#include <windows.h>

int main(int argc, char *argv[]) {
	HWND hwnd;
	HDC hdc;
	HGLRC hglrc;

	hwnd = CreateWindowEx(0, \"a\", \"a\", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
	    CW_USEDEFAULT, 0,0, NULL, NULL, GetModuleHandle(NULL), NULL);
	hdc = GetDC(hwnd);
	hglrc = wglCreateContext(hdc);
	SwapBuffers(hdc);
	wglDeleteContext(hglrc);
	ReleaseDC(hwnd, hdc);
	DestroyWindow(hwnd);
	return (0);
}
" HAVE_WGL)
	if(HAVE_WGL)
		BB_Save_Define(HAVE_WGL)
		set(OPENGL_LIBS "${OPENGL_LIBS} -lgdi32")
	else()
		BB_Save_Undef(HAVE_WGL)
	endif()

	set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
	set(CMAKE_REQUIRED_LIBRARIES ${ORIG_CMAKE_REQUIRED_LIBRARIES})

	BB_Save_MakeVar(OPENGL_CFLAGS "${OPENGL_CFLAGS}")
	BB_Save_MakeVar(OPENGL_LIBS "${OPENGL_LIBS}")
endmacro()

macro(Disable_OpenGL)
	set(HAVE_OPENGL OFF)
	set(HAVE_GLU OFF)
	set(HAVE_GLX OFF)
	set(HAVE_GLEXT OFF)
	set(HAVE_WGL OFF)
	BB_Save_Undef(HAVE_OPENGL)
	BB_Save_Undef(HAVE_GLU)
	BB_Save_Undef(HAVE_GLX)
	BB_Save_Undef(HAVE_GLEXT)
	BB_Save_Undef(HAVE_WGL)
	BB_Save_MakeVar(OPENGL_CFLAGS "")
	BB_Save_MakeVar(OPENGL_LIBS "")
endmacro()

#
# From BSDBuild/strtold.pm:
#
macro(Check_Strtold)
	check_c_source_compiles("
#ifndef __NetBSD__
# define _XOPEN_SOURCE 600
#endif
#include <stdlib.h>
int
main(int argc, char *argv[])
{
	long double ld;
	char *ep = NULL;
	char *foo = \"1234\";

	ld = strtold(foo, &ep);
	return (ld != 1234.0);
}
" _MK_HAVE_STRTOLD)
	if (_MK_HAVE_STRTOLD)
		BB_Save_Define(_MK_HAVE_STRTOLD)
	else()
		BB_Save_Undef(_MK_HAVE_STRTOLD)
	endif()
endmacro()

#
# From BSDBuild/sys_stat.pm:
#
macro(Check_Sys_Stat_h)
	check_c_source_compiles("
#include <sys/types.h>
#include <sys/stat.h>
int main(int argc, char *argv[]) {
	struct stat sb;
	uid_t uid;
	if (stat(\"/tmp/foo\", &sb) != 0) { return (1); }
	return ((uid = sb.st_uid) == (uid_t)0);
}
" _MK_HAVE_SYS_STAT_H)
	if (_MK_HAVE_SYS_STAT_H)
		BB_Save_Define(_MK_HAVE_SYS_STAT_H)
	else()
		BB_Save_Undef(_MK_HAVE_SYS_STAT_H)
	endif()
endmacro()

#
# From BSDBuild/sdl2.pm:
#
macro(Check_Sdl2)
	set(SDL2_CFLAGS "")
	set(SDL2_LIBS "")

	find_package(SDL2)
	if(SDL2_FOUND)
		set(HAVE_SDL2 ON)

		foreach(sdl2incdir ${SDL2_INCLUDE_DIRS})
			list(APPEND SDL2_CFLAGS "-I${sdl2incdir}")
		endforeach()

		find_library(SDL2_LIBRARY NAMES SDL2)
		if(SDL2_LIBRARY)
			list(APPEND SDL2_LIBS "${SDL2_LIBRARY}")
		endif()

		message(STATUS "Found SDL2: ${SDL2_LIBS} (found version \"${SDL2_VERSION}\")")
		BB_Save_Define(HAVE_SDL2)
	else()
		set(HAVE_SDL2 OFF)
		BB_Save_Undef(HAVE_SDL2)
	endif()

	BB_Save_MakeVar(SDL2_CFLAGS "${SDL2_CFLAGS}")
	BB_Save_MakeVar(SDL2_LIBS "${SDL2_LIBS}")
endmacro()

macro(Disable_Sdl2)
	set(HAVE_SDL2 OFF)
	BB_Save_Undef(HAVE_SDL2)
	BB_Save_MakeVar(SDL2_CFLAGS "")
	BB_Save_MakeVar(SDL2_LIBS "")
endmacro()

#
# From BSDBuild/cc_attributes.pm:
#
macro(Check_Cc_Attributes)
	set(ORIG_CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS}")
	set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -Wall -Werror")

	check_c_source_compiles("
int main(int argc, char *argv[])
{
	struct s1 { int x,y,z; } __attribute__ ((aligned(16)));
	return (0);
}
" HAVE_ALIGNED_ATTRIBUTE)
	if (HAVE_ALIGNED_ATTRIBUTE)
		BB_Save_Define(HAVE_ALIGNED_ATTRIBUTE)
	else()
		BB_Save_Undef(HAVE_ALIGNED_ATTRIBUTE)
	endif()

	check_c_source_compiles("
void foostring(char *, int) __attribute__ ((__bounded__(__string__,1,2)));
void foostring(char *a, int c) { }
void foobuffer(void *, int) __attribute__ ((__bounded__(__buffer__,1,2)));
void foobuffer(void *a, int c) { }
int main(void)
{
	char buf[32];
	foostring(buf, sizeof(buf));
	foobuffer(buf, sizeof(buf));
	return (0);
}
" HAVE_BOUNDED_ATTRIBUTE)
	if (HAVE_BOUNDED_ATTRIBUTE)
		BB_Save_Define(HAVE_BOUNDED_ATTRIBUTE)
	else()
		BB_Save_Undef(HAVE_BOUNDED_ATTRIBUTE)
	endif()

	check_c_source_compiles("
int foo(int) __attribute__ ((const));
int foo(int x) { return (x*x); }
int main(int argc, char *argv[])
{
	int x = foo(1);
	return (x);
}
" HAVE_CONST_ATTRIBUTE)
	if (HAVE_CONST_ATTRIBUTE)
		BB_Save_Define(HAVE_CONST_ATTRIBUTE)
	else()
		BB_Save_Undef(HAVE_CONST_ATTRIBUTE)
	endif()

	check_c_source_compiles("
void foo(void) __attribute__ ((deprecated));
void foo(void) { }

int main(int argc, char *argv[])
{
/*	foo(); */
	return (0);
}
" HAVE_DEPRECATED_ATTRIBUTE)
	if (HAVE_DEPRECATED_ATTRIBUTE)
		BB_Save_Define(HAVE_DEPRECATED_ATTRIBUTE)
	else()
		BB_Save_Undef(HAVE_DEPRECATED_ATTRIBUTE)
	endif()

	check_c_source_compiles("
#include <stdarg.h>
void foo1(char *, ...)
     __attribute__((__format__ (printf, 1, 2)));
void foo2(char *, ...)
     __attribute__((__format__ (__printf__, 1, 2)));
void foo1(char *a, ...) {}
void foo2(char *a, ...) {}
int main(int argc, char *argv[])
{
	foo1(\"foo %s\", \"bar\");
	foo2(\"foo %d\", 1);
	return (0);
}
" HAVE_FORMAT_ATTRIBUTE)
	if (HAVE_FORMAT_ATTRIBUTE)
		BB_Save_Define(HAVE_FORMAT_ATTRIBUTE)
	else()
		BB_Save_Undef(HAVE_FORMAT_ATTRIBUTE)
	endif()

	check_c_source_compiles("
#include <stdio.h>
#include <stdlib.h>

void *myalloc(size_t len) __attribute__ ((__malloc__));
void *myalloc(size_t len) { return (NULL); }
int main(int argc, char *argv[])
{
	void *p = myalloc(10);
	return (p != NULL);
}
" HAVE_MALLOC_ATTRIBUTE)
	if (HAVE_MALLOC_ATTRIBUTE)
		BB_Save_Define(HAVE_MALLOC_ATTRIBUTE)
	else()
		BB_Save_Undef(HAVE_MALLOC_ATTRIBUTE)
	endif()

	check_c_source_compiles("
#include <unistd.h>
#include <stdlib.h>
void foo(void) __attribute__ ((noreturn));
void foo(void) { _exit(0); }
int main(int argc, char *argv[])
{
	foo();
}
" HAVE_NORETURN_ATTRIBUTE)
	if (HAVE_NORETURN_ATTRIBUTE)
		BB_Save_Define(HAVE_NORETURN_ATTRIBUTE)
	else()
		BB_Save_Undef(HAVE_NORETURN_ATTRIBUTE)
	endif()

	check_c_source_compiles("
int main(int argc, char *argv[])
{
	struct s1 { char c; int x,y,z; } __attribute__ ((packed));
	return (0);
}
" HAVE_PACKED_ATTRIBUTE)
	if (HAVE_PACKED_ATTRIBUTE)
		BB_Save_Define(HAVE_PACKED_ATTRIBUTE)
	else()
		BB_Save_Undef(HAVE_PACKED_ATTRIBUTE)
	endif()

	check_c_source_compiles("
volatile int glo = 1234;
int foo(int) __attribute__ ((pure));
int foo(int x) { return (x*x + glo); }
int main(int argc, char *argv[])
{
	int x = foo(1);
	glo = 2345;
	x = foo(2);
	return (x);
}
" HAVE_PURE_ATTRIBUTE)
	if (HAVE_PURE_ATTRIBUTE)
		BB_Save_Define(HAVE_PURE_ATTRIBUTE)
	else()
		BB_Save_Undef(HAVE_PURE_ATTRIBUTE)
	endif()

	check_c_source_compiles("
int main(int argc, char *argv[])
{
	int __attribute__ ((unused)) variable;
	return (0);
}
" HAVE_UNUSED_VARIABLE_ATTRIBUTE)
	if (HAVE_UNUSED_VARIABLE_ATTRIBUTE)
		BB_Save_Define(HAVE_UNUSED_VARIABLE_ATTRIBUTE)
	else()
		BB_Save_Undef(HAVE_UNUSED_VARIABLE_ATTRIBUTE)
	endif()

	check_c_source_compiles("
int foo(void) __attribute__ ((warn_unused_result));
int foo(void) { return (1); }
int main(int argc, char *argv[])
{
	int rv = foo();
	return (rv);
}
" HAVE_WARN_UNUSED_RESULT_ATTRIBUTE)
	if (HAVE_WARN_UNUSED_RESULT_ATTRIBUTE)
		BB_Save_Define(HAVE_WARN_UNUSED_RESULT_ATTRIBUTE)
	else()
		BB_Save_Undef(HAVE_WARN_UNUSED_RESULT_ATTRIBUTE)
	endif()

	set(CMAKE_REQUIRED_FLAGS "${ORIG_CMAKE_REQUIRED_FLAGS}")
endmacro()

macro(Disable_Cc_Attributes)
	BB_Save_Undef(HAVE_ALIGNED_ATTRIBUTE)
	BB_Save_Undef(HAVE_BOUNDED_ATTRIBUTE)
	BB_Save_Undef(HAVE_CONST_ATTRIBUTE)
	BB_Save_Undef(HAVE_DEPRECATED_ATTRIBUTE)
	BB_Save_Undef(HAVE_FORMAT_ATTRIBUTE)
	BB_Save_Undef(HAVE_MALLOC_ATTRIBUTE)
	BB_Save_Undef(HAVE_NORETURN_ATTRIBUTE)
	BB_Save_Undef(HAVE_PACKED_ATTRIBUTE)
	BB_Save_Undef(HAVE_PURE_ATTRIBUTE)
	BB_Save_Undef(HAVE_UNUSED_VARIABLE_ATTRIBUTE)
	BB_Save_Undef(HAVE_WARN_UNUSED_RESULT_ATTRIBUTE)
endmacro()

#
# From BSDBuild/math_c99.pm:
#
macro(Check_Math_C99)
	set(ORIG_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})

	set(MATH_C99_CFLAGS "")
	set(MATH_C99_LIBS "")

	if (MINGW)
		message("Disabling C99 math due to libmingwex linker errors")
		BB_Save_Undef(HAVE_MATH_C99)
	else()
		check_library_exists(m pow "" HAVE_LIBM_POW)
		if (HAVE_LIBM_POW)
			set(CMAKE_REQUIRED_LIBRARIES m)
			set(MATH_C99_LIBS "-lm")
		endif()

		check_c_source_compiles("
#include <math.h>

int
main(int argc, char *argv[])
{
	float f = 1.0;
	double d = 1.0;

	d = fabs(d);
	f = sqrtf(fabsf(f));
	return (f > d) ? 0 : 1;
}
" HAVE_MATH_C99)
		if (HAVE_MATH_C99)
			BB_Save_Define(HAVE_MATH_C99)
		else()
			BB_Save_Undef(HAVE_MATH_C99)
		endif()
	endif()

	BB_Save_MakeVar(MATH_C99_CFLAGS "${MATH_C99_CFLAGS}")
	BB_Save_MakeVar(MATH_C99_LIBS "${MATH_C99_LIBS}")

	set(CMAKE_REQUIRED_LIBRARIES ${ORIG_CMAKE_REQUIRED_LIBRARIES})
endmacro()

macro(Disable_Math_C99)
	BB_Save_Undef(HAVE_MATH_C99)
	BB_Save_MakeVar(MATH_C99_CFLAGS "")
	BB_Save_MakeVar(MATH_C99_LIBS "")
endmacro()

#
# From BSDBuild/db4.pm:
#
macro(Check_Db4)
	set(DB4_CFLAGS "")
	set(DB4_LIBS "")
	if(FREEBSD)
		set(ORIG_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
		set(ORIG_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
		set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -I/usr/local/include")
		set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES} -L/usr/local/lib -ldb-18")

		CHECK_INCLUDE_FILE(db18/db.h HAVE_DB18_DB_H)
		if(HAVE_DB18_DB_H)
			check_c_source_compiles("
#ifdef __FreeBSD__
# include <db18/db.h>
#else
# include <db.h>
#endif

int main(int argc, char *argv[]) {
	DB *db;
	db_create(&db, NULL, 0);
	return (0);
}
" HAVE_DB4)
			if(HAVE_DB4)
				set(DB4_CFLAGS "-I/usr/local/include")
				set(DB4_LIBS "-L/usr/local/lib" "-ldb-18")
				BB_Save_Define(HAVE_DB4)
			else()
				BB_Save_Undef(HAVE_DB4)
			endif()
		else()
			set(HAVE_DB4 OFF)
			BB_Save_Undef(HAVE_DB4)
		endif()

		set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
		set(CMAKE_REQUIRED_LIBRARIES ${ORIG_CMAKE_REQUIRED_LIBRARIES})
	else()
		check_c_source_compiles("
#ifdef __FreeBSD__
# include <db18/db.h>
#else
# include <db.h>
#endif

int main(int argc, char *argv[]) {
	DB *db;
	db_create(&db, NULL, 0);
	return (0);
}
" HAVE_DB4)
		if(HAVE_DB4)
			BB_Save_Define(HAVE_DB4)
		else()
			BB_Save_Undef(HAVE_DB4)
		endif()
	endif()

	BB_Save_MakeVar(DB4_CFLAGS "${DB4_CFLAGS}")
	BB_Save_MakeVar(DB4_LIBS "${DB4_LIBS}")
endmacro()

macro(Disable_Db4)
	set(HAVE_DB4 OFF)
	BB_Save_MakeVar(DB4_CFLAGS "")
	BB_Save_MakeVar(DB4_LIBS "")
	BB_Save_Undef(HAVE_DB4)
endmacro()

#
# From BSDBuild/iconv.pm:
#
macro(Check_Iconv)
	set(ICONV_CFLAGS "")
	set(ICONV_LIBS "")

	include(FindIconv)
	if(Iconv_FOUND)
		set(HAVE_ICONV ON)
		BB_Save_Define(HAVE_ICONV)
		if(${Iconv_INCLUDE_DIRS})
			set(ICONV_CFLAGS "-I${Iconv_INCLUDE_DIRS}")
		endif()
		set(ICONV_LIBS "${Iconv_LIBRARIES}")
	else()
		set(HAVE_ICONV OFF)
		BB_Save_Undef(HAVE_ICONV)
	endif()

	BB_Save_MakeVar(ICONV_CFLAGS "${ICONV_CFLAGS}")
	BB_Save_MakeVar(ICONV_LIBS "${ICONV_LIBS}")
endmacro()

macro(Disable_Iconv)
	set(HAVE_ICONV OFF)
	BB_Save_Undef(HAVE_ICONV)
	BB_Save_MakeVar(ICONV_CFLAGS "")
	BB_Save_MakeVar(ICONV_LIBS "")
endmacro()

#
# From BSDBuild/getaddrinfo.pm:
#
macro(Check_Getaddrinfo)
	check_c_source_compiles("
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int
main(int argc, char *argv[])
{
	struct addrinfo hints, *res0;
	const char *s;
	int rv;

	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	rv = getaddrinfo(\"hostname\", \"port\", &hints, &res0);
	s = gai_strerror(rv);
	freeaddrinfo(res0);
	return (s != NULL);
}
" HAVE_GETADDRINFO)
	if (HAVE_GETADDRINFO)
		BB_Save_Define(HAVE_GETADDRINFO)
	else()
		BB_Save_Undef(HAVE_GETADDRINFO)
	endif()
endmacro()

#
# From BSDBuild/altivec.pm:
#
macro(Check_Altivec)
	set(ORIG_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})

	set(CMAKE_REQUIRED_FLAGS "${ORIG_CMAKE_REQUIRED_FLAGS} -maltivec")
	check_c_source_compiles("
float a[4] = { 1,2,3,4 };
float b[4] = { 5,6,7,8 };
float c[4];

int
main(int argc, char *argv[])
{
	vector float *va = (vector float *)a;
	vector float *vb = (vector float *)b;
	vector float *vc = (vector float *)c;

	*vc = vec_add(*va, *vb);
	return (0);
}
" HAVE_ALTIVEC)
	if (HAVE_ALTIVEC)
		set(ALTIVEC_CFLAGS "-maltivec")
		BB_Save_MakeVar(ALTIVEC_CFLAGS "${ALTIVEC_CFLAGS}")
		BB_Save_Define(HAVE_ALTIVEC)
	else()
		set(ALTIVEC_CFLAGS "")
		BB_Save_MakeVar(ALTIVEC_CFLAGS "")
		BB_Save_Undef(HAVE_ALTIVEC)
	endif()

	set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
endmacro()

macro(Disable_Altivec)
	set(ALTIVEC_CFLAGS "")
	BB_Save_MakeVar(ALTIVEC_CFLAGS "")
	BB_Save_Undef(HAVE_ALTIVEC)
endmacro()

#
# From BSDBuild/mysql.pm:
#
macro(Check_Mysql)
	set(MYSQL_CFLAGS "")
	set(MYSQL_LIBS "")

	set(ORIG_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
	set(ORIG_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
	set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -I/usr/local/include/mysql")
	set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES} -L/usr/local/lib/mysql -lmysqlclient_r")

	CHECK_INCLUDE_FILE(mysql.h HAVE_MYSQL_H)
	if(HAVE_MYSQL_H)
		check_c_source_compiles("
#include <mysql.h>
#include <string.h>

int
main(int argc, char *argv[])
{
	MYSQL *my = mysql_init(NULL);
	if (my != NULL) { mysql_close(my); }
	return (0);
}
" HAVE_MYSQL)
		if(HAVE_MYSQL)
			set(MYSQL_CFLAGS "-I/usr/local/include")
			set(MYSQL_LIBS "-L/usr/local/lib/mysql" "-lmysqlclient_r")
			BB_Save_Define(HAVE_MYSQL)
		else()
			BB_Save_Undef(HAVE_MYSQL)
		endif()
	else()
		set(HAVE_MYSQL OFF)
		BB_Save_Undef(HAVE_MYSQL)
	endif()

	set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
	set(CMAKE_REQUIRED_LIBRARIES ${ORIG_CMAKE_REQUIRED_LIBRARIES})

	BB_Save_MakeVar(MYSQL_CFLAGS "${MYSQL_CFLAGS}")
	BB_Save_MakeVar(MYSQL_LIBS "${MYSQL_LIBS}")
endmacro()

macro(Disable_Mysql)
	set(HAVE_MYSQL OFF)
	BB_Save_MakeVar(MYSQL_CFLAGS "")
	BB_Save_MakeVar(MYSQL_LIBS "")
	BB_Save_Undef(HAVE_MYSQL)
endmacro()

#
# From BSDBuild/freetype.pm:
#
macro(Check_FreeType)
	set(FREETYPE_CFLAGS "")
	set(FREETYPE_LIBS "")

	include(FindFreetype)
	if(FREETYPE_FOUND)
		set(HAVE_FREETYPE ON)
		foreach(freetypeincdir ${FREETYPE_INCLUDE_DIRS})
			list(APPEND FREETYPE_CFLAGS "-I${freetypeincdir}")
		endforeach()
		foreach(freetypelib ${FREETYPE_LIBRARIES})
			list(APPEND FREETYPE_LIBS "${freetypelib}")
		endforeach()
		BB_Save_Define(HAVE_FREETYPE)
	else()
		set(HAVE_FREETYPE OFF)
		BB_Save_Undef(HAVE_FREETYPE)
	endif()

	BB_Save_MakeVar(FREETYPE_CFLAGS "${FREETYPE_CFLAGS}")
	BB_Save_MakeVar(FREETYPE_LIBS "${FREETYPE_LIBS}")
endmacro()

macro(Disable_FreeType)
	set(HAVE_FREETYPE OFF)
	BB_Save_Undef(HAVE_FREETYPE)
	BB_Save_MakeVar(FREETYPE_CFLAGS "")
	BB_Save_MakeVar(FREETYPE_LIBS "")
endmacro()

#
# From BSDBuild/gettext.pm:
#
macro(Check_Gettext)
	set(GETTEXT_CFLAGS "")
	set(GETTEXT_LIBS "")

	include(FindIntl)
	if(Intl_FOUND)
		set(HAVE_GETTEXT ON)
		BB_Save_Define(HAVE_GETTEXT)
		if(Intl_INCLUDE_DIRS)
			list(APPEND GETTEXT_CFLAGS "-I${Intl_INCLUDE_DIRS}")
		endif()
		set(GETTEXT_LIBS "${Intl_LIBRARIES}")
	else()
		set(HAVE_GETTEXT OFF)
		BB_Save_Undef(HAVE_GETTEXT)
	endif()
	
	BB_Save_MakeVar(GETTEXT_CFLAGS "${GETTEXT_CFLAGS}")
	BB_Save_MakeVar(GETTEXT_LIBS "${GETTEXT_LIBS}")
endmacro()

macro(Disable_Gettext)
	set(HAVE_GETTEXT OFF)
	BB_Save_Undef(HAVE_GETTEXT)
	BB_Save_MakeVar(GETTEXT_CFLAGS "")
	BB_Save_MakeVar(GETTEXT_LIBS "")
endmacro()

#
# From BSDBuild/db5.pm:
#
macro(Check_Db5)
	# TODO
endmacro()

macro(Disable_Db5)
	BB_Save_MakeVar(DB5_CFLAGS "")
	BB_Save_MakeVar(DB5_LIBS "")
	BB_Save_Undef(HAVE_DB5)
endmacro()

#
# From BSDBuild/cc.pm:
#
macro(Check_Cc)

	if(CMAKE_C_COMPILER_ID MATCHES "Clang")
		set(USE_CLANG TRUE)
		set(HAVE_CC_ASM TRUE)
		if(MSVC)
			set(MSVC_CLANG TRUE)
		endif()
	elseif(CMAKE_COMPILER_IS_GNUCC)
		set(USE_GCC TRUE)
		set(HAVE_CC_ASM TRUE)
	elseif(MSVC_VERSION GREATER 1400) # VisualStudio 8.0+
		set(HAVE_CC_ASM TRUE)
	else()
		set(HAVE_CC_ASM FALSE)
	endif()
	if(EMSCRIPTEN)
		set(HAVE_CC_ASM FALSE)
	endif()

	if (HAVE_CC_ASM)
		BB_Save_Define(HAVE_CC_ASM)
	else()
		BB_Save_Undef(HAVE_CC_ASM)
	endif()

	if(HAIKU)
		set(LINKER_LANGUAGE CXX)
	endif()

	check_c_source_compiles("
int main(int argc, char *argv[]) { return (0); }
" HAVE_CC)
	if (HAVE_CC)
		BB_Save_Define(HAVE_CC)
	else()
		BB_Save_Undef(HAVE_CC)
	endif()

	check_c_source_compiles("
#if !defined(__clang__)
# error \"is not clang\"
#endif
int main(int argc, char *argv[]) { return (0); }
" HAVE_CC_CLANG)
	if (HAVE_CC_CLANG)
		if(NOT USE_CLANG)
			message(WARNING "Compiler appears to be clang but Cmake disagrees")
		endif()
		BB_Save_Define(HAVE_CC_CLANG)
	else()
		if(USE_CLANG)
			message(WARNING "Compiler appears to be NOT clang but Cmake disagrees")
		endif()
		BB_Save_Undef(HAVE_CC_CLANG)
	endif()

	check_c_source_compiles("
#if !defined(__GNUC__) || defined(__clang__)
# error \"is not gcc\"
#endif
int main(int argc, char *argv[]) { return (0); }
" HAVE_CC_GCC)
	if (HAVE_CC_GCC)
		if(NOT USE_GCC)
			message(WARNING "Compiler appears to be gcc but Cmake disagrees")
		endif()
		BB_Save_Define(HAVE_CC_GCC)
	else()
		if(USE_GCC)
			message(WARNING "Compiler appears to be NOT gcc but Cmake disagrees")
		endif()
		BB_Save_Undef(HAVE_CC_GCC)
	endif()

	#
	# XXX TODO
	#
	BB_Save_Undef(HAVE_CC65)
	BB_Save_Undef(HAVE_EMCC)
	BB_Save_MakeVar(PICFLAGS "")
	BB_Save_MakeVar(EXECSUFFIX "")

	set(ORIG_CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS}")
	set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -Wall")
	check_c_source_compiles("
int main(int argc, char *argv[]) { return (0); }
" HAVE_CC_WARNINGS)
	if (HAVE_CC_WARNINGS)
		BB_Save_Define(HAVE_CC_WARNINGS)
	else()
		BB_Save_Undef(HAVE_CC_WARNINGS)
	endif()
	set(CMAKE_REQUIRED_FLAGS "${ORIG_CMAKE_REQUIRED_FLAGS}")

	check_c_source_compiles("
#include <stdio.h>
int
main(int argc, char *argv[])
{
	float f = 0.1f;
	double d = 0.2;

	printf(\"%f\", f);
	return ((double)f + d) > 0.2 ? 1 : 0;
}
" HAVE_FLOAT)
	if (HAVE_FLOAT)
		BB_Save_Define(HAVE_FLOAT)
	else()
		BB_Save_Undef(HAVE_FLOAT)
	endif()

	check_c_source_compiles("
#include <stdio.h>
int
main(int argc, char *argv[])
{
	long double ld = 0.1;

	printf(\"%Lf\", ld);
	return (ld + 0.1) > 0.2 ? 1 : 0;
}
" HAVE_LONG_DOUBLE)
	if (HAVE_LONG_DOUBLE)
		BB_Save_Define(HAVE_LONG_DOUBLE)
	else()
		BB_Save_Undef(HAVE_LONG_DOUBLE)
	endif()

	check_c_source_compiles("
int
main(int argc, char *argv[])
{
	long long ll = -1;
	unsigned long long ull = 1;

	return (ll != -1 || ull != 1);
}
" HAVE_LONG_LONG)
	if (HAVE_LONG_LONG)
		BB_Save_Define(HAVE_LONG_LONG)
	else()
		BB_Save_Undef(HAVE_LONG_LONG)
	endif()

	check_c_source_compiles("
#include <sys/types.h>
#include <sys/stat.h>
#include <windows.h>

int main(int argc, char *argv[]) {
	struct stat sb;
	DWORD rv;
	rv = GetFileAttributes(\"foo\");
	stat(\"foo\", &sb);
	return(0);
}
" HAVE_CYGWIN)
	if (HAVE_CYGWIN)
		BB_Save_Define(HAVE_CYGWIN)
	else()
		BB_Save_Undef(HAVE_CYGWIN)
	endif()

	BB_Save_MakeVar(PROG_GUI_FLAGS "")
	BB_Save_MakeVar(PROG_CLI_FLAGS "")
	BB_Save_MakeVar(LIBTOOLOPTS_SHARED "")
endmacro()

#
# From BSDBuild/math.pm:
#
macro(Check_Math)
	set(ORIG_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})

	set(MATH_CFLAGS "")
	set(MATH_LIBS "")

	check_library_exists(m pow "" HAVE_LIBM_POW)
	if (HAVE_LIBM_POW)
		set(CMAKE_REQUIRED_LIBRARIES "m")
		set(MATH_LIBS "-lm")
	endif()

	check_c_source_compiles("
#include <math.h>

int
main(int argc, char *argv[])
{
	double d = 1.0;
	d = fabs(d);
	return (0);
}
" HAVE_MATH)
	if (HAVE_MATH)
		BB_Save_Define(HAVE_MATH)
	else()
		BB_Save_Undef(HAVE_MATH)
	endif()

	BB_Save_MakeVar(MATH_CFLAGS "${MATH_CFLAGS}")
	BB_Save_MakeVar(MATH_LIBS "${MATH_LIBS}")

	set(CMAKE_REQUIRED_LIBRARIES ${ORIG_CMAKE_REQUIRED_LIBRARIES})
endmacro()

macro(Disable_Math)
	BB_Save_Undef(HAVE_MATH)
endmacro()

#
# From BSDBuild/sys_types.pm:
#
macro(Check_Sys_Types_h)

	check_c_source_compiles("
#include <sys/types.h>
int main(int argc, char *argv[]) {
	int8_t s8 = 2;
	u_int8_t u8 = 2;
	int32_t s32 = 1234;
	u_int32_t u32 = 5678;
	return (s8+u8 == 4 && s32+u32 > 6000 ? 0 : 1);
}
" _MK_HAVE_SYS_TYPES_H)
	if (_MK_HAVE_SYS_TYPES_H)
		BB_Save_Define(_MK_HAVE_SYS_TYPES_H)
	else()
		BB_Save_Undef(_MK_HAVE_SYS_TYPES_H)
	endif()

	check_c_source_compiles("
#include <stdint.h>
int main(int argc, char *argv[]) {
	int8_t s8 = 2;
	uint8_t u8 = 2;
	int32_t s32 = 1234;
	uint32_t u32 = 5678;
	return (s8+u8 == 4 && s32+u32 > 6000 ? 0 : 1);
}
" _MK_HAVE_STDINT_H)
	if (_MK_HAVE_STDINT_H)
		BB_Save_Define(_MK_HAVE_STDINT_H)
	else()
		BB_Save_Undef(_MK_HAVE_STDINT_H)
	endif()

	if (_MK_HAVE_SYS_TYPES_H)
		check_c_source_compiles("
#include <sys/types.h>
#include <stdio.h>
int main(int argc, char *argv[]) {
	int64_t i64 = 0;
	u_int64_t u64 = 0;
	printf(\"%lld %llu\", (long long)i64, (unsigned long long)u64);
	return (i64 != 0 || u64 != 0);
}
" HAVE_INT64_T)
		if (HAVE_INT64_T)
			BB_Save_Define(HAVE_INT64_T)
		else()
			BB_Save_Undef(HAVE_INT64_T)
		endif()

		check_c_source_compiles("
#include <sys/types.h>
#include <stdio.h>
int main(int argc, char *argv[]) {
	__int64 i64 = 0;
	printf(\"%lld\", (long long)i64);
	return (i64 != 0);
}
" HAVE___INT64)
		if (HAVE___INT64)
			BB_Save_Define(HAVE___INT64)
		else()
			BB_Save_Undef(HAVE___INT64)
		endif()
	endif()

	if ((HAVE_INT64_T) OR (HAVE___INT64))
		BB_Save_Define(HAVE_64BIT)
	else()
		BB_Save_Undef(HAVE_64BIT)
	endif()

endmacro()

macro(Disable_Sys_Types_h)
	BB_Save_Undef(_MK_HAVE_SYS_TYPES_H)
	BB_Save_Undef(_MK_HAVE_STDINT_H)
	BB_Save_Undef(HAVE_64BIT)
	BB_Save_Undef(HAVE_INT64_T)
	BB_Save_Undef(HAVE___INT64)
endmacro()

#
# From BSDBuild/clock_gettime.pm:
#
macro(Check_Clock_gettime)
	set(CLOCK_CFLAGS "")
	set(CLOCK_LIBS "")

	set(ORIG_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})

	check_c_source_compiles("
#include <time.h>
int
main(int argc, char *argv[])
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
#ifdef __FreeBSD__
	clock_gettime(CLOCK_SECOND, &ts);
#endif
	return (0);
}
" HAVE_CLOCK_GETTIME)
	if(HAVE_CLOCK_GETTIME)
		BB_Save_Define(HAVE_CLOCK_GETTIME)
	else()
		check_library_exists(rt clock_gettime "" HAVE_LIBRT_CLOCK_GETTIME)
		if(HAVE_LIBRT_CLOCK_GETTIME)
			set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES} -lrt")
			check_c_source_compiles("
#include <time.h>
int
main(int argc, char *argv[])
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
#ifdef __FreeBSD__
	clock_gettime(CLOCK_SECOND, &ts);
#endif
	return (0);
}
" HAVE_CLOCK_GETTIME_IN_LIBRT)
			if(HAVE_CLOCK_GETTIME_IN_LIBRT)
				BB_Save_Define(HAVE_CLOCK_GETTIME)
				set(CLOCK_LIBS "-lrt")
			else()
				BB_Save_Undef(HAVE_CLOCK_GETTIME)
			endif()
		else()
			BB_Save_Undef(HAVE_CLOCK_GETTIME)
		endif()
	endif()

	BB_Save_MakeVar(CLOCK_CFLAGS "${CLOCK_CFLAGS}")
	BB_Save_MakeVar(CLOCK_LIBS "${CLOCK_LIBS}")

	set(CMAKE_REQUIRED_LIBRARIES ${ORIG_CMAKE_REQUIRED_LIBRARIES})
endmacro()

macro(Disable_Clock_gettime)
	BB_Save_Undef(HAVE_CLOCK_GETTIME)
endmacro()

#
# From BSDBuild/fdclose.pm:
#
macro(Check_Fdclose)
	check_c_source_compiles("
#include <stdio.h>
int
main(int argc, char *argv[])
{
	FILE *f = fopen(\"/dev/null\",\"r\");
	int fdp;

	return fdclose(f, &fdp);
}
" HAVE_FDCLOSE)
	if (HAVE_FDCLOSE)
		BB_Save_Define(HAVE_FDCLOSE)
	else()
		BB_Save_Undef(HAVE_FDCLOSE)
	endif()
endmacro()

#
# From BSDBuild/png.pm:
#
macro(Check_Png)
	set(PNG_CFLAGS "")
	set(PNG_LIBS "")

	include(FindPNG)
	if(PNG_FOUND)
		set(HAVE_PNG ON)

		foreach(pngdef ${PNG_DEFINITIONS})
			list(APPEND PNG_CFLAGS "-D${pngdef}")
		endforeach()
		foreach(pngincdir ${PNG_INCLUDE_DIRS})
			list(APPEND PNG_CFLAGS "-I${pngincdir}")
		endforeach()
		foreach(pnglib ${PNG_LIBRARIES})
			list(APPEND PNG_LIBS "${pnglib}")
		endforeach()

		BB_Save_Define(HAVE_PNG)

		if(${PNG_VERSION_STRING} VERSION_GREATER_EQUAL "1.4.0")
			set(HAVE_LIBPNG14 ON)
			BB_Save_Define(HAVE_LIBPNG14)
		else()
			set(HAVE_LIBPNG14 OFF)
			BB_Save_Undef(HAVE_LIBPNG14)
		endif()
	else()
		set(HAVE_PNG OFF)
		set(HAVE_LIBPNG14 OFF)
		BB_Save_Undef(HAVE_PNG)
		BB_Save_Undef(HAVE_LIBPNG14)
	endif()

	BB_Save_MakeVar(PNG_CFLAGS "${PNG_CFLAGS}")
	BB_Save_MakeVar(PNG_LIBS "${PNG_LIBS}")
endmacro()

macro(Disable_Png)
	set(HAVE_PNG OFF)
	set(HAVE_LIBPNG14 OFF)
	BB_Save_Undef(HAVE_PNG)
	BB_Save_Undef(HAVE_LIBPNG14)
	BB_Save_MakeVar(PNG_CFLAGS "")
	BB_Save_MakeVar(PNG_LIBS "")
endmacro()

#
# From BSDBuild/select.pm:
#
macro(Check_Select)
	check_c_source_compiles("
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int
main(int argc, char *argv[])
{
	struct timeval tv;
	int rv;

	tv.tv_sec = 1;
	tv.tv_usec = 1;
	rv = select(0, NULL, NULL, NULL, &tv);
	return (rv == -1 && errno != EINTR);
}
" HAVE_SELECT)
	if (HAVE_SELECT)
		BB_Save_Define(HAVE_SELECT)
	else()
		BB_Save_Undef(HAVE_SELECT)
	endif()
endmacro()

#
# From BSDBuild/pthreads.pm:
#
macro(Check_Pthreads)
	set(ORIG_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
	set(ORIG_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})

	set(PTHREADS_CFLAGS "")

	find_library(PTHREADS_LIBS NAMES "pthread")

	set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES} ${PTHREADS_LIBS}")
	check_c_source_compiles("
#include <pthread.h>
#include <signal.h>

static void *start_routine(void *arg)
{
	return (NULL);
}
int main(int argc, char *argv[])
{
	pthread_mutex_t mutex;
	pthread_t thread;
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_lock(&mutex);
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);
	pthread_create(&thread, NULL, start_routine, NULL);
	return (0);
}
" HAVE_PTHREADS)
	if (HAVE_PTHREADS)
		BB_Save_Define(HAVE_PTHREADS)
	else()
		#
		# Check for the -pthread flag (older OpenBSD, etc.)
		#
		set(CMAKE_REQUIRED_FLAGS "${ORIG_CMAKE_REQUIRED_FLAGS} -pthread")
		check_c_source_compiles("
#include <pthread.h>
#include <signal.h>

static void *start_routine(void *arg)
{
	return (NULL);
}
int main(int argc, char *argv[])
{
	pthread_mutex_t mutex;
	pthread_t thread;
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_lock(&mutex);
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);
	pthread_create(&thread, NULL, start_routine, NULL);
	return (0);
}
" HAVE_PTHREADS_PTHREAD_FLAG)
		if (HAVE_PTHREADS_PTHREAD_FLAG)
			set(PTHREADS_CFLAGS "-pthread")
			set(PTHREADS_LIBS "")
			set(HAVE_PTHREADS ON)
			BB_Save_Define(HAVE_PTHREADS)
		else()
			BB_Save_Undef(HAVE_PTHREADS)
		endif()
		set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
	endif()

	check_c_source_compiles("
#include <pthread.h>
#include <signal.h>

int main(int argc, char *argv[])
{
	pthread_mutex_t mutex;
	pthread_mutexattr_t mutexattr;
	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&mutex, &mutexattr);
	return (0);
}
" HAVE_PTHREAD_MUTEX_RECURSIVE)
	if(HAVE_PTHREAD_MUTEX_RECURSIVE)
		BB_Save_Define(HAVE_PTHREAD_MUTEX_RECURSIVE)
	else()
		BB_Save_Undef(HAVE_PTHREAD_MUTEX_RECURSIVE)
	endif()

	check_c_source_compiles("
#include <pthread.h>
#include <signal.h>

int main(int argc, char *argv[])
{
	pthread_mutex_t mutex;
	pthread_mutexattr_t mutexattr;
	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(&mutex, &mutexattr);
	return (0);
}
" HAVE_PTHREAD_MUTEX_RECURSIVE_NP)
	if(HAVE_PTHREAD_MUTEX_RECURSIVE_NP)
		BB_Save_Define(HAVE_PTHREAD_MUTEX_RECURSIVE_NP)
	else()
		BB_Save_Undef(HAVE_PTHREAD_MUTEX_RECURSIVE_NP)
	endif()

	check_c_source_compiles("
#include <pthread.h>

int main(int argc, char *argv[])
{
	pthread_mutex_t mutex = NULL;
	pthread_mutex_init(&mutex, NULL);
	return (mutex != NULL);
}
" HAVE_PTHREAD_MUTEX_T_POINTER)
	if(HAVE_PTHREAD_MUTEX_T_POINTER)
		BB_Save_Define(HAVE_PTHREAD_MUTEX_T_POINTER)
	else()
		BB_Save_Undef(HAVE_PTHREAD_MUTEX_T_POINTER)
	endif()

	check_c_source_compiles("
#include <pthread.h>

int main(int argc, char *argv[])
{
	pthread_cond_t cond = NULL;
	pthread_cond_init(&cond, NULL);
	return (cond != NULL);
}
" HAVE_PTHREAD_COND_T_POINTER)
	if(HAVE_PTHREAD_COND_T_POINTER)
		BB_Save_Define(HAVE_PTHREAD_COND_T_POINTER)
	else()
		BB_Save_Undef(HAVE_PTHREAD_COND_T_POINTER)
	endif()

	check_c_source_compiles("
#include <pthread.h>
static void *start_routine(void *arg) { return (NULL); }
int main(int argc, char *argv[])
{
	pthread_t th = NULL;
	return pthread_create(&th, NULL, start_routine, NULL);
}
" HAVE_PTHREAD_T_POINTER)
	if(HAVE_PTHREAD_T_POINTER)
		BB_Save_Define(HAVE_PTHREAD_T_POINTER)
	else()
		BB_Save_Undef(HAVE_PTHREAD_T_POINTER)
	endif()

	if(NOT FREEBSD)
		set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -U_XOPEN_SOURCE -D_XOPEN_SOURCE=600")
	endif()
	check_c_source_compiles("
#include <pthread.h>
#include <signal.h>

int main(int argc, char *argv[])
{
	pthread_mutex_t mutex;
	pthread_mutexattr_t mutexattr;
	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&mutex, &mutexattr);
	return (0);
}
" HAVE_PTHREADS_XOPEN)
	if(HAVE_PTHREADS_XOPEN)
		if(NOT FREEBSD)
			set(PTHREADS_XOPEN_CFLAGS "-U_XOPEN_SOURCE -D_XOPEN_SOURCE=600")
		endif()
		BB_Save_Define(HAVE_PTHREADS_XOPEN)
	else()
		BB_Save_Undef(HAVE_PTHREADS_XOPEN)
	endif()

	BB_Save_MakeVar(PTHREADS_CFLAGS "${PTHREADS_CFLAGS}")
	BB_Save_MakeVar(PTHREADS_LIBS "${PTHREADS_LIBS}")
	BB_Save_MakeVar(PTHREADS_XOPEN_CFLAGS "${PTHREADS_XOPEN_CFLAGS}")
	BB_Save_MakeVar(PTHREADS_XOPEN_LIBS "")

	set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
	set(CMAKE_REQUIRED_LIBRARIES ${ORIG_CMAKE_REQUIRED_LIBRARIES})
endmacro()

macro(Disable_Pthreads)
	BB_Save_MakeVar(PTHREADS_CFLAGS "")
	BB_Save_MakeVar(PTHREADS_LIBS "")
	BB_Save_MakeVar(PTHREADS_XOPEN_CFLAGS "")
	BB_Save_MakeVar(PTHREADS_XOPEN_LIBS "")

	BB_Save_Undef(HAVE_PTHREADS)
	BB_Save_Undef(HAVE_PTHREADS_XOPEN)
	BB_Save_Undef(HAVE_PTHREAD_MUTEX_RECURSIVE)
	BB_Save_Undef(HAVE_PTHREAD_MUTEX_RECURSIVE_NP)
	BB_Save_Undef(HAVE_PTHREAD_MUTEX_T_POINTER)
	BB_Save_Undef(HAVE_PTHREAD_COND_T_POINTER)
	BB_Save_Undef(HAVE_PTHREAD_T_POINTER)
endmacro()

#
# From BSDBuild/strsep.pm:
#
macro(Check_Strsep)
	check_c_source_compiles("
#include <string.h>

int
main(int argc, char *argv[])
{
	char foo[32], *pFoo = &foo[0];
	char *s;

	foo[0] = 0;
	s = strsep(&pFoo, \" \");
	return (s != NULL);
}
" HAVE_STRSEP)
	if (HAVE_STRSEP)
		BB_Save_Define(HAVE_STRSEP)
	else()
		BB_Save_Undef(HAVE_STRSEP)
	endif()
endmacro()
