# Agar-AU Library
# Audio and sound toolkit (requires GUI and threads)

# Base sources
au_sources = files(
  'au.c',
  'au_wave.c',
)

# Conditional sources based on features
if have_pthreads
  if get_option('sndfile') and sndfile_dep.found()
    au_sources += files('au_dev_file.c')
  endif

  if portaudio_dep.found()
    au_sources += files('au_dev_pa.c')
  endif
endif

au_deps = [
  libag_core_dep,
  libag_gui_dep,
  math_dep,
]

if get_option('sndfile') and sndfile_dep.found()
  au_deps += sndfile_dep
endif

if portaudio_dep.found()
  au_deps += portaudio_dep
endif

if have_pthreads
  au_deps += thread_dep
endif

au_c_args = global_defs + warning_flags + ['-D_AGAR_AU_INTERNAL']
# Include local headers and config_inc for generated config headers
au_inc = [include_directories('.', '../core', '../gui'), config_inc]

if get_option('static')
  static_pic = get_option('static_pic') and get_option('shared')

  libag_au_static = static_library('ag_au',
    au_sources,
    c_args: au_c_args,
    include_directories: au_inc,
    dependencies: au_deps,
    install: true,
    pic: static_pic,
  )
endif

if get_option('shared')
  libag_au_shared = shared_library('ag_au',
    au_sources,
    c_args: au_c_args,
    include_directories: au_inc,
    dependencies: au_deps,
    version: agar_soversion,
    soversion: agar_soversion_major,
    install: true,
  )
  libag_au = libag_au_shared
else
  libag_au = libag_au_static
endif

libag_au_dep = declare_dependency(
  link_with: libag_au,
  include_directories: au_inc,
  dependencies: au_deps,
)

install_headers(
  'au_dev_out.h',
  'au_init.h',
  'au_math.h',
  'au_pub.h',
  'au_wave.h',
  'begin.h',
  'close.h',
  subdir: 'agar/au',
)
install_symlink(
  'au.h',
  install_dir : 'include/agar',
  pointing_to: 'au/au_pub.h')

pkg.generate(
  libag_au,
  name: 'agar-au',
  description: 'Agar audio library',
  version: agar_version,
  subdirs: 'agar',
  requires: ['agar'],
)

# Install manpages
au_man3 = files(
  'AU.3',
  'AU_DevOut.3',
  'AU_Wave.3',
)
install_man(au_man3)
