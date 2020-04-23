![Agar](mk/agar-logo.png)

### What is it?

Agar is a type of sugar polymer obtained from seaweed and red algae. It becomes gelatinous in water and is primarly used as a culture medium for microbiological work. Agar is also the name of an open source GUI toolkit, a set of software libraries for building graphical user interfaces.

Agar works with a wide variety of platforms and graphics systems, from modern desktops to classic consoles and embedded devices. Agar's low-level layers are modular, extensible and easily ported to new hardware platforms and graphics systems. Standard [AG_Drivers](https://libagar.org/man3/AG_Driver) include:

* [sdlfb](https://libagar.org/man3/AG_DriverSDLFB) (SDL, frame-buffer)
* [sdlgl](https://libagar.org/man3/AG_DriverSDLGL) (SDL, OpenGL)
* [glx](https://libagar.org/man3/AG_DriverGLX) (X11, OpenGL)
* [cocoa](https://libagar.org/man3/AG_DriverCocoa) (MacOS X, OpenGL)
* [wgl](https://libagar.org/man3/AG_DriverWGL) (Windows, OpenGL)

For graphics systems without window capabilities (or cases where a MDI-style interface is preferred over desktop integration), a _single-window_ mode is provided in which case Agar provides its own internal window manager. Agar implements a powerful styling engine and includes a real-time "Style Editor" with support for cascading [stylesheets](https://libagar.org/man3/AG_StyleSheet).

Agar includes a good standard library of general-purpose widgets, but it is always meant to be extended. It is customary to write new widgets as part of an application or an external library. Complex user interface elements can be realized often using very few lines of code. The [class registration interface](https://libagar.org/man3/AG_Object#CLASSES) simplifies the process.

To help developers with the debugging process, the Debug build performs extensive object validity and class-membership tests on all pointers passed to API calls, so memory errors trigger run-time "Illegal access" assertions as early as possible. Agar is [thread-safe](https://libagar.org/man3/AG_Threads) when built with threads support. Public API calls are reentrant (unless documented otherwise), but Agar does not rely on threads for its own operation.

### Libraries included (stable)

* [CORE](https://libagar.org/man3/AG_Intro#AGAR-CORE): Platform, I/O, object system (non graphical)
* [GUI (Base)](https://libagar.org/man3/AG_Intro#AGAR-GUI:_BASE_SYSTEM): GUI base framework
* [GUI (Widgets)](https://libagar.org/man3/AG_Intro#AGAR-GUI:_STANDARD_WIDGETS): GUI standard widgets
* [MATH](https://libagar.org/man3/AG_Intro#AGAR-MATH): Matrices, vectors & advanced rendering methods
* [NET](https://libagar.org/man3/AG_Intro#AGAR-NET): Network interface, HTTP application server
* [VG](https://libagar.org/man3/AG_Intro#AGAR-VG): Vector graphics

### Libraries included (beta)

* [AU](https://libagar.org/man3/AG_Intro#AGAR-AU): Audio interface library
* [SG](https://libagar.org/man3/AG_Intro#AGAR-SG): General-purpose 3D engine
* [SK](https://libagar.org/man3/AG_Intro#AGAR-SK): Sketches with constraints
* [MAP](https://libagar.org/man3/AG_Intro#AGAR-MAP): Tile engine

### How to install

See [**INSTALL.md**](INSTALL.md), or:

- **[On BSD](https://libagar.org/docs/inst/bsd.html)** ![](img/bsd.png)
- **[On Linux](https://libagar.org/docs/inst/linux.html)** ![](img/linux.png)
- **[On MacOS](https://libagar.org/docs/inst/osx.html)** ![](img/osx.png)
- **[On Unix](https://libagar.org/docs/inst/unix.html)** ![](img/sunhp.png)
- **[On Windows with MingGW32](https://libagar.org/docs/inst/win-mingw.html)** ![](img/win.png)
- **[On Windows with MingGW64](https://libagar.org/docs/inst/win-mingw64.html)** ![](img/win.png)
- **[On Windows with Visual Studio](https://libagar.org/docs/inst/win-vs.html)** ![](img/win.png)

### Availability

#### Latest Stable Release

* [Download Source](https://libagar.org/download.html#stable)
* On FreeBSD: ***pkg install agar***

#### Latest Development Sources

* Main Repo: [https://dev.csoft.net/agar/trunk](https://dev.csoft.net/agar/trunk)
* On GitHub: [JulNadeauCA/libagar](https://github.com/JulNadeauCA/libagar)

#### Portability

Agar is [portable](https://libagar.org/portable.html) to different platforms including FreeBSD, IRIX, Linux, MacOS Classic, MacOS X, NetBSD, OpenBSD, Solaris and Windows. Agar has been used on ARM embedded devices. It has even been ported to game consoles such as the GP2x, the Nintendo Gamecube/Wii and the Xbox.

#### License

* [BSD 2-Clause License](https://libagar.org/license.html)

### Contributing

* Sign up to the [**Patreon**](https://patreon.com/libagar) for exclusive content including articles, code and tutorials.
* [**Contribute**](https://libagar.org/contribute.html) by donating, supporting our sponsors, reporting bugs or submitting code.

