# Building Agar with Meson

This document describes how to build Agar using the Meson build system.

## Quick Start

```bash
# Configure with defaults
meson setup buildDir

# Compile
meson compile -C buildDir

# Install (sudo invoked automatically)
meson install -C buildDir
```

## Configuration

Meson uses a separate build directory (out-of-tree builds). To configure with custom options:

```bash
# View all available options
meson configure
meson configure buildDir -Dwarnings=true

# Configure with specific options
meson setup buildDir -Dgui=true -Dmath=true -Dthreads=enabled
```

## Build Options

### Core Features

- `gui` - Build Agar-GUI library (default: true)
- `math` - Build Agar-Math library (default: true)
- `net` - Build Agar-Net library (default: true)
- `vg` - Build Agar-VG library (default: true)
- `sg` - Build Agar-SG library (default: true)
- `sk` - Build Agar-SK library (default: true)
- `au` - Build Agar-AU library (default: true)
- `map` - Build Agar-MAP library (default: true)

### Memory Model

- `memory_model` - Select Agar memory model: auto, small, medium, large (default: auto)
  - `small` - For 8-bit systems with limited addressing (e.g., C64, NES)
  - `medium` - For 32-bit systems
  - `large` - For 64-bit systems (auto-detected on x86_64, aarch64, ppc64)

### Library Types

- `static` - Build static libraries (default: true)
- `shared` - Build shared libraries (default: true)
- `static_pic` - Build static libraries with PIC for shared linking (default: false)

### Feature Options

- `threads` - Enable thread safety (default: auto)
- `dso` - Enable dynamic shared objects (default: true)
- `event_loop` - Enable event loop (default: true)
- `timers` - Enable timers (default: true)
- `unicode` - Enable Unicode support (default: true)
- `widgets` - Enable widget system (default: true)
- `serialization` - Enable object serialization (default: true)
- `string` - Enable AG_String (default: true)
- `user` - Enable user/group support (default: true)
- `verbosity` - Enable verbose messages (default: true)
- `ansi_color` - Enable ANSI color in terminal (default: true)
- `legacy` - Enable legacy API support (default: false)
- `namespaces` - Enable namespace support (default: true)

### Inline Options

- `inline` - Master switch for all inline functions (default: true)
- `inline_byteswap` - Inline byte-swapping (default: true)
- `inline_error` - Inline error handling (default: true)
- `inline_event` - Inline event processing (default: true)
- `inline_io` - Inline I/O functions (default: true)
- `inline_object` - Inline object operations (default: true)
- `inline_string` - Inline string operations (default: true)
- `inline_surface` - Inline surface operations (default: true)
- `inline_widget` - Inline widget operations (default: true)

### Graphics Drivers

- `sdl` - Enable SDL 1.2 support (default: auto)
- `sdl2` - Enable SDL 2.0 support (default: auto)
- `gl` - Enable OpenGL support (default: auto)
- `glx` - Enable GLX (X11/OpenGL) driver (auto-detected)
- `x11` - Enable X11 support (default: auto)
- `xinerama` - Enable Xinerama multi-monitor (default: auto)
- `cocoa` - Enable macOS Cocoa driver (default: auto)
- `wgl` - Enable Windows WGL driver (auto-detected on Windows)

### Image Formats

- `freetype` - Enable FreeType font rendering (default: auto)
- `fontconfig` - Enable Fontconfig font selection (default: auto)
- `png` - Enable PNG image support (default: auto)
- `jpeg` - Enable JPEG image support (default: auto)

### Audio (Agar-AU)

- `sndfile` - Enable libsndfile support (default: auto)
- `portaudio` - Enable PortAudio support (default: auto)

### Database (Agar-Core)

- `db4` - Enable Berkeley DB support (default: false)
- `mysql` - Enable MySQL support (default: false)

### Networking (Agar-Net)

- `web` - Enable built-in HTTP server (default: false)

### Math Precision (Agar-Math)

- `math_precision` - Select floating-point precision: single, double, quad (default: double)

### SIMD Optimizations

- `sse` - Enable SSE optimizations (default: auto)
- `sse2` - Enable SSE2 instructions (default: false)
- `sse3` - Enable SSE3 instructions (default: false)
- `sse_inline` - Inline SSE functions (default: true)
- `altivec` - Enable AltiVec (PowerPC) optimizations (default: auto)
- `altivec_inline` - Inline AltiVec functions (default: true)

### Development Options

- `warnings` - Enable recommended compiler warnings (default: false)
- `type_safety` - Enable type safety checks (default: false)
- `tests` - Build test suite (default: false, auto-enabled in debug builds)

## Common Build Scenarios

### Minimal Build (Core only, no GUI)

```bash
meson setup buildDir \
  -Dgui=false \
  -Dmath=false \
  -Dnet=false
```

### Full Build (All libraries)

```bash
meson setup buildDir \
  -Dgui=true \
  -Dmath=true \
  -Dnet=true \
  -Dvg=true \
  -Dsg=true \
  -Dsk=true \
  -Dau=true \
  -Dmap=true
```

### Debug Build with Tests

```bash
meson setup buildDir \
  --buildtype=debug \
  -Dwarnings=true \
  -Dtype_safety=true \
  -Dtests=true
```

### Optimized Release Build

```bash
meson setup buildDir \
  --buildtype=release \
  -Db_lto=true \
  -Doptimization=3
```

### Static-Only Build

```bash
meson setup buildDir \
  -Ddefault_library=static \
  -Dshared=false \
  -Dstatic=true
```

### Cross-Compilation Example (Windows from Linux)

```bash
# Create cross-file: windows.txt
cat > windows.txt << EOF
[binaries]
c = 'x86_64-w64-mingw32-gcc'
cpp = 'x86_64-w64-mingw32-g++'
ar = 'x86_64-w64-mingw32-ar'
strip = 'x86_64-w64-mingw32-strip'
pkg-config = 'x86_64-w64-mingw32-pkg-config'

[host_machine]
system = 'windows'
cpu_family = 'x86_64'
cpu = 'x86_64'
endian = 'little'
EOF

meson setup buildDir --cross-file windows.txt
```

## Building and Installing

```bash
# Configure
meson setup buildDir

# Build (using all CPU cores)
meson compile -C buildDir

# Run tests
meson test -C buildDir

# Install to default prefix (/usr/local)
sudo meson install -C buildDir

# Install to custom prefix
meson setup buildDir --prefix=/opt/agar
meson compile -C buildDir
meson install -C buildDir
```

## Testing and Examples

LibAgar includes a comprehensive test suite (`agartest`) that serves both as an
interactive test application and a demonstration of library features. The test
suite is built automatically in debug builds or when explicitly enabled.

### Building the Test Suite

Tests are built conditionally:
- **Automatically** in debug builds (`--buildtype=debug`)
- **Manually enabled** with `-Dtests=true` option

```bash
# Option 1: Debug build (tests auto-enabled)
meson setup buildDir --buildtype=debug
meson compile -C buildDir

# Option 2: Release build with tests explicitly enabled
meson setup buildDir --buildtype=release -Dtests=true
meson compile -C buildDir

# Option 3: Add tests to existing build
meson configure buildDir -Dtests=true
meson compile -C buildDir
```

### Running the Test Suite

The `agartest` executable is an interactive GUI application that provides access to all test modules:

```bash
# Run the interactive test application
./buildDir/tests/agartest

# Run specific test(s) from command line
./buildDir/tests/agartest widgets
./buildDir/tests/agartest buttons checkbox fonts

# Run with specific graphics driver
./buildDir/tests/agartest -d sdl2gl      # SDL 2.0 + OpenGL
./buildDir/tests/agartest -d sdl2fb      # SDL 2.0 framebuffer
./buildDir/tests/agartest -d glx         # X11 + OpenGL (multi-window)

# Run with custom font
./buildDir/tests/agartest -t "Courier:12:b"
./buildDir/tests/agartest -t "DejaVu Sans:10"

# Enable debugging
./buildDir/tests/agartest -D             # Debug level 2
./buildDir/tests/agartest -C             # Console output to stderr

# Run tests as benchmarks
./buildDir/tests/agartest -b surface plotting math
```

### agartest Command-Line Options

| Option | Description |
|--------|-------------|
| `-b` | Run specified tests as benchmarks |
| `-C` | Redirect verbose/debug output to stderr (instead of GUI console) |
| `-D` | Enable object/variable debugging (debug level = 2) |
| `-W` | Force software timing wheel (instead of kernel APIs like select/kqueue) |
| `-q` | Disable verbose and debug messages |
| `-d <driver>` | Specify Agar graphics driver (sdl2gl, sdl2fb, glx, etc.) |
| `-s <file>` | Load custom stylesheet |
| `-t <spec>` | Set default font (format: `name:size[:flags]`) |
| `-v` | Print version and exit |

### Available Test Modules

The test suite includes the following interactive test modules:

**Core Widget Tests:**
- `buttons` - Button widgets and events
- `checkbox` - Checkbox and radio button widgets
- `widgets` - General widget demonstrations
- `customwidget` - Custom widget implementation example
- `radio` - Radio button groups
- `console` - Console widget with text output
- `textbox` - Text input and editing
- `textdlg` - Text entry dialogs
- `table` - Table/grid widget
- `scrollbar` - Scrollbar widget
- `scrollview` - Scrollable view container
- `pane` - Paned window widget

**Graphics and Rendering:**
- `fonts` - Font rendering and text display
- `charsets` - Character set support
- `compositing` - Compositing and alpha blending
- `fixedres` - Fixed resolution modes
- `glview` - OpenGL rendering widget (requires `-Dgl=auto/enabled`)
- `imageloading` - Image loading (PNG, JPEG, BMP)
- `palette` - Color palettes
- `rendertosurface` - Off-screen rendering
- `surface` - Surface operations

**Layout and UI:**
- `focusing` - Keyboard focus handling
- `keyevents` - Keyboard event processing
- `windows` - Window management
- `maximized` - Window maximization
- `minimal` - Minimal window example
- `fspaths` - File system path selection
- `loader` - Dynamic module loading

**System Integration:**
- `objsystem` - Object system demonstration
- `threads` - Multi-threading examples (requires `-Dthreads=true`)
- `timeouts` - Timer and timeout handling
- `user` - User/group information
- `configsettings` - Configuration settings

**Math and Science (requires `-Dmath=true`):**
- `math` - Mathematical functions
- `plotting` - 2D plotting and graphing
- `bezier` - Bézier curve rendering
- `string` - String formatting
- `unitconv` - Unit conversion

**Networking (requires `-Dnet=true`):**
- `network` - Network functionality
- `sockets` - Socket operations

**Audio (requires `-Dau=true`):**
- `audio` - Audio playback and recording

### Standalone Examples

The `tests/` directory also contains standalone example programs that use BSDBuild:

```bash
# agarhello - Simple "Hello World" GUI application
cd tests/agarhello
./configure
make
./agarhello

# agarevloop - Event loop integration example
cd tests/agarevloop
./configure
make
./agarevloop

# agarsdl - SDL integration example
cd tests/agarsdl
./configure
make
./agarsdl

# agarreinit - Reinitialization example
cd tests/agarreinit
./configure
make
./agarreinit

# agarcplusplus - C++ integration example
cd tests/agarcplusplus
./configure
make
./agarcplusplus
```

**Note:** These standalone examples currently only support BSDBuild. Meson build files for these examples may be added in future releases.

### Running Meson Test Framework

If the test suite has registered Meson tests (future enhancement. None yet.):

```bash
# Run all registered tests
meson test -C build

# Run with verbose output
meson test -C build -v

# Run specific test
meson test -C build testname

# Run tests in parallel
meson test -C build -j 4
```

### Test Data Files

The test suite includes data files (images, text, etc.) installed to `<prefix>/share/agartest/`:
- Sample images: `agar.png`, `parrot.png`, `menubg.bmp`, `pepe.jpg`
- Icons: `axe.png`, `helmet.png`, `sword.bmp`
- Test data: `loss.txt` (text file)
- Font samples: `champden.png`, `mamismoke.png`

### Common Test Scenarios

**Test all widgets with OpenGL:**
```bash
./buildDir/tests/agartest -d glx widgets
```

**Benchmark surface operations:**
```bash
./buildDir/tests/agartest -b surface
```

**Test font rendering with custom font:**
```bash
./buildDir/tests/agartest -t "Courier:14" fonts charsets
```

**Test with SDL2 framebuffer (no OpenGL):**
```bash
./build/tests/agartest -d sdl2fb
```

**Debug mode with console output:**
```bash
./build/tests/agartest -D -C
```

## Inspecting the Build

```bash
# View current configuration
meson configure build

# Show build information
meson introspect build --buildoptions
meson introspect build --dependencies
meson introspect build --targets

# Show summary
cat build/meson-logs/meson-log.txt
```

## Cleaning

```bash
# Clean build artifacts (keeps configuration)
ninja -C build clean

# Or completely remove build directory
rm -rf build
```

## Configuration Header Generation

### Overview

LibAgar's source code expects individual configuration headers for each feature (e.g., `have_opengl.h`, `version.h`) in the `include/agar/config/` directory. The Meson build system generates these headers using `configure_file()` with custom templates.

### How It Works

Meson generates ~70 individual headers at configure time:

1. **Boolean features** (e.g., `HAVE_OPENGL`):
   - Enabled: `#ifndef HAVE_OPENGL\n#define HAVE_OPENGL "yes"\n#endif`
   - Disabled: `#undef HAVE_OPENGL`

2. **String values** (e.g., `VERSION`):
   - `#ifndef VERSION\n#define VERSION "1.7.1"\n#endif`

### Templates

Four templates in `meson/templates/` define the header formats:

- **config_bool_yes.h.in** - For enabled boolean features (defines as `"yes"`)
- **config_bool_no.h.in** - For disabled boolean features (`#undef` directive)
- **config_string.h.in** - For string configuration values (e.g., `VERSION`, `RELEASE`)
- **config_symbol.h.in** - For symbolic constants without quotes (e.g., `AG_MODEL` → `AG_LARGE`)

Example of each template:

**config_bool_yes.h.in:**
```c
#ifndef @MACRO@
#define @MACRO@ "yes"
#endif
```

**config_bool_no.h.in:**
```c
#undef @MACRO@
```

**config_string.h.in:**
```c
#ifndef @MACRO@
#define @MACRO@ "@VALUE@"
#endif
```

**config_symbol.h.in:**
```c
#ifndef @MACRO@
#define @MACRO@ @VALUE@
#endif
```

See the [Meson configure_file() documentation](https://mesonbuild.com/Reference-manual_functions.html#configure_file) for details.

### Differences from Other Build Systems

| Build System | Config Header Approach | When Generated |
|--------------|------------------------|----------------|
| **BSDBuild** | Shell scripts during `./configure` create individual headers using `hdefine()` | Configure time |
| **CMake** | CMakeChecks.cmake uses `BB_Save_Define()` to generate individual headers | Configure time |
| **Meson** | Uses `configure_file()` with templates, generates headers programmatically | Configure time |

### Key Differences in Meson Approach

1. **Declarative**: Configuration is defined in dictionaries rather than imperative scripts
2. **Template-based**: Uses reusable templates instead of per-feature generation logic
3. **Parallel generation**: All headers generated concurrently for faster configuration
4. **Proper dependency tracking**: Headers regenerate automatically when configuration changes
5. **Type-safe**: Meson's type system prevents configuration errors at build-file parse time

### Adding New Configuration Headers

To add a new config header:

1. Add the variable to `meson.build`:
   ```meson
   bool_configs = {
     'HAVE_NEW_FEATURE': have_new_feature,
     # ... existing configs
   }
   ```

2. Meson will automatically generate `have_new_feature.h` using the appropriate template

3. The header will be installed to `include/agar/config/`

For more information, see the [Meson Configuration documentation](https://mesonbuild.com/Configuration.html).

## Known Issues and Fixes

During the implementation of the Meson build system, several compatibility issues were identified and resolved. These fixes are backward-compatible with BSDBuild and CMake builds.

## Comparison with BSDBuild and CMake

### BSDBuild (Autoconf-style)

```bash
./configure --prefix=/usr/local --enable-debug
make depend all
sudo make install
```

### CMake

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
sudo cmake --install build
```

### Meson (This)

```bash
meson setup build --buildtype=debug
meson compile -C build
sudo meson install -C build
```

## Environment Variables

Meson respects standard environment variables:

- `CC` - C compiler
- `CXX` - C++ compiler
- `CFLAGS` - C compiler flags
- `LDFLAGS` - Linker flags
- `PKG_CONFIG_PATH` - pkg-config search path

Example:

```bash
CC=clang meson setup build
```

## pkg-config Integration

After installation, use pkg-config to find Agar:

```bash
# Get compile flags
pkg-config --cflags agar

# Get linker flags
pkg-config --libs agar

# For specific libraries
pkg-config --cflags --libs agar-math agar-gui
```

## Troubleshooting

### Dependencies Not Found

```bash
# Check available dependencies
pkg-config --list-all | grep -E '(sdl|freetype|opengl)'

# Set PKG_CONFIG_PATH if needed
export PKG_CONFIG_PATH=/opt/local/lib/pkgconfig:$PKG_CONFIG_PATH
meson setup build --wipe
```

### Reconfiguring After Changes

```bash
# Wipe and reconfigure
meson setup build --wipe

# Or remove and recreate
rm -rf build
meson setup build
```

### Build Failures

```bash
# View detailed logs
cat build/meson-logs/meson-log.txt

# Verbose compilation
meson compile -C build -v
```

### Verifying Build Success

After a successful build, verify that all libraries were built:

```bash
# Check shared libraries
find build -name "libag_*.so.*" -type f

# Expected output:
# build/core/libag_core.so.8.0.0
# build/gui/libag_gui.so.8.0.0
# build/math/libag_math.so.8.0.0
# build/net/libag_net.so.8.0.0
# build/vg/libag_vg.so.8.0.0
# build/sg/libag_sg.so.8.0.0
# build/sk/libag_sk.so.8.0.0

# Check static libraries
find build -name "libag_*.a" -type f

# Verify config headers were generated
ls build/agar/config/*.h | wc -l
# Should show ~110 config headers
```

### Config Header Generation Issues

If config headers are missing or incorrect:

```bash
# Check template files exist
ls -la meson/templates/

# Should show:
# config_bool_yes.h.in
# config_bool_no.h.in
# config_string.h.in
# config_symbol.h.in

# Regenerate build configuration
rm -rf build
meson setup build
```

## Additional Resources

- Meson documentation: https://mesonbuild.com/
- Agar documentation: https://libagar.org/
- BSDBuild (primary build system): See configure.in and README
- CMake build system: See CMakeLists.txt and CMAKE.md

