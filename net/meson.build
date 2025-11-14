# Agar-Net Library
# Network interface library with optional HTTP/1.1 application server

# Base sources
net_sources = files(
  'net.c',
  'net_dummy.c',
)

# Conditional sources based on platform and features
if have_getaddrinfo
  net_sources += files('net_bsd.c')
endif

if host_system == 'windows'
  # Windows socket support
  if cc.has_header('winsock2.h')
    net_sources += files('net_winsock2.c')
  elif cc.has_header('winsock.h')
    net_sources += files('net_winsock1.c')
  endif
endif

# HTTP application server support
if get_option('web')
  net_sources += files(
    'web.c',
    'web_auth.c',
    'web_var.c',
  )
endif

# Network library dependencies
net_deps = [
  libag_core_dep,
  math_dep,
]

if get_option('net') and get_option('web') and get_option('zlib') and zlib_dep.found()
  net_deps += zlib_dep
endif

if host_system == 'windows'
  # Add ws2_32 library for Windows sockets
  net_deps += cc.find_library('ws2_32', required: false)
endif

# Compile flags
net_c_args = global_defs + warning_flags + [
  '-D_AGAR_NET_INTERNAL',
]

# Include directories
# Include local headers and config_inc for generated config headers
net_inc = [include_directories('.', '../core'), config_inc]

# Build library (respects parent project's default_library option)
libag_net = library('ag_net',
  net_sources,
  c_args: net_c_args,
  include_directories: net_inc,
  dependencies: net_deps,
  version: agar_soversion,
  soversion: agar_soversion_major,
  install: true,
)

# Declare dependency
libag_net_dep = declare_dependency(
  link_with: libag_net,
  include_directories: net_inc,
  dependencies: net_deps,
)

# Override dependency for wrap support
meson.override_dependency('agar-net', libag_net_dep)

# Install headers
install_headers(
  'begin.h',
  'close.h',
  'net.h',
  'net_pub.h',
  'nullability.h',
  'web.h',
  subdir: 'agar/net',
)
install_symlink(
  'net.h',
  install_dir : 'include/agar',
  pointing_to: 'net/net_pub.h')

# Generate pkg-config file
pkg.generate(
  libag_net,
  name: 'agar-net',
  description: 'Agar network interface',
  version: agar_version,
  subdirs: 'agar',
  requires: ['agar-core'],
)

# Install manpages
net_man3 = files(
  'AG_Net.3',
  'AG_Web.3',
)
install_man(net_man3)
