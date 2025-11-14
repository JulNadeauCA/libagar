# Agar Test Suite
# Interactive test and demo application

# Base test sources
test_sources = files(
  'agartest.c',
  'buttons.c',
  'charsets.c',
  'checkbox.c',
  'compositing.c',
  'configsettings.c',
  'console.c',
  'customwidget.c',
  'customwidget_mywidget.c',
  'fixedres.c',
  'focusing.c',
  'fonts.c',
  'fspaths.c',
  'glview.c',
  'imageloading.c',
  'keyevents.c',
  'loader.c',
  'maximized.c',
  'minimal.c',
  'network.c',
  'objsystem.c',
  'objsystem_animal.c',
  'objsystem_mammal.c',
  'pane.c',
  'palette.c',
  'radio.c',
  'rendertosurface.c',
  'scrollbar.c',
  'scrollview.c',
  'sockets.c',
  'surface.c',
  'table.c',
  'textbox.c',
  'textdlg.c',
  'threads.c',
  'timeouts.c',
  'unitconv.c',
  'user.c',
  'widgets.c',
  'windows.c',
)

# Conditional sources - Agar-Math tests
if get_option('math')
  test_sources += files(
    'bezier.c',
    'bezier_widget.c',
    'math.c',
    'plotting.c',
    'string.c',
  )
endif

# Conditional sources - Agar-AU tests
if get_option('au')
  test_sources += files('audio.c')
endif

# Dependencies
test_deps = [
  libag_core_dep,
  libag_gui_dep,
]

if get_option('math')
  test_deps += libag_math_dep
endif

if get_option('au')
  test_deps += libag_au_dep
endif

# Compile arguments
test_c_args = global_defs + [
  '-D_USE_AGAR_STD',
  '-D_USE_AGAR_QUEUE',
  '-D_USE_AGAR_TYPES',
  '-D_USE_AGAR_ATTRIBUTES',
  '-D_USE_AGAR_NULLABILITY',
]

# Build the agartest executable
agartest = executable('agartest',
  test_sources,
  c_args: test_c_args,
  dependencies: test_deps,
  include_directories: config_inc,  # Need build root for <agar/...> includes
  install: true,
)

# Data files for tests
test_data_files = [
  'agar.png',
  'agar64.png',
  'agar-index.png',
  'agar.bmp',
  'agar-1.bmp',
  'agar-2.bmp',
  'agar-3.bmp',
  'agar-4.bmp',
  'axe.bmp',
  'axe.png',
  'helmet.png',
  'helmet.bmp',
  'helmet-socket.bmp',
  'loss.txt',
  'menubg.bmp',
  'pepe.jpg',
  'sword.bmp',
  'sword-socket.bmp',
  'sq-agar.bmp',
  'sq-agar.png',
  'champden.png',
  'parrot.png',
  'parrot8.png',
  'mamismoke.png',
]

# Install data files
install_data(test_data_files,
  install_dir: get_option('datadir') / 'agartest',
)

# Install source files as data (for examples)
install_data(test_sources,
  install_dir: get_option('datadir') / 'agartest',
)

# Install man page
install_man('agartest.1')

