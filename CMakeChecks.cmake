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

macro(BB_Save_Define arg)
	string(TOLOWER "${arg}" arg_lower)
	file(WRITE "${CONFIG_DIR}/${arg_lower}.h" "#ifndef ${arg}
#define ${arg} \"yes\"
#endif
")
	file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/Makefile.config" "${arg}=yes
")
endmacro()

macro(BB_Save_Define_Value arg val)
	string(TOLOWER "${arg}" arg_lower)
	file(WRITE "${CONFIG_DIR}/${arg_lower}.h" "#ifndef ${arg}
#define ${arg} \"${val}\"
#endif
")
	file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/Makefile.config" "${arg}=\"${val}\"
")
endmacro()

macro(BB_Save_Define_Value_Bare arg val)
	string(TOLOWER "${arg}" arg_lower)
	file(WRITE "${CONFIG_DIR}/${arg_lower}.h" "#ifndef ${arg}
#define ${arg} ${val}
#endif
")
	file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/Makefile.config" "${arg}=\"${val}\"
")
endmacro()


macro(BB_Save_Undef arg)
	string(TOLOWER "${arg}" arg_lower)
	file(WRITE "${CONFIG_DIR}/${arg_lower}.h" "#undef ${arg}
")
	file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/Makefile.config" "${arg}=no
")
endmacro()

macro(BB_Save_MakeVar arg val)
	file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/Makefile.config" "${arg}=${val}
")
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
" HAVE_STRTOLL)
	if (HAVE_STRTOLL)
		BB_Save_Define(HAVE_STRTOLL)
	else()
		BB_Save_Undef(HAVE_STRTOLL)
	endif()
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
	# TODO
endmacro()

macro(Disable_Db4)
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
# From BSDBuild/mysql.pm:
#
macro(Check_Mysql)
	# TODO
endmacro()

macro(Disable_Mysql)
	BB_Save_MakeVar(MYSQL_CFLAGS "")
	BB_Save_MakeVar(MYSQL_LIBS "")
	BB_Save_Undef(HAVE_MYSQL)
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
		if(${Intl_INCLUDE_DIRS})
			set(GETTEXT_CFLAGS "-I${Intl_INCLUDE_DIRS}")
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
	set(PTHREADS_LIBS "-lpthread")

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
	BB_Save_MakeVar(PTHREADS_XOPEN_LIBS "${PTHREADS_XOPEN_LIBS}")

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
