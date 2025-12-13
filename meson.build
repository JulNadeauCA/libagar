project('agar', 'c', 'cpp', 'objc',
  version: '1.8.0',
  license: 'BSD-2-Clause',
  meson_version: '>=0.60.0',
  default_options: [
    'c_std=c11',  # Use c11 for portable code (no GNU extensions)
    'warning_level=1',
    'buildtype=release'
  ]
)

# ============================================================================
# Project Information
# ============================================================================

agar_version = meson.project_version()
agar_release = 'Pyramid Magic'
agar_soversion = '9.0.0'
agar_soversion_major = '9'

message('    _       _       _     ___')
message('  / _ \\   / _ \\   / _ \\  |  _ \\')
message(' | |_| | | (_| | | |_| | | |_) |')
message(' |_| |_|  \\__, | |_| |_| |_| |_|')
message('          |___/                 ')
message('')
message('Agar version @0@ ("@1@")'.format(agar_version, agar_release))

# ============================================================================
# Compiler and System Setup
# ============================================================================

cc = meson.get_compiler('c')
host_system = host_machine.system()
host_cpu = host_machine.cpu_family()

# Platform defines
platform_defs = []
if host_system == 'darwin'
  platform_defs += '-D_DARWIN_C_SOURCE'
elif host_system == 'netbsd'
  platform_defs += '-D_NETBSD_SOURCE'
endif

# Global defines
global_defs = [
  '-D_AGAR_INTERNAL',
  '-D_DEFAULT_SOURCE',
  '-D_BSD_SOURCE',
] + platform_defs

# ============================================================================
# Configuration Options
# ============================================================================

# Check for conflicting options
if get_option('sdl') and get_option('sdl2')
  error('Conflict: Cannot build with both sdl and sdl2 options')
endif

# Determine memory model
memory_model_opt = get_option('memory_model')
if memory_model_opt == 'auto'
  # Auto-detect based on pointer size
  if host_cpu.contains('x86_64') or host_cpu.contains('aarch64') or host_cpu.contains('ppc64')
    memory_model = 'large'
  elif host_cpu.contains('bbc') or host_cpu.contains('c64') or host_cpu.contains('nes')
    memory_model = 'small'
  else
    memory_model = 'medium'
  endif
else
  memory_model = memory_model_opt
endif

message('Memory model: @0@'.format(memory_model.to_upper()))

# Disable inline functions if requested
if not get_option('inline')
  foreach opt : ['inline_byteswap', 'inline_error', 'inline_event', 'inline_io',
                 'inline_object', 'inline_string', 'inline_tbl', 'inline_threads',
                 'inline_variable', 'inline_surface', 'inline_widget']
    set_variable(opt, false)
  endforeach
endif

# ============================================================================
# Feature Detection
# ============================================================================

# Check for standard headers
have_float_h = cc.has_header('float.h')
have_dl_h = cc.has_header('dl.h')
have_dlfcn_h = cc.has_header('dlfcn.h')
have_limits_h = cc.has_header('limits.h')
have_mach_o_dyld_h = cc.has_header('mach-o/dyld.h')
have_net_if_h = cc.has_header('net/if.h')
have_sys_stat_h = cc.has_header('sys/stat.h')
have_sys_types_h = cc.has_header('sys/types.h')
have_stdint_h = cc.has_header('stdint.h')
have_stdlib_h = cc.has_header('stdlib.h')
have_sys_param_h = cc.has_header('sys/param.h')
have_sys_uio_h = cc.has_header('sys/uio.h')
have_unistd_h = cc.has_header('unistd.h')

# Check for functions
have_asprintf = cc.has_function('asprintf', prefix: '#include <stdio.h>')
have_execvp = cc.has_function('execvp', prefix: '#include <unistd.h>')
have_getenv = cc.has_function('getenv')
have_getpwnam_r = cc.has_function('getpwnam_r', prefix: '#include <pwd.h>')
have_getpwuid = cc.has_function('getpwuid', prefix: '#include <pwd.h>')
have_getopt = cc.has_function('getopt', prefix: '#include <getopt.h>')
have_gettimeofday = cc.has_function('gettimeofday', prefix: '#include <sys/time.h>')
have_getuid = cc.has_function('getuid', prefix: '#include <unistd.h>')
have_glob = cc.has_function('glob', prefix: '#include <glob.h>')
have_kqueue = cc.has_function('kqueue', prefix: '#include <sys/event.h>')
have_mprotect = cc.has_function('mprotect', prefix: '#include <sys/mman.h>')
have_nanosleep = cc.has_function('nanosleep', prefix: '#include <time.h>')
have_select = cc.has_function('select', prefix: '#include <sys/select.h>')
have_snprintf = cc.has_function('snprintf', prefix: '#include <stdio.h>')
have_strsep = cc.has_function('strsep', prefix: '#include <string.h>')
have_strtold = cc.has_function('strtold', prefix: '#include <stdlib.h>')
have_strtoll = cc.has_function('strtoll', prefix: '#include <stdlib.h>')
have_vasprintf = cc.has_function('vasprintf', prefix: '#include <stdio.h>')
have_vsnprintf = cc.has_function('vsnprintf', prefix: '#include <stdio.h>')

# Check for the SO_ACCEPTFILTER socket option
so_acceptfilter_test = '''
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <fcntl.h>
  int main() {
    int fd=0, rv;
    struct accept_filter_arg afa;
    socklen_t afaLen = sizeof(afa);
    afa.af_name[0] = 'A';
    afa.af_arg[0] = 'A';
    rv = setsockopt(fd, SOL_SOCKET, SO_ACCEPTFILTER, &afa, afaLen);
    return (rv != 0);
  }
'''
have_so_acceptfilter = cc.links(so_acceptfilter_test, name: 'sockopt SO_ACCEPTFILTER')

# Check for the SO_LINGER socket option
so_linger_test = '''
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
int main() {
  int fd=0, rv;
  struct linger ling;
  socklen_t lingLen=sizeof(ling);
  ling.l_onoff=1; ling.l_linger=1;
  rv = setsockopt(fd, SOL_SOCKET, SO_LINGER, &ling, lingLen);
  return (rv != 0);
}
'''
have_so_linger = cc.links(so_linger_test, name: 'sockopt SO_LINGER')

# Check for the SO_NOSIGPIPE socket option
so_nosigpipe_test = '''
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
int main() {
	int fd=0, val=1, rv;
	socklen_t len = sizeof(val);
	rv = setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &val, len);
	return (rv != 0);
}
'''
have_so_nosigpipe = cc.links(so_nosigpipe_test, name: 'sockopt SO_NOSIGPIPE')

# Check for the SO_OOBINLINE socket option
so_oobinline_test = '''
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
int main() {
	int fd=0, val=1, rv;
	socklen_t len = sizeof(val);
	rv = setsockopt(fd, SOL_SOCKET, SO_OOBINLINE, &val, len);
	return (rv != 0);
}
'''
have_so_oobinline = cc.links(so_oobinline_test, name: 'sockopt SO_OOBINLINE')

# Check for the SO_REUSEPORT socket option
so_reuseport_test = '''
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
int main() {
	int fd=0, val=1, rv;
	socklen_t len = sizeof(val);
	rv = setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &val, len);
	return (rv != 0);
}
'''
have_so_reuseport = cc.links(so_reuseport_test, name: 'sockopt SO_REUSEPORT')

# Check for the SO_TIMESTAMP socket option
so_timestamp_test = '''
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
int main() {
	int fd=0, val=1, rv;
	socklen_t len = sizeof(val);
	rv = setsockopt(fd, SOL_SOCKET, SO_TIMESTAMP, &val, len);
	return (rv != 0);
}
'''
have_so_timestamp = cc.links(so_timestamp_test, name: 'sockopt SO_TIMESTAMP')

# Check for clock_gettime (may need librt)
librt = cc.find_library('rt', required: false)
have_clock_gettime = cc.has_function('clock_gettime', prefix: '#include <time.h>', dependencies: librt)

# Check for dlopen function (may need libdl)
libdl = cc.find_library('dl', required: false)
have_dlopen = cc.has_function('dlopen', prefix: '#include <dlfcn.h>', dependencies: libdl)

# Check for mach-o dyld
if have_mach_o_dyld_h
  have_dyld = cc.has_function('dyld', prefix: '#include <mach-o/dyld.h>')
  if have_dyld
    dyld_return_on_error_test = '''
      #include <mach-o/dyld.h>
      int main() {
        NSObjectFileImage img;
        NSObjectFileImageReturnCode rv;
        void *handle;

        rv = NSCreateObjectFileImageFromFile("foo", &img);
        handle = (void *)NSLinkModule(img, "foo", NSLINKMODULE_OPTION_RETURN_ON_ERROR|
                                                  NSLINKMODULE_OPTION_NONE);
        if (handle == NULL) {
          NSLinkEditErrors errs;
          int n;
          const char *f, *s = NULL;
          NSLinkEditError(&errs, &n, &f, &s);
        }
        return 0;
      }
      '''
      have_dyld_return_on_error = cc.links(dyld_return_on_error_test,
        name: 'NSLINKMODULE_OPTION_RETURN_ON_ERROR')
  else
      have_dyld_return_on_error = false
  endif
else
  have_dyld = false
  have_dyld_return_on_error = false
endif

# Check for shl_load function (needs libdld)
if have_dl_h
  libdld = cc.find_library('dld', required: false)
  have_shl_load = cc.has_function('shl_load', prefix: '#include <dl.h>', dependencies: libdld)
else
  have_shl_load = false
endif

# Check for the SIOCGIFCONF interface
if have_net_if_h
  siocgifconf_test = '''
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <sys/ioctl.h>
  #include <sys/time.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <netinet/in.h>
  #include <net/if.h>
  #include <netdb.h>
  int main() {
    char buf[4096];
    struct ifconf ic;
    struct ifreq *ifr;
    int sock;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) { return (1); }
    ic.ifc_len = sizeof(buf);
    ic.ifc_buf = (caddr_t)buf;
    if (ioctl(sock, SIOCGIFCONF, &ic) < 0) { return (1); }
    #if !defined(_SIZEOF_ADDR_IFREQ)
    #define _SIZEOF_ADDR_IFREQ sizeof
    #endif
    for (ifr = (struct ifreq *)buf;
         (char *)ifr < &buf[ic.ifc_len];
         ifr = (struct ifreq *)((char *)ifr + _SIZEOF_ADDR_IFREQ(*ifr))) {
      if (ifr->ifr_addr.sa_family == AF_INET)
        return (1);
    }
    close(sock);
    return (0);
  }
  '''
  have_siocgifconf = cc.links(siocgifconf_test, name : 'ioctl SIOCGIFCONF')
else
  have_siocgifconf = false
endif

# Check for math library
libm = cc.find_library('m', required: false)
have_math = cc.has_function('sin', prefix: '#include <math.h>', dependencies: libm)

# Check for C99 math functions
have_math_c99 = cc.has_function('sinf', prefix: '#include <math.h>', dependencies: libm)

# Check for signal support
have_signal = cc.has_function('signal', prefix: '#include <signal.h>')

# Check for long double
have_long_double = cc.has_type('long double', prefix: '#include <stdlib.h>')

# Check for rand48 functions (drand48, lrand48)
have_rand48 = cc.has_function('drand48', prefix: '#include <stdlib.h>')

# Check for getaddrinfo (networking)
have_getaddrinfo = cc.has_function('getaddrinfo', prefix: '#include <netdb.h>')

# Check for timerfd (Linux-specific)
have_timerfd = cc.has_header_symbol('sys/timerfd.h', 'timerfd_create')

# Check for dirfd function
have_dirfd = cc.has_function('dirfd', prefix: '#include <dirent.h>')

# Check for fdclose function
have_fdclose = cc.has_function('fdclose', prefix: '#include <stdio.h>')

# ============================================================================
# Dependency Detection
# ============================================================================

# Threads
thread_dep = dependency('threads', required: get_option('threads'))
have_pthreads = thread_dep.found()
have_pthread_recursive = false

if have_pthreads
  # Check for recursive mutex support
  # Need _GNU_SOURCE or _DEFAULT_SOURCE for POSIX extensions
  pthread_test = '''
    #define _GNU_SOURCE
    #include <pthread.h>
    int main() {
      pthread_mutex_t mutex;
      pthread_mutexattr_t mutexattr;
      pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
      pthread_mutex_init(&mutex, &mutexattr);
      return 0;
    }
  '''
  have_pthread_recursive = cc.links(pthread_test, dependencies: thread_dep,
                                    name: 'PTHREAD_MUTEX_RECURSIVE')

  # Check for non-portable recursive mutex constant (Linux/glibc specific)
  pthread_np_test = '''
    #define _GNU_SOURCE
    #include <pthread.h>
    int main() {
      pthread_mutex_t mutex;
      pthread_mutexattr_t mutexattr;
      pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
      pthread_mutex_init(&mutex, &mutexattr);
      return 0;
    }
  '''
  have_pthread_recursive_np = cc.links(pthread_np_test, dependencies: thread_dep,
                                       name: 'PTHREAD_MUTEX_RECURSIVE_NP')
else
  have_pthread_recursive_np = false
endif

# Math library (required)
math_dep = libm

#
# Agar-GUI dependencies
# 
if get_option('gui')
  
  # FreeType (recommended)
  if get_option('freetype')
    freetype_dep = dependency('freetype2', version: '>=7.0.1', required: true)
  endif

  # Fontconfig
  if get_option('fontconfig')
    fontconfig_dep = dependency('fontconfig', version: '>=2.6.0', required: true)
  endif

  # SDL 1.2
  if get_option('sdl')
    sdl_dep = dependency('sdl', version: '>=1.2.0', required: true)
  endif

  # SDL 2.0
  if get_option('sdl2')
    sdl2_dep = dependency('sdl2', version: '>=2.0.22', required: true)
  endif

  # OpenGL
  if get_option('gl')
    gl_dep = dependency('gl', required: true)
    have_opengl = gl_dep.found()
  else
    have_opengl = false
  endif

  # X11 (for GUI GLX driver)
  if get_option('x11')
    x11_dep = dependency('x11', required: true)
    have_x11 = x11_dep.found()

    xkb_test = '''
    #include <X11/Xlib.h>
    #include <X11/XKBlib.h>
    int main(int argc, char *argv[])
    {
      Display *D = XOpenDisplay(NULL);
      KeyCode kc = 0;
      KeySym ks = XkbKeycodeToKeysym(D, kc, 0, 0);
      XCloseDisplay(D);
      return (ks != NoSymbol);
    }
    '''
    have_xkb = cc.links(xkb_test, dependencies: x11_dep,
      name : 'libX11 XKB extension')

    xf86misc_test = '''
    #include <X11/Xlib.h>
    #include <X11/extensions/xf86misc.h>
    int main() {
      Display *D = XOpenDisplay(NULL);
      int d,rv;
      rv = XF86MiscQueryExtension(D, &d, &d);
      XCloseDisplay(D);
      return (rv != 0);
    }
    '''
    have_xf86misc = cc.links(xf86misc_test,
      args : '-lXxf86misc',
      dependencies : x11_dep,
      name : 'libX11 XF86MISC extension')
  else
    have_x11 = false
    have_xkb = false
    have_xf86misc = false
  endif

  # Xinerama (for GUI)
  if get_option('xinerama')
    xinerama_dep = dependency('xinerama', required: true)
  endif

  # GLX is part of OpenGL on most systems
  have_glx = have_opengl and have_x11

  # JPEG image support
  if get_option('jpeg')
    jpeg_dep = dependency('libjpeg', required: true)
  endif

  # PNG image support
  if get_option('png')
    png_dep = dependency('libpng', version: '>=1.2.49', required: true)
  endif

endif

#
# Agar-SG dependencies
#
if get_option('sg')

  if get_option('glu')
    glu_dep = dependency('glu', required: true)
  endif

endif

#
# Agar-Net dependencies
#
if get_option('net') and get_option('web')

  # zlib (for the Agar-Net HTTP server)
  if get_option('zlib')
    zlib_dep = dependency('zlib', required: true)
  endif

endif

# iconv (character set conversion support)
if get_option('iconv')
  iconv_dep = dependency('iconv', required: true)
endif

# gettext (for NLS)
if get_option('nls')
  gettext_dep = dependency('gettext', required: true)
  intl_dep = dependency('intl', required: false)
else
  gettext_dep = disabler()
  intl_dep = disabler()
endif

# Berkeley DB (for CORE AG_Db)
if get_option('db4')
  db4_dep = dependency('db4', required: true)
endif

# MySQL (for CORE AG_Db)
if get_option('mysql')
  mysql_dep = dependency('mysqlclient', version: '>=5.5.8', required: true)
endif

# libsndfile (for AU)
if get_option('au')
  if get_option('sndfile')
    sndfile_dep = dependency('sndfile', version: '>=1.0.21', required: true)
  endif
  if get_option('portaudio')
    portaudio_dep = dependency('portaudio-2.0', version: '>=19', required: true)
  endif
endif

# ============================================================================
# Compiler Feature Detection
# ============================================================================

# Check for compiler attributes
have_aligned_attribute = cc.has_function_attribute('aligned')
# bounded is OpenBSD-specific, check manually
bounded_test = '''
  __attribute__((__bounded__(__string__, 1, 2)))
  void test(char *buf, size_t len);
  void test(char *buf, size_t len) {}
  int main() { return 0; }
'''
have_bounded_attribute = cc.compiles(bounded_test, name: 'C supports function attribute bounded')
have_const_attribute = cc.has_function_attribute('const')
have_deprecated_attribute = cc.has_function_attribute('deprecated')
have_format_attribute = cc.has_function_attribute('format')
have_malloc_attribute = cc.has_function_attribute('malloc')
have_noreturn_attribute = cc.has_function_attribute('noreturn')
have_packed_attribute = cc.has_function_attribute('packed')
have_pure_attribute = cc.has_function_attribute('pure')
have_unused_attribute = cc.has_function_attribute('unused')
have_warn_unused_result_attribute = cc.has_function_attribute('warn_unused_result')

# Check for SSE support
if get_option('sse')
  sse_test = '''
    #include <xmmintrin.h>
    int main() {
      __m128 a;
      return 0;
    }
  '''

  sse2_test = '''
  #include <emmintrin.h>
  int main() {
    double a[4] __attribute__ ((aligned(16)));
    double b[4] __attribute__ ((aligned(16)));
    double rv;
    __m128d v1,v2;
    a[0]=1.0f; a[1]=2.0f; a[2]=3.0f; a[3]=4.0f;
    b[0]=1.0f; b[1]=2.0f; b[2]=3.0f; b[3]=4.0f;
    v1 = _mm_load_pd(a);
    v2 = _mm_load_pd(b);
    v1 = _mm_xor_pd(v1, v2);
    _mm_store_sd(&rv, v1);
    return (0);
  }
  '''

  sse3_test = '''
  #include <pmmintrin.h>
  int main() {
    float a[4] __attribute__ ((aligned(16)));
    float b[4] __attribute__ ((aligned(16)));
    __m128 v1,v2;
    float rv;
    a[0]=1.0f; a[1]=2.0f; a[2]=3.0f; a[3]=4.0f;
    b[0]=1.0f; b[1]=2.0f; b[2]=3.0f; b[3]=4.0f;
    v1 = _mm_load_ps(a);
    v2 = _mm_load_ps(b);
    v1 = _mm_mul_ps(v1, v2);
    v1 = _mm_hadd_ps(v1, v1);
    v1 = _mm_hadd_ps(v1, v1);
    _mm_store_ss(&rv, v1);
    return (0);
  }
  '''

  have_sse = cc.compiles(sse_test, args: ['-msse'], name: 'SSE intrinsics')
  if have_sse
    sse_args = ['-msse']
    if get_option('sse2')
      have_sse2 = cc.compiles(sse2_test, args: ['-msse2'], name: 'SSE2 intrinsics')
      if have_sse2
        sse_args += '-msse2'
      endif
    endif
    if get_option('sse3')
      have_sse3 = cc.compiles(sse3_test, args: ['-msse3'], name: 'SSE3 intrinsics')
      if have_sse3
        sse_args += '-msse3'
      endif
    endif
  else
    sse_args = []
  endif
else
  have_sse = false
  have_sse2 = false
  have_sse3 = false
  sse_args = []
endif

# Check for AltiVec support
if get_option('altivec')
  altivec_test = '''
    #include <altivec.h>
    int main() {
      vector unsigned int a;
      return 0;
    }
  '''
  have_altivec = cc.compiles(altivec_test, args: ['-maltivec'], name: 'AltiVec intrinsics')
  if have_altivec
    altivec_args = ['-maltivec']
  else
    altivec_args = []
  endif
else
  have_altivec = false
  altivec_args = []
endif

# ============================================================================
# Warning Flags
# ============================================================================

warning_flags = []
if get_option('warnings')
  warning_flags = ['-Wall', '-Werror', '-Wmissing-prototypes', '-Wno-switch']
  if host_system == 'darwin'
    warning_flags += '-Wno-deprecated-declarations'
  endif
endif

# ============================================================================
# Configuration Data for config.h
# ============================================================================

cdata = configuration_data()

# Version information
cdata.set_quoted('VERSION', agar_version)
cdata.set_quoted('RELEASE', agar_release)

# Memory model
if memory_model == 'small'
  cdata.set('AG_MODEL', 'AG_SMALL')
elif memory_model == 'medium'
  cdata.set('AG_MODEL', 'AG_MEDIUM')
else
  cdata.set('AG_MODEL', 'AG_LARGE')
endif

# Build type
if get_option('buildtype').startswith('debug')
  cdata.set('AG_DEBUG', 1)
  cdata.set('AG_TYPE_SAFETY', 1)
elif get_option('type_safety')
  cdata.set('AG_TYPE_SAFETY', 1)
endif

# Feature options
cdata.set10('AG_ANSI_COLOR',    get_option('ansi_color'))
cdata.set10('AG_ENABLE_DSO',    get_option('dso'))
cdata.set10('AG_ENABLE_EXEC',   get_option('exec'))
cdata.set10('AG_ENABLE_STRING', get_option('string'))
cdata.set10('AG_EVENT_LOOP',    get_option('event_loop'))
cdata.set10('AG_LEGACY',        get_option('legacy'))
cdata.set10('AG_NAMED_ARGS',    get_option('named_args'))
cdata.set10('AG_NAMESPACES',    get_option('namespaces'))
cdata.set10('AG_SERIALIZATION', get_option('serialization'))
cdata.set10('AG_TIMERS',        get_option('timers'))
cdata.set10('AG_UNICODE',       get_option('unicode'))
cdata.set10('AG_USER',          get_option('user'))
cdata.set10('AG_VERBOSITY',     get_option('verbosity'))
cdata.set10('AG_WIDGETS',       get_option('widgets'))
cdata.set10('AG_WM_HINTS',      get_option('wm_hints'))

# Inline options
cdata.set10('AG_INLINE_BYTESWAP', get_option('inline_byteswap'))
cdata.set10('AG_INLINE_ERROR',    get_option('inline_error'))
cdata.set10('AG_INLINE_IO',       get_option('inline_io'))
cdata.set10('AG_INLINE_OBJECT',   get_option('inline_object'))
cdata.set10('AG_INLINE_STRING',   get_option('inline_string'))
cdata.set10('AG_INLINE_SURFACE',  get_option('inline_surface'))
cdata.set10('AG_INLINE_TBL',      get_option('inline_tbl'))
cdata.set10('AG_INLINE_THREADS',  get_option('inline_threads'))
cdata.set10('AG_INLINE_VARIABLE', get_option('inline_variable'))
cdata.set10('AG_INLINE_WIDGET',   get_option('inline_widget'))

# Threads
cdata.set10('AG_THREADS', get_option('threads') and have_pthreads)

# Network
cdata.set10('AG_NETWORK', get_option('net'))
cdata.set10('AG_WEB',     get_option('web'))

# Library enables
cdata.set10('ENABLE_AU',   get_option('au'))
cdata.set10('ENABLE_GUI',  get_option('gui'))
cdata.set10('ENABLE_MAP',  get_option('map'))
cdata.set10('ENABLE_MATH', get_option('math'))
cdata.set10('ENABLE_NET',  get_option('net'))
cdata.set10('ENABLE_SG',   get_option('sg'))
cdata.set10('ENABLE_SK',   get_option('sk'))
cdata.set10('ENABLE_VG',   get_option('vg'))

# System features
cdata.set10('HAVE_ASPRINTF',      have_asprintf)
cdata.set10('HAVE_CLOCK_GETTIME', have_clock_gettime)
cdata.set10('HAVE_DLOPEN',        have_dlopen)
cdata.set10('HAVE_EXECVP',        have_execvp)
cdata.set10('HAVE_FLOAT_H',       have_float_h)
cdata.set10('HAVE_FLOAT',         get_option('float'))
cdata.set10('HAVE_GETADDRINFO',   have_getaddrinfo)
cdata.set10('HAVE_GETENV',        have_getenv)
cdata.set10('HAVE_GETPWNAM_R',    have_getpwnam_r)
cdata.set10('HAVE_GETPWUID',      have_getpwuid)
cdata.set10('HAVE_GETTIMEOFDAY',  have_gettimeofday)
cdata.set10('HAVE_GETUID',        have_getuid)
cdata.set10('HAVE_GETOPT',        have_getopt)
cdata.set10('HAVE_GLOB',          have_glob)
cdata.set10('HAVE_KQUEUE',        have_kqueue)
cdata.set10('HAVE_LIMITS_H',      have_limits_h)
cdata.set10('HAVE_LONG_DOUBLE',   have_long_double)
cdata.set10('HAVE_MATH',          have_math)
cdata.set10('HAVE_MATH_C99',      have_math_c99)
cdata.set10('HAVE_MPROTECT',      have_mprotect)
cdata.set10('HAVE_NANOSLEEP',     have_nanosleep)
cdata.set10('HAVE_PTHREADS',      have_pthreads)
cdata.set10('HAVE_SELECT',        have_select)
cdata.set10('HAVE_SIGNAL',        have_signal)
cdata.set10('HAVE_SNPRINTF',      have_snprintf)
cdata.set10('HAVE_STDINT_H',      have_stdint_h)
cdata.set10('HAVE_STDLIB_H',      have_stdlib_h)
cdata.set10('HAVE_STRSEP',        have_strsep)
cdata.set10('HAVE_STRTOLL',       have_strtoll)
cdata.set10('HAVE_STRTOLD',       have_strtold)
cdata.set10('HAVE_SYS_STAT_H',    have_sys_stat_h)
cdata.set10('HAVE_SYS_PARAM_H',   have_sys_param_h)
cdata.set10('HAVE_SYS_TYPES_H',   have_sys_types_h)
cdata.set10('HAVE_SYS_UIO_H',     have_sys_uio_h)
cdata.set10('HAVE_TIMERFD',       have_timerfd)
cdata.set10('HAVE_UNISTD_H',      have_unistd_h)
cdata.set10('HAVE_VASPRINTF',     have_vasprintf)
cdata.set10('HAVE_VSNPRINTF',     have_vsnprintf)

# Dependencies
cdata.set10('HAVE_ALTIVEC',    get_option('altivec') and have_altivec)
cdata.set10('HAVE_DB4',        get_option('db4') and db4_dep.found())
cdata.set10('HAVE_FREETYPE',   get_option('gui') and get_option('freetype') and freetype_dep.found())
cdata.set10('HAVE_FONTCONFIG', get_option('gui') and get_option('fontconfig') and fontconfig_dep.found())
cdata.set10('HAVE_GLU',        get_option('sg') and get_option('glu') and glu_dep.found())
cdata.set10('HAVE_GLX',        get_option('gui') and get_option('x11') and have_glx)
cdata.set10('HAVE_JPEG',       get_option('gui') and get_option('jpeg') and jpeg_dep.found())
cdata.set10('HAVE_MYSQL',      get_option('mysql') and mysql_dep.found())
cdata.set10('HAVE_OPENGL',     get_option('gui') and have_opengl)
cdata.set10('HAVE_PNG',        get_option('gui') and get_option('png') and png_dep.found())
cdata.set10('HAVE_PORTAUDIO',  get_option('au') and get_option('portaudio') and portaudio_dep.found())
cdata.set10('HAVE_SDL',        get_option('gui') and get_option('sdl') and sdl_dep.found())
cdata.set10('HAVE_SDL2',       get_option('gui') and get_option('sdl2') and sdl2_dep.found())
cdata.set10('HAVE_SNDFILE',    get_option('au') and get_option('sndfile') and sndfile_dep.found())
cdata.set10('HAVE_SSE',        get_option('sse') and have_sse)
cdata.set10('HAVE_SSE2',       get_option('sse2') and have_sse2)
cdata.set10('HAVE_SSE3',       get_option('sse3') and have_sse3)
cdata.set10('HAVE_X11',        get_option('gui') and get_option('x11') and have_x11)
cdata.set10('HAVE_XF86MISC',   get_option('gui') and get_option('x11') and have_xf86misc)
cdata.set10('HAVE_XINERAMA',   get_option('gui') and get_option('xinerama') and xinerama_dep.found())
cdata.set10('HAVE_XKB',        get_option('gui') and get_option('x11') and have_xkb)
cdata.set10('HAVE_ZLIB',       get_option('net') and get_option('web') and get_option('zlib') and zlib_dep.found())

# Compiler attributes
cdata.set10('AG_USE_ATTRIBUTES', get_option('attributes'))

# Math precision
if get_option('math_precision') == 'single'
  cdata.set('SINGLE_PRECISION', 1)
elif get_option('math_precision') == 'double'
  cdata.set('DOUBLE_PRECISION', 1)
else # quad
  cdata.set('QUAD_PRECISION', 1)
endif

# SSE/AltiVec inline
cdata.set10('INLINE_SSE', get_option('sse_inline'))
cdata.set10('INLINE_ALTIVEC', get_option('altivec_inline'))

# ============================================================================
# Generate Config Headers
# ============================================================================

# Create include directory structure
incdir = meson.project_build_root() / 'include' / 'agar'
config_h = configure_file(
  output: 'config.h',
  configuration: cdata
)

# Copy config.h to include/agar/config/
install_headers(config_h, subdir: 'agar/config')

# ============================================================================
# Generate Individual Config Headers (LibAgar expects separate headers)
# ============================================================================

# LibAgar's source code expects individual config headers for each feature
# instead of a monolithic config.h. We generate them using templates.

# Boolean configuration variables (HAVE_* and AG_* features)
bool_configs = {

  # Core AG_* feature flags
  'AG_ANSI_COLOR':     get_option('ansi_color'),
  'AG_DEBUG':          get_option('buildtype').startswith('debug'),
  'AG_ENABLE_DSO':    (get_option('dso')),
  'AG_ENABLE_EXEC':   (get_option('exec') and have_execvp),
  'AG_ENABLE_STRING':  get_option('string'),
  'AG_EVENT_LOOP':     get_option('event_loop'),
  'AG_LEGACY':         get_option('legacy'),
  'AG_NAMED_ARGS':     get_option('named_args'),
  'AG_NAMESPACES':     get_option('namespaces'),
  'AG_SERIALIZATION':  get_option('serialization'),
  'AG_THREADS':       (have_pthreads and get_option('threads')),
  'AG_TIMERS':         get_option('timers'),
  'AG_TYPE_SAFETY':   (get_option('buildtype').startswith('debug') or get_option('type_safety')),
  'AG_UNICODE':        get_option('unicode'),
  'AG_USE_ATTRIBUTES': get_option('attributes'),
  'AG_USER':           get_option('user'),
  'AG_VERBOSITY':      get_option('verbosity'),
  'AG_WEB':            get_option('web'),
  'AG_WIDGETS':        get_option('widgets'),
  'AG_WM_HINTS':       get_option('wm_hints'),

  # Enable flags for libraries
  'ENABLE_GUI': get_option('gui'),
  'ENABLE_NLS': get_option('nls'),

  # Math precision flags
  'SINGLE_PRECISION': get_option('math_precision') == 'single',
  'DOUBLE_PRECISION': get_option('math_precision') == 'double',
  'QUAD_PRECISION':   get_option('math_precision') == 'quad',

  # Platform detection
  'HAVE_64BIT':    host_cpu.contains('64'),
  'HAVE_COCOA':    host_system == 'darwin',
  'HAVE_CSIDL':    host_system == 'windows',
  'HAVE_CYGWIN':   host_system == 'cygwin',
  'HAVE_WGL':      host_system == 'windows',
  'HAVE_WINSOCK1': false,
  'HAVE_WINSOCK2': host_system == 'windows',

  # System headers and build system markers (_MK_*)
  'HAVE_DLFCN_H':         have_dlfcn_h,
  'HAVE_DL_H':            have_dl_h,
  'HAVE_MACH_O_DYLD_H':   have_mach_o_dyld_h,
  '_MK_BIG_ENDIAN':       host_machine.endian() == 'big',
  '_MK_LITTLE_ENDIAN':    host_machine.endian() == 'little',
  '_MK_HAVE_FLOAT_H':     have_float_h,
  '_MK_HAVE_LIMITS_H':    have_limits_h,
  '_MK_HAVE_SIGNAL':      have_signal,
  '_MK_HAVE_STDINT_H':    have_stdint_h,
  '_MK_HAVE_STDLIB_H':    have_stdlib_h,
  '_MK_HAVE_STRTOLD':     have_strtold,
  '_MK_HAVE_STRTOLL':     have_strtoll,
  '_MK_HAVE_SYS_STAT_H':  have_sys_stat_h,
  '_MK_HAVE_SYS_TYPES_H': have_sys_types_h,
  '_MK_HAVE_UNISTD_H':    have_unistd_h,

  # System functions
  'HAVE_CLOCK_GETTIME':        have_clock_gettime,
  'HAVE_DIRFD':                have_dirfd,
  'HAVE_DLOPEN':               have_dlopen,
  'HAVE_DYLD':                 have_dyld,
  'HAVE_DYLD_RETURN_ON_ERROR': have_dyld_return_on_error,
  'HAVE_EXECVP':               have_execvp,
  'HAVE_FDCLOSE':              have_fdclose,
  'HAVE_FLOAT':                get_option('float'),
  'HAVE_GETADDRINFO':          have_getaddrinfo,
  'HAVE_GETENV':               have_getenv,
  'HAVE_GETOPT':               have_getopt,
  'HAVE_GETPWNAM_R':           have_getpwnam_r,
  'HAVE_GETPWUID':             have_getpwuid,
  'HAVE_GETTIMEOFDAY':         have_gettimeofday,
  'HAVE_GETUID':               have_getuid,
  'HAVE_GLOB':                 have_glob,
  'HAVE_INT64_T':              have_stdint_h,             # int64_t from stdint.h
  'HAVE___INT64':              host_system == 'windows',  # Windows __int64 type
  'HAVE_KQUEUE':               have_kqueue,
  'HAVE_LONG_DOUBLE':          have_long_double,
  'HAVE_LONG_LONG':            true,                      # C99 requirement
  'HAVE_MATH':                 have_math,
  'HAVE_MATH_C99':             have_math_c99,
  'HAVE_NANOSLEEP':            have_nanosleep,
  'HAVE_PTHREADS':             have_pthreads,
  'HAVE_PTHREADS_XOPEN':       have_pthreads,             # Same as HAVE_PTHREADS for X/Open threads
  'HAVE_SELECT':               have_select,
  'HAVE_SETSOCKOPT':           have_getaddrinfo,
  'HAVE_SHL_LOAD':             have_shl_load,
  'HAVE_SIOCGIFCONF':          have_siocgifconf,
  'HAVE_SNPRINTF':             have_snprintf,
  'HAVE_SO_ACCEPTFILTER':      have_so_acceptfilter,
  'HAVE_SO_LINGER':            have_so_linger,
  'HAVE_SO_NOSIGPIPE':         have_so_nosigpipe,
  'HAVE_SO_OOBINLINE':         have_so_oobinline,
  'HAVE_SO_REUSEPORT':         have_so_reuseport,
  'HAVE_SO_TIMESTAMP':         have_so_timestamp,
  'HAVE_TIMERFD':              have_timerfd,
  'HAVE_VASPRINTF':            have_vasprintf,
  'HAVE_VSNPRINTF':            have_vsnprintf,

  # Compiler attributes
  'HAVE_ALIGNED_ATTRIBUTE':            have_aligned_attribute,
  'HAVE_BOUNDED_ATTRIBUTE':            have_bounded_attribute,
  'HAVE_CONST_ATTRIBUTE':              have_const_attribute,
  'HAVE_DEPRECATED_ATTRIBUTE':         have_deprecated_attribute,
  'HAVE_FORMAT_ATTRIBUTE':             have_format_attribute,
  'HAVE_MALLOC_ATTRIBUTE':             have_malloc_attribute,
  'HAVE_NORETURN_ATTRIBUTE':           have_noreturn_attribute,
  'HAVE_PACKED_ATTRIBUTE':             have_packed_attribute,
  'HAVE_PURE_ATTRIBUTE':               have_pure_attribute,
  'HAVE_UNUSED_ATTRIBUTE':             have_unused_attribute,
  'HAVE_UNUSED_VARIABLE_ATTRIBUTE':    have_unused_attribute,  # Same as HAVE_UNUSED_ATTRIBUTE
  'HAVE_WARN_UNUSED_RESULT_ATTRIBUTE': have_warn_unused_result_attribute,
  'HAVE_PTHREAD_MUTEX_RECURSIVE':      (have_pthreads and have_pthread_recursive),
  'HAVE_PTHREAD_MUTEX_RECURSIVE_NP':   (have_pthreads and have_pthread_recursive_np),

  # Library dependencies
  'HAVE_ALTIVEC':    get_option('altivec') and have_altivec,
  'HAVE_CG':         false,
  'HAVE_DB4':        get_option('db4') and db4_dep.found(),
  'HAVE_FONTCONFIG': get_option('gui') and get_option('fontconfig') and fontconfig_dep.found(),
  'HAVE_FREETYPE':   get_option('gui') and get_option('freetype') and freetype_dep.found(),
  'HAVE_GETTEXT':    get_option('nls') and gettext_dep.found(),
  'HAVE_GLEXT':      get_option('gui') and have_opengl,  # TODO
  'HAVE_GLU':        get_option('sg') and get_option('glu') and glu_dep.found(),
  'HAVE_GLX':        get_option('gui') and get_option('glx') and have_glx,
  'HAVE_ICONV':      get_option('iconv') and iconv_dep.found(),
  'HAVE_JPEG':       get_option('gui') and get_option('jpeg') and jpeg_dep.found(),
  'HAVE_LIBPNG14':   get_option('gui') and get_option('png') and png_dep.found(),
  'HAVE_OPENGL':     get_option('gui') and have_opengl,
  'HAVE_PNG':        get_option('gui') and get_option('png') and png_dep.found(),
  'HAVE_PORTAUDIO':  get_option('au') and get_option('portaudio') and portaudio_dep.found(),
  'HAVE_SDL':        get_option('gui') and get_option('sdl') and sdl_dep.found(),
  'HAVE_SDL2':       get_option('gui') and get_option('sdl2') and sdl2_dep.found(),
  'HAVE_SNDFILE':    get_option('au') and get_option('sndfile') and sndfile_dep.found(),
  'HAVE_SSE':        get_option('sse') and have_sse,
  'HAVE_SSE2':       get_option('sse2') and have_sse2,
  'HAVE_SSE3':       get_option('sse3') and have_sse3,
  'HAVE_X11':        get_option('gui') and get_option('x11') and x11_dep.found(),
  'HAVE_XF86MISC':   get_option('gui') and get_option('x11') and have_xf86misc,
  'HAVE_XINERAMA':   get_option('gui') and get_option('xinerama') and xinerama_dep.found(),
  'HAVE_XKB':        get_option('gui') and get_option('x11') and have_xkb,
  'HAVE_ZLIB':       get_option('net') and get_option('web') and get_option('zlib') and zlib_dep.found(),

  # Test-specific dependencies
  'HAVE_AGAR_AU':   get_option('au'),
  'HAVE_AGAR_MATH': get_option('math'),
  'HAVE_RAND48':    have_rand48,

  # Inline options
  'INLINE_SSE':     get_option('sse_inline'),
  'INLINE_ALTIVEC': get_option('altivec_inline'),
}

# Determine AG_MODEL value (must be a symbolic constant, not a boolean)
if memory_model == 'small'
  ag_model_value = 'AG_SMALL'
elif memory_model == 'medium'
  ag_model_value = 'AG_MEDIUM'
else
  ag_model_value = 'AG_LARGE'
endif

# String configuration variables (quoted strings)
string_configs = {
  'VERSION': agar_version,
  'RELEASE': agar_release,
  'DATADIR': get_option('prefix') / get_option('datadir'),
  'LOCALEDIR': get_option('prefix') / get_option('datadir') / 'locale',
  'TTFDIR': get_option('prefix') / get_option('datadir') / 'fonts',
}

# Symbol configuration variables (unquoted symbolic constants)
symbol_configs = {
  # AG_MODEL must be a symbolic constant (AG_SMALL=16, AG_MEDIUM=32, AG_LARGE=64)
  'AG_MODEL': ag_model_value,
}

# Generate boolean config headers
# Generate to build root with final names (Meson doesn't allow subdirs in output)
# Then create symlinks in agar/config/ directory structure
bool_header_files = []
foreach name, value : bool_configs
  template = value ? 'config_bool_yes.h.in' : 'config_bool_no.h.in'
  cfg = configuration_data()
  cfg.set('MACRO', name)

  header = configure_file(
    input: 'meson/templates/' + template,
    output: name.to_lower() + '.h',
    configuration: cfg,
    install_dir: get_option('includedir') / 'agar/config'
  )
  bool_header_files += [header]
endforeach

# Generate string config headers
string_header_files = []
foreach name, value : string_configs
  cfg = configuration_data()
  cfg.set('MACRO', name)
  cfg.set('VALUE', value)

  header = configure_file(
    input: 'meson/templates/config_string.h.in',
    output: name.to_lower() + '.h',
    configuration: cfg,
    install_dir: get_option('includedir') / 'agar/config'
  )
  string_header_files += [header]
endforeach

# Generate symbol config headers (for symbolic constants without quotes)
symbol_header_files = []
foreach name, value : symbol_configs
  cfg = configuration_data()
  cfg.set('MACRO', name)
  cfg.set('VALUE', value)

  header = configure_file(
    input: 'meson/templates/config_symbol.h.in',
    output: name.to_lower() + '.h',
    configuration: cfg,
    install_dir: get_option('includedir') / 'agar/config'
  )
  symbol_header_files += [header]
endforeach

# Create directory structure for headers at configure time
# The source expects #include <agar/core/...>, #include <agar/gui/...>, etc.
# and #include <agar/config/...> for generated config headers.

builddir = meson.project_build_root()
sourcedir = meson.project_source_root()

# Create agar directory structure in build directory
run_command('mkdir', '-p', builddir / 'agar' / 'config', check: true)

# Symlink source library directories into build/agar/
foreach lib : ['core', 'gui', 'math', 'net', 'vg', 'sg', 'sk', 'au', 'map']
  if run_command('test', '-d', sourcedir / lib, check: false).returncode() == 0
    run_command('ln', '-sfn', sourcedir / lib, builddir / 'agar' / lib, check: false)

    # Create top-level convenience headers (e.g., agar/core.h)
    # These are needed for #include <agar/core.h> style includes in applications
    # Prefer the public header (*_pub.h) if it exists, otherwise use the library header
    pub_header = sourcedir / lib / (lib + '_pub.h')
    lib_header = sourcedir / lib / (lib + '.h')

    if run_command('test', '-f', pub_header, check: false).returncode() == 0
      # Copy the public header (includes all widgets/modules)
      run_command('cp', pub_header, builddir / 'agar' / (lib + '.h'), check: false)
      run_command('cp', pub_header, builddir / (lib + '.h'), check: false)
    elif run_command('test', '-f', lib_header, check: false).returncode() == 0
      # Fallback to symlinking the library header
      run_command('ln', '-sfn', lib / (lib + '.h'), builddir / 'agar' / (lib + '.h'), check: false)
    endif
  endif
endforeach

# Symlink generated config headers into build/agar/config/
# This is done after config headers are generated
foreach name, value : bool_configs
  header_file = name.to_lower() + '.h'
  run_command('ln', '-sfn', builddir / header_file, builddir / 'agar' / 'config' / header_file, check: false)
endforeach

foreach name, value : string_configs
  header_file = name.to_lower() + '.h'
  run_command('ln', '-sfn', builddir / header_file, builddir / 'agar' / 'config' / header_file, check: false)
endforeach

foreach name, value : symbol_configs
  header_file = name.to_lower() + '.h'
  run_command('ln', '-sfn', builddir / header_file, builddir / 'agar' / 'config' / header_file, check: false)
endforeach

# Set up include directories
# Build root contains agar/ with symlinks to source libs and config headers
config_inc = include_directories('.')

# ============================================================================
# Preprocess Include Files
# ============================================================================

# Note: The BSDBuild/CMake systems use Perl scripts to preprocess headers
# with gen-declspecs.pl. In Meson, we skip this preprocessing step as:
# 1. Headers work fine without preprocessing in most cases
# 2. Proper Meson integration would require custom_target() for each header
# 3. This keeps the build simple and maintainable
#
# Headers are installed directly from source via install_headers() in each
# library's meson.build file.

# ============================================================================
# Build Libraries
# ============================================================================

# Subdirectories for each library
subdir('core')

if get_option('gui')
  subdir('gui')
endif

if get_option('math')
  subdir('math')
endif

if get_option('net')
  subdir('net')
endif

if get_option('vg') and get_option('gui')
  subdir('vg')
endif

if get_option('sg') and get_option('gui') and get_option('math') and have_opengl
  subdir('sg')
endif

if get_option('sk') and get_option('gui') and get_option('math') and have_opengl
  subdir('sk')
endif

if get_option('au') and get_option('gui')
  subdir('au')
endif

if get_option('map') and get_option('gui')
  subdir('map')
endif

# ============================================================================
# Tests
# ============================================================================

# Only build tests when not used as a subproject
if not meson.is_subproject()
  if get_option('buildtype').startswith('debug') or get_option('tests')
    subdir('tests')
  endif
endif

# ============================================================================
# Summary
# ============================================================================

summary({
  'Version': agar_version,
  'Release': agar_release,
  'Memory Model': memory_model.to_upper(),
}, section: 'Project')

summary({
  'ag_au':   get_option('au') and get_option('gui'),
  'ag_gui':  get_option('gui'),
  'ag_map':  get_option('map') and get_option('gui'),
  'ag_math': get_option('math'),
  'ag_net':  get_option('net'),
  'ag_sg':   get_option('sg') and get_option('gui') and get_option('math') and have_opengl,
  'ag_sk':   get_option('sk') and get_option('gui') and get_option('math') and have_opengl,
  'ag_vg':   get_option('vg') and get_option('gui'),
}, section: 'Libraries')

summary({
  'FreeType':   get_option('gui') and get_option('freetype') and freetype_dep.found(),
  'Fontconfig': get_option('gui') and get_option('fontconfig') and fontconfig_dep.found(),
  'OpenGL':     get_option('gui') and have_opengl,
  'SDL 1.2':    get_option('gui') and get_option('sdl') and sdl_dep.found(),
  'SDL 2.0':    get_option('gui') and get_option('sdl2') and sdl2_dep.found(),
  'X11/GLX':    get_option('gui') and get_option('glx') and have_glx,
  'PNG':        get_option('gui') and get_option('png') and png_dep.found(),
  'JPEG':       get_option('gui') and get_option('jpeg') and jpeg_dep.found(),
  'Threads':    have_pthreads,
}, section: 'Dependencies')

