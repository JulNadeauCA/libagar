# Agar-Core Library
# Core object system and utility library (platform-independent)

# Base source files (always compiled)
core_sources = files(
  'byteswap.c',
  'config.c',
  'core.c',
  'cpuinfo.c',
  'crc32.c',
  'data_source.c',
  'db.c',
  'dir.c',
  'dso.c',
  'error.c',
  'event.c',
  'exec.c',
  'file.c',
  'getopt.c',
  'load_integral.c',
  'load_real.c',
  'load_string.c',
  'load_version.c',
  'object.c',
  'string.c',
  'tbl.c',
  'text.c',
  'time.c',
  'time_dummy.c',
  'timeout.c',
  'threads.c',
  'vasprintf.c',
  'vsnprintf.c',
  'user.c',
  'user_dummy.c',
  'user_getenv.c',
  'variable.c',
  'vec.c',
)

# Conditional source files based on detected features
if not have_snprintf
  core_sources += files('snprintf.c')
endif

if host_system == 'windows'
  core_sources += files('time_win32.c')
endif

if have_clock_gettime
  if have_nanosleep
    core_sources += files('time_posix.c')
  endif
  if have_pthreads
    core_sources += files('time_renderer.c')
  endif
endif

if have_gettimeofday and have_select
  core_sources += files('time_gettimeofday.c')
endif

if have_getpwuid
  core_sources += files('user_posix.c')
endif

if host_system == 'windows'
  # CSIDL support for Windows
  core_sources += files('user_win32.c')
endif

if get_option('db4')
  core_sources += files('db_bdb.c')
endif

if get_option('mysql')
  core_sources += files('db_mysql.c')
endif

# Core library dependencies
core_deps = [math_dep]

if have_dlopen
  core_deps += libdl
endif

if have_shl_load and libdld.found()
  core_deps += libdld
endif

if have_clock_gettime and librt.found()
  core_deps += librt
endif

if have_pthreads
  core_deps += thread_dep
endif

if get_option('db4') and db4_dep.found()
  core_deps += db4_dep
endif

if get_option('mysql') and mysql_dep.found()
  core_deps += mysql_dep
endif

if get_option('iconv') and iconv_dep.found()
  core_deps += iconv_dep
endif

if get_option('nls')
  if gettext_dep.found()
    core_deps += gettext_dep
  endif
  if intl_dep.found()
    core_deps += intl_dep
  endif
endif

# Compile flags
core_c_args = global_defs + warning_flags + [
  '-D_AGAR_CORE_INTERNAL',
]

# Include directories
# Include '.' for local headers and config_inc for generated config headers
core_inc = [include_directories('.'), config_inc]

# Version script for symbol visibility (optional)
core_link_args = []
if host_system == 'linux'
  # Could add version script here if needed
  core_link_args = []
endif

# Build library (respects parent project's default_library option)
libag_core = library('ag_core',
  core_sources,
  c_args: core_c_args,
  include_directories: core_inc,
  dependencies: core_deps,
  version: agar_soversion,
  soversion: agar_soversion_major,
  install: true,
  link_args: core_link_args,
)

# Declare dependency for other libraries to use
libag_core_dep = declare_dependency(
  link_with: libag_core,
  include_directories: core_inc,
  dependencies: core_deps,
)

# Override dependency for wrap support
meson.override_dependency('agar-core', libag_core_dep)

# Install headers
install_headers(
  'agsi.h',
  'ag_limits.h',
  'ag_string.h',
  'agtime.h',
  'attributes.h',
  'begin.h',
  'btree.h',
  'byteswap.h',
  'classes.h',
  'close.h',
  'close_attributes.h',
  'close_types.h',
  'config.h',
  'core.h',
  'core_init.h',
  'core_pub.h',
  'cpuinfo.h',
  'crc32.h',
  'data_source.h',
  'db.h',
  'dir.h',
  'dso.h',
  'error.h',
  'event.h',
  'exec.h',
  'file.h',
  'getopt.h',
  'inline_byteswap.h',
  'inline_error.h',
  'inline_event.h',
  'inline_load_integral.h',
  'inline_load_real.h',
  'inline_object.h',
  'inline_string.h',
  'inline_tbl.h',
  'inline_threads.h',
  'inline_variable.h',
  'load_integral.h',
  'load_real.h',
  'load_string.h',
  'load_version.h',
  'nullability.h',
  'object.h',
  'options.h',
  'queue.h',
  'queue_close.h',
  'snprintf.h',
  'string_strcasecmp.h',
  'tbl.h',
  'text.h',
  'threads.h',
  'threads_nullability.h',
  'types.h',
  'user.h',
  'variable.h',
  'vasprintf.h',
  'vec.h',
  'version.h',
  'vsnprintf.h',
  'win32.h',
  'xbox.h',
  subdir: 'agar/core',
)
install_symlink(
  'core.h',
  install_dir : 'include/agar',
  pointing_to: 'core/core_pub.h')

# Generate pkg-config file
pkg = import('pkgconfig')
pkg.generate(
  libag_core,
  name: 'agar-core',
  description: 'Agar object system and utility library',
  version: agar_version,
  subdirs: 'agar',
)

# Install manpages
core_man3 = files(
  'AG_ByteSwap.3',
  'AG_CPUInfo.3',
  'AG_Config.3',
  'AG_Core.3',
  'AG_DSO.3',
  'AG_DataSource.3',
  'AG_Db.3',
  'AG_Error.3',
  'AG_Event.3',
  'AG_EventLoop.3',
  'AG_Execute.3',
  'AG_File.3',
  'AG_Getopt.3',
  'AG_Intro.3',
  'AG_Limits.3',
  'AG_Object.3',
  'AG_Queue.3',
  'AG_String.3',
  'AG_Tbl.3',
  'AG_TextElement.3',
  'AG_Threads.3',
  'AG_Time.3',
  'AG_Timer.3',
  'AG_User.3',
  'AG_Variable.3',
  'AG_Version.3',
)
install_man(core_man3)
