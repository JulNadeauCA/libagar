# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

LibAgar is a cross-platform GUI toolkit and application framework written in C. It provides a modular architecture with multiple libraries:

- **CORE**: Platform abstraction, I/O, object system (non-graphical)
- **GUI**: Base framework and standard widgets
- **MATH**: Matrices, vectors, advanced rendering methods
- **NET**: Network interface, HTTP application server
- **VG**: Vector graphics library
- **AU**: Audio interface library
- **SG**: General-purpose 3D engine
- **SK**: Sketches with constraints
- **MAP**: Tile engine

## Build System

LibAgar supports three build systems. See [MESON.md](MESON.md) for detailed Meson documentation.

### 1. BSDBuild
The traditional build system using Autoconf-style configuration:

```bash
./configure --help                    # View all options
./configure --enable-debug            # Debug build with type-safety checks
./configure --prefix=$HOME            # Custom install location
make depend all                       # Build with dependencies
make install                          # Install (may need sudo)
```

Common configure options:
- `--enable-debug`: Enables AG_DEBUG, type-safety, and GUI debugger
- `--enable-warnings`: Enable suggested compiler warnings
- `--disable-threads`: Disable multi-thread support
- `--disable-{au,map,web}`: Disable specific libraries
- `--with-freetype[=PREFIX]`: FreeType support (required for GUI)
- `--with-sdl2[=PREFIX]`: SDL 2.0 driver support
- `--with-gl[=PREFIX]`: OpenGL rendering support

### 2. CMake
Cross-platform build system with IDE integration:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build
```

### 3. Meson
Fast, user-friendly build system with excellent dependency management:

```bash
# Quick start
meson setup buildDir                     # Configure with auto-detected dependencies
meson compile -C buildDir                # Build (parallel by default)
meson install -C buildDir                # Install (may need sudo)

# Common options
meson setup buildDir \
  --buildtype=debug \                 # debug, release, debugoptimized
  -Dgui=true \                        # Enable Agar-GUI
  -Dmath=true \                       # Enable Agar-Math
  -Dthreads=enabled                   # Enable thread support

# View all options
meson configure buildDir

# See MESON.md for complete documentation
```

**Key advantages of Meson:**
- Fast configuration and compilation (parallel by default)
- Out-of-tree builds (never modifies source directory)
- Better dependency detection and error messages
- Native support for modern build tools (ninja, IDE integration)
- Comprehensive test framework integration
- See [MESON.md](MESON.md) for ~70 configuration options

### 4. Makefile (Direct)
Direct makefile invocation (requires prior configuration):

```bash
make depend all
make install
```

## Build System Comparison

| Feature | BSDBuild | CMake | Meson |
|---------|----------|-------|-------|
| Configuration Style | Autoconf-like (shell scripts) | Imperative (CMake language) | Declarative (Python-like) |
| Build Speed | Fast ("make -j" supported; out-of-tree builds are fastest) | Fast | Fastest (parallel by default) |
| Config Headers | Shell script generation (~110 files) | BB_Save_Define macros | Template-based (4 templates) |
| Out-of-tree Builds | Optional | Yes | Always (required) |
| Dependency Detection | Manual scripting | FindPackage modules | pkg-config + automatic fallbacks |
| IDE Integration | Fair (project file generation via premake) | Excellent (generates project files) | Good (compile_commands.json) |
| Cross-compilation | Manual configuration | Toolchain files | Built-in support |
| Cross-compilation to 8-Bit Systems | Yes | No | No |
| BSD-style "make depend" target | Yes | No | No |
| Incremental Builds | Make-based | Make or Ninja | Ninja (default) |
| Learning Curve | Moderate (documented in manual pages) | Moderate | Low (simple syntax) |
| Maturity in LibAgar | Primary/Legacy (well-tested) | Stable | New (modern approach) |

## Directory Structure

### Source Libraries
- `core/`: Non-graphical core and utility library (object system, platform, I/O)
- `gui/`: The Agar GUI (graphics, window system, GUI framework and standard widget library)
- `math/`: The Agar Math library (vectors, matrices, geometry and advanced rendering methods)
- `net/`: Network library (network routines, HTTP application server)
- `vg/`: Vector graphics library
- `au/`: Audio interface and extensions library
- `sg/`: General-purpose 3D engine with visualization widget
- `sk/`: Dimensioned 2D sketches with constraint-solving and visualization widget
- `map/`: 2D tile engine

### Build System Files
- `configure`, `configure.in`: BSDBuild configuration script and source
- `CMakeLists.txt`, `CMakeChecks.cmake`: CMake build system
- `meson.build`, `meson_options.txt`: Meson build system (root level)
- `*/meson.build`: Per-library Meson build definitions
- `meson/`: Meson build system support files
  - `meson/templates/`: Config header templates (4 files)
    - `config_bool_yes.h.in`: Enabled boolean features
    - `config_bool_no.h.in`: Disabled boolean features
    - `config_string.h.in`: String configuration values
    - `config_symbol.h.in`: Symbolic constants (e.g., AG_MODEL)
  - `meson/create_config_symlinks.py`: Helper script (legacy, not used in current build)

### Build Artifacts (Generated)
- `include/`: Generated header files (BSDBuild/CMake builds)
- `buildDir/` or `build/`: Meson build output directory (out-of-tree)
  - `build/agar/`: Symlinks to source directories + generated config headers
  - `build/agar/config/`: ~110 individual config headers
  - `build/*/libag_*.so.*`: Built shared libraries
  - `build/*/libag_*.a`: Built static libraries

### Development & Testing
- `tests/`: Test suite and examples (includes `agartest` program)
- `mk/`: Build system infrastructure (BSDBuild)
- `tools/`: Build tools and utilities
- `ada/`: Ada language bindings
- `p5-Agar/`: Perl language bindings

### Documentation
- `MESON.md`: Meson build system documentation
- `CLAUDE.md`: This file (AI assistant guidance)
- `README.md`, `INSTALL.md`, `CHANGELOG.md`: General documentation

## Architecture

### Object System (AG_Object)

LibAgar uses an object-oriented design in C with a hierarchical class system:

- All GUI elements inherit from `AG_Object`
- Classes are registered at runtime via `AG_RegisterClass()`
- Objects support serialization, events, timers, and variables
- See `core/object.h` and `core/object.c`

### Event System (AG_Event)

- Event-driven architecture with named events
- Events can have typed arguments (accessed via `AG_EVENT_ARGS()`)
- Events support both named and positional arguments (when `AG_NAMED_ARGS` is defined)
- See `core/event.h` and `core/event.c`

### Widget System (AG_Widget)

All widgets inherit from `AG_Widget` (which inherits from `AG_Object`):

- Widgets have a rendering context (OpenGL or framebuffer)
- Each widget has size requisition, allocation, and drawing methods
- Widgets are contained in windows (`AG_Window`)
- Style attributes are managed via `AG_StyleSheet`

Key widget base files:
- `gui/widget.c`, `gui/widget.h`: Base widget implementation
- `gui/window.c`, `gui/window.h`: Window management
- `gui/primitive.c`, `gui/primitive.h`: Drawing primitives

### Driver System (AG_Driver)

LibAgar abstracts graphics backends through drivers:

- **Multi-window drivers**: `glx` (X11+OpenGL), `wgl` (Windows+OpenGL), `cocoa` (macOS+OpenGL), `sdl2mw` (SDL2 multi-window)
- **Single-window drivers**: `sdlfb`, `sdlgl`, `sdl2fb`, `sdl2gl`
- Drivers are in `gui/drv_*.c` files

### Memory Models

LibAgar supports three memory models (configure via `--with-memory-model`):
- **SMALL**: For embedded/retro systems (BBC, C64, NES)
- **MEDIUM**: Default for 32-bit systems
- **LARGE**: For 64-bit systems

### Debug vs Release Builds

When `--enable-debug` is used:
- `AG_DEBUG` is defined
- Type-safety checks are performed at runtime (`AG_TYPE_SAFETY`)
- Object validity checks on API calls
- GUI debugger is available (`AG_GuiDebugger`)
- Use `AG_Verbose()`, `AG_Debug()` for debug output

## Code Conventions

### Inline Functions

LibAgar extensively uses inline functions for performance. Controlled by configure flags:
- `--with-inline-byteswap`: Inline endianness swaps
- `--with-inline-error`: Inline malloc() wrappers
- `--with-inline-event`: Inline event argument accessors
- `--with-inline-io`: Inline serialization frontends
- `--with-inline-object`: Inline AG_Object operations
- `--with-inline-surface`: Inline pixel access routines
- `--with-inline-widget`: Inline AG_Widget functions

Inline implementations are in `inline_*.h` files within each library directory.

### Naming Conventions

- Public API functions: `AG_FunctionName()` (PascalCase with AG_ prefix)
- Private/internal functions: lowercase with AG_ prefix or no prefix
- Object methods: `AG_ObjectName` + method name (e.g., `AG_ButtonText()`)
- Macros: ALL_CAPS with `AG_` prefix
- Types: `AG_TypeName` (PascalCase)

### Header Files

LibAgar uses a sophisticated header system with several key components:

**Config Headers:**
- Generated during build (~110 individual header files)
- Location varies by build system:
  - BSDBuild/CMake: `include/agar/config/`
  - Meson: `buildDir/agar/config/` (symlinked)
- Four types of config headers (Meson):
  - Boolean enabled: `#define MACRO "yes"`
  - Boolean disabled: `#undef MACRO`
  - String values: `#define MACRO "value"`
  - Symbolic constants: `#define MACRO value` (no quotes)

**Public API Headers:**
- `*_pub.h`: Public subset included by main header
- `inline_*.h`: Inline function implementations
- `begin.h`, `close.h`: Compiler attribute management (must be paired)

**Header Include Path (Meson builds):**
- `-I../` includes project root
- `-I../core`, `-I../gui`, etc. for each library
- `-Ibuild/agar` for symlinked structure + config headers
- Order matters: config headers must be found before source headers

**Forward Declarations:**
- Used to avoid circular dependencies (e.g., `struct ag_fmt_string;`)
- Allows pointers to incomplete types
- Common pattern: use `void *` with type comments when full type causes circular dependency

### Thread Safety

When built with `--enable-threads`:
- Most public API calls are reentrant
- Objects have built-in locking via `AG_ObjectLock()` / `AG_ObjectUnlock()`
- Event handlers run in a locked context
- See `core/threads.h` for thread primitives

## Source Compatibility

### Internal vs External API

LibAgar provides two API styles controlled by preprocessor macros:

**External API (default for applications):**
- Use explicit `AG_` prefixed functions: `AG_Malloc()`, `AG_Strlcpy()`, `AG_Strdup()`
- Safe for applications linking against LibAgar

**Internal API (for LibAgar source code):**
- Define `_AGAR_INTERNAL` or `_USE_AGAR_STD` to enable shorthand macros
- Allows using `Malloc()`, `Strlcpy()`, `Strdup()` without `AG_` prefix
- Defined in wrapper headers (e.g., `core/ag_string.h`)
- **Warning:** Only use these macros within LibAgar source files

Example from `core/ag_string.h`:
```c
#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_STD)
# define Strlcat(dst,src,dsize)  AG_Strlcat((dst),(src),(dsize))
# define Strlcpy(dst,src,dsize)  AG_Strlcpy((dst),(src),(dsize))
# define Strdup(s)               AG_Strdup(s)
# define TryStrdup(s)            AG_TryStrdup(s)
#endif
```

### Platform Compatibility

**Global Defines (set by build system):**
- `_DARWIN_C_SOURCE`: macOS compatibility
- `_NETBSD_SOURCE`: NetBSD compatibility
- Set globally in build configuration, not per-file

**Conditional Compilation:**
- Use `HAVE_*` defines to check for features (e.g., `HAVE_OPENGL`, `HAVE_FREETYPE`)
- Check memory model with `AG_MODEL` (AG_SMALL, AG_MEDIUM, AG_LARGE)
- Platform detection via standard defines: `__WIN32__`, `__APPLE__`, `__linux__`

### Memory Management

**Always use LibAgar's allocators:**
- `AG_Malloc()`, `AG_Realloc()`, `AG_Free()` instead of standard malloc/realloc/free
- `AG_TryMalloc()`, `AG_TryRealloc()` for non-fatal allocation attempts
- Internal code can use `Malloc()`, `Realloc()`, `Free()` when `_AGAR_INTERNAL` is defined
- Reason: Allows LibAgar to track allocations and provide debugging support

### String Functions

**Use LibAgar's safe string functions:**
- `AG_Strlcpy()`, `AG_Strlcat()`: Safe string copying (always NUL-terminates)
- `AG_Strdup()`, `AG_TryStrdup()`: Safe string duplication
- `AG_Strcasecmp()`, `AG_Strncasecmp()`: Case-insensitive comparison
- Available as shorthand (`Strlcpy`, etc.) when `_AGAR_INTERNAL` is defined

## Testing

The `tests/` directory contains the test suite:

```bash
cd tests
./configure
make depend all
./agartest                 # Run the interactive test suite
```

Individual test programs can be found in subdirectories like `tests/agarhello/`.

## Common Development Tasks

### Adding a New Widget

1. Create `gui/my_widget.c` and `gui/my_widget.h`
2. Define the widget structure inheriting from `AG_Widget`
3. Implement class methods: `Init`, `Draw`, `SizeRequest`, `SizeAllocate`
4. Register the class: `AG_RegisterClass(&myWidgetClass)`
5. Add to `gui/Makefile` in `SRCS` variable
6. Add man page: `gui/AG_MyWidget.3`

### Adding a Configure Option

1. Edit `configure.in`
2. Add `register()` call for the option
3. Add `check()` or conditional logic
4. Add corresponding `hdefine()` or `mdefine()` calls
5. Regenerate configure: `mkconfigure` (requires BSDBuild)

### Platform-Specific Code

Platform detection is done in `configure.in`:
- `${host}`: Target platform (e.g., `x86_64-linux-gnu`)
- Conditional compilation via `#ifdef HAVE_FEATURE` in C code
- Platform-specific sources added via `mappend()` in configure.in

### Working with AG_Surface

`AG_Surface` represents a software or hardware-accelerated pixel buffer:
- Supports various pixel formats (indexed, RGB, RGBA)
- Can load from BMP, PNG, JPEG (if libraries available)
- Blitting operations in `gui/surface.c`
- Hardware acceleration when OpenGL is available

## Important Notes

- The `include/` directory is generated during build - do not edit files there directly
- Configuration results are in `Makefile.config` and `include/agar/config/`
- LibAgar defines global symbols like `_DARWIN_C_SOURCE` and `_NETBSD_SOURCE` for platform compatibility
- When using inline functions, there's always a non-inline fallback (lowercase function name)
- The style system (`gui/stylesheet.c`) allows runtime theming via CSS-like syntax
- Always check `HAVE_*` defines before using platform-specific features
- Use `AG_Malloc()`, `AG_Realloc()`, `AG_Free()` instead of direct malloc/free calls

