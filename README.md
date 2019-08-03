![Agar](mk/agar-logo.png)

### What is it?

Agar is a type of sugar polymer obtained from seaweed and red algae. Agar
becomes gelatinous in water and is primarly used as a culture medium for
microbiological work. Agar is also the name of an open source GUI toolkit,
a set of software libraries for building graphical user interfaces.

Agar works with a wide variety of platforms and graphics systems, from
modern desktops to classic consoles and embedded devices. Agar's low-level
layers are modular, extensible and easily ported to new hardware platforms
and graphics systems. Standard [drivers](http://libagar.org/man3/AG_Driver)
include:

* sdlfb (SDL, frame-buffer)
* sdlgl (SDL, OpenGL)
* glx (X11, OpenGL)
* cocoa (MacOS X, OpenGL)
* wgl (Windows, OpenGL)

For graphics systems without window capabilities, a _single-window_ mode
is provided (in which case Agar provides an internal window manager).

A set of general-purpose widgets are included in the Agar distribution.
Agar's class registration interface allows new widgets to be implemented
externally (i.e., as part of an application or library). GUI elements are
styled with [Cascading Stylesheets](http://libagar.org/man3/AG_StyleSheet).

Agar is [thread-safe](http://libagar.org/man3/AG_Threads) when built with
threads support.

### Libraries Included

* [CORE](http://libagar.org/man3/AG_Intro#AGAR-CORE):
  Platform, I/O, Object System
* [GUI (Base)](http://libagar.org/man3/AG_Intro#AGAR-GUI:_BASE_SYSTEM):
  GUI Framework
* [GUI (Widgets)](http://libagar.org/man3/AG_Intro#AGAR-GUI:_STANDARD_WIDGETS):
  GUI Standard Widgets
* [MATH](http://libagar.org/man3/AG_Intro#AGAR-MATH):
  Matrices, Vectors, Advanced Rendering Methods
* [SG](http://libagar.org/man3/AG_Intro#AGAR-SG):
  3D Engine
* [SK](http://libagar.org/man3/AG_Intro#AGAR-SK):
  2D Sketch and Constraint Solver
* [RG](http://libagar.org/man3/AG_Intro#AGAR-RG):
  Feature-based Tileable Graphics
* [AU](http://libagar.org/man3/AG_Intro#AGAR-AU):
  Audio Interface Library
* [MAP](http://libagar.org/man3/AG_Intro#AGAR-MAP):
  Simple Tile Engine
* [VG](http://libagar.org/man3/AG_Intro#AGAR-VG):
  Simple 2D Vector Graphics

**CORE** implements Agar's [Object System](http://libagar.org/man3/AG_Object),
a single-inheritance system that can be used from different languages (for
example, it is possible to subclass an Agar class written in C with one
written in Ada and vice-versa). Agar objects are serializable (and **CORE**
provides [serialization routines](http://libagar.org/man3/AG_DataSource)
to help with that). A tree of Agar objects is referred to as a VFS or
_virtual filesystem_. The **CORE** library is non-graphical and usable by
command-line programs or daemons. It provides simple interfaces to operating
system services such as filesystem access, dynamic library loading, network
services, process execution, I/O multiplexing, timers and threads.

### Installation

* **[On BSD](http://libagar.org/docs/inst/bsd.html)**
  ![BSD](img/bsd.png)
* **[On Linux](http://libagar.org/docs/inst/linux.html)**
  ![BSD](img/linux.png)
* **[On MacOS](http://libagar.org/docs/inst/osx.html)**
  ![BSD](img/osx.png)
* **[On Unix](http://libagar.org/docs/inst/unix.html)**
  ![BSD](img/sunhp.png)
* **[On Windows with MingGW32](http://libagar.org/docs/inst/win-mingw.html)**
  ![BSD](img/win.png)
* **[On Windows with MingGW64](http://libagar.org/docs/inst/win-mingw64.html)**
  ![BSD](img/win.png)
* **[On Windows with Visual Studio](http://libagar.org/docs/inst/win-vs.html)**
  ![BSD](img/win.png)

### Availability

#### Latest Stable Release

* [Download Source](http://libagar.org/download.html#stable)
* On FreeBSD: ***pkg install agar***

#### Latest Development Sources

* Main Repo: [https://dev.csoft.net/agar/trunk](https://dev.csoft.net/agar/trunk)
* On GitHub: [JulNadeauCA/libagar](https://github.com/JulNadeauCA/libagar)

#### Portability

Agar is [portable](http://libagar.org/portable.html) to different platforms
including FreeBSD, IRIX, Linux, MacOS Classic, MacOS X, NetBSD, OpenBSD,
Solaris and Windows. Agar has been used on ARM embedded devices. It has even
been ported to game consoles such as the GP2x, the Nintendo Gamecube/Wii and
the Xbox.

#### License

* [BSD 2-Clause License](http://libagar.org/license.html)

#### Contributing

* [Contribute to the project](http://libagar.org/contribute.html)

