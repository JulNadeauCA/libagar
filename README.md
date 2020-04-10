![Agar](mk/agar-logo.png)

### What is it?

Agar is a type of sugar polymer obtained from seaweed and red algae. It becomes
gelatinous in water and is primarly used as a culture medium for microbiological
work. Agar is also the name of an open source GUI toolkit, a set of software
libraries for building graphical user interfaces.

Agar works with a wide variety of platforms and graphics systems, from modern
desktops to classic consoles and embedded devices. Agar's low-level layers are
modular, extensible and easily ported to new hardware platforms and graphics
systems. Standard [AG_Drivers](https://libagar.org/man3/AG_Driver) include:

* [sdlfb](https://libagar.org/man3/AG_DriverSDLFB) (SDL, frame-buffer)
* [sdlgl](https://libagar.org/man3/AG_DriverSDLGL) (SDL, OpenGL)
* [glx](https://libagar.org/man3/AG_DriverGLX) (X11, OpenGL)
* [cocoa](https://libagar.org/man3/AG_DriverCocoa) (MacOS X, OpenGL)
* [wgl](https://libagar.org/man3/AG_DriverWGL) (Windows, OpenGL)

For graphics systems without window capabilities (or cases where a MDI-style
interface is preferred over desktop integration), a _single-window_ mode is
provided in which case Agar provides its own internal window manager.

Agar includes a good standard library of general-purpose widgets, but it is
meant to be extended. It is customary to write new Agar widgets as part of an
application or an external library. Complex user interface elements can be
realized often using very few lines of code. The [class registration interface](https://libagar.org/man3/AG_Object#CLASSES)
simplifies the process.

Agar provides a powerful styling engine and a real-time "Style Editor" with
support for cascading [stylesheets](https://libagar.org/man3/AG_StyleSheet).

Agar is [thread-safe](https://libagar.org/man3/AG_Threads) when built with threads support. Public calls are reentrant
(unless documented otherwise). Agar does not rely on threads for its own operation.

To help developers with the debugging process, the Debug build performs
extensive object validity and class-membership tests on all pointers passed to
Agar API calls, so memory errors trigger run-time "Illegal access" assertions
as early as possible.

### Libraries Included (stable)

* [CORE](https://libagar.org/man3/AG_Intro#AGAR-CORE):
  Platform, I/O, Object System (non graphical)
* [GUI (Base)](https://libagar.org/man3/AG_Intro#AGAR-GUI:_BASE_SYSTEM):
  GUI Base Framework
* [GUI (Widgets)](https://libagar.org/man3/AG_Intro#AGAR-GUI:_STANDARD_WIDGETS):
  GUI Standard Widgets
* [MATH](https://libagar.org/man3/AG_Intro#AGAR-MATH):
  Matrices, Vectors, Advanced Rendering Methods
* [NET](https://libagar.org/man3/AG_Intro#AGAR-NET):
  Network Interface, HTTP Application Server
* [VG](https://libagar.org/man3/AG_Intro#AGAR-VG):
  Simple 2D Vector Graphics

### Libraries Included (beta)

* [AU](https://libagar.org/man3/AG_Intro#AGAR-AU):
  Audio Interface Library
* [SG](https://libagar.org/man3/AG_Intro#AGAR-SG):
  3D Engine
* [SK](https://libagar.org/man3/AG_Intro#AGAR-SK):
  2D Sketch and Constraint Solver
* [RG](https://libagar.org/man3/AG_Intro#AGAR-RG):
  Feature-based Tileable Graphics
* [MAP](https://libagar.org/man3/AG_Intro#AGAR-MAP):
  Simple Tile Engine

### Installation

* **[On BSD](https://libagar.org/docs/inst/bsd.html)**
  ![BSD](img/bsd.png)
* **[On Linux](https://libagar.org/docs/inst/linux.html)**
  ![BSD](img/linux.png)
* **[On MacOS](https://libagar.org/docs/inst/osx.html)**
  ![BSD](img/osx.png)
* **[On Unix](https://libagar.org/docs/inst/unix.html)**
  ![BSD](img/sunhp.png)
* **[On Windows with MingGW32](https://libagar.org/docs/inst/win-mingw.html)**
  ![BSD](img/win.png)
* **[On Windows with MingGW64](https://libagar.org/docs/inst/win-mingw64.html)**
  ![BSD](img/win.png)
* **[On Windows with Visual Studio](https://libagar.org/docs/inst/win-vs.html)**
  ![BSD](img/win.png)

### Availability

#### Latest Stable Release

* [Download Source](https://libagar.org/download.html#stable)
* On FreeBSD: ***pkg install agar***

#### Latest Development Sources

* Main Repo: [https://dev.csoft.net/agar/trunk](https://dev.csoft.net/agar/trunk)
* On GitHub: [JulNadeauCA/libagar](https://github.com/JulNadeauCA/libagar)

#### Portability

Agar is [portable](https://libagar.org/portable.html) to different platforms
including FreeBSD, IRIX, Linux, MacOS Classic, MacOS X, NetBSD, OpenBSD,
Solaris and Windows. Agar has been used on ARM embedded devices. It has even
been ported to game consoles such as the GP2x, the Nintendo Gamecube/Wii and
the Xbox.

#### License

* [BSD 2-Clause License](https://libagar.org/license.html)

### Contributing

* [Contribute to the project](https://libagar.org/contribute.html)

#### Support us on Patreon

For exclusive content, tutorials and articles, sign up for the [LibAgar Patreon](https://patreon.com/libagar)
(as low as $1/month).

