![Agar](mk/agar-logo.png)

# How to install Agar

### On [DragonFly](https://www.dragonflybsd.org), [FreeBSD](https://freebsd.org), [NetBSD](https://netbsd.org) & [OpenBSD](https://openbsd.org) ![](img/bsd.png)

##### Prerequisites
- [FreeType](https://freetype.org) (`pkg install freetype`).

##### Optional
- [Xorg](https://www.x.org) (`pkg install xorg`). Enables [**glx**](https://libagar.org/man3/AG_DriverGLX).
- [SDL](https://libsdl.org) (`pkg install sdl`). Enables [**sdlfb**](https://libagar.org/man3/AG_DriverSDLFB) & [**sdlgl**](https://libagar.org/man3/AG_DriverSDLGL).
- [png](http://libpng.org) (`pkg install png`). Enables [**PNG loading**](https://libagar.org/man3/AG_SurfaceFromPNG).
- [jpeg-turbo](https://libjpeg-turbo.org) (`pkg install jpeg-turbo`). Enables [**JPEG loading**](https://libagar.org/man3/AG_SurfaceFromJPEG).
- [libiconv](https://gnu.org/software/libiconv) (`pkg install libiconv`). Support encodings other than ASCII & UTF-8 in [**Textbox**](https://libagar.org/man3/AG_Textbox)/[**Editable**](https://libagar.org/man3/AG_Editable).
- [gettext](https://gnu.org/software/gettext) (`pkg install gettext-runtime gettext-tools`). Enables native language support.

##### Installing from source

1. [**Download**](https://libagar.org/download.html) and unpack the sources to a temporary location:

	`$ wget https://stable.hypertriton.com/agar/agar-1.6.0.tar.gz`

	`$ tar -xzf agar-1.6.0.tar.gz`

	`$ cd agar-1.6.0`

2. Run the **configure** script. Use `--help` to see available options. Some examples:
	- Install to an alternate location (default = `/usr/local`):

	`$ ./configure --prefix=$HOME`

	- Enable run-time assertions, type-safety checks & [GUI debugger](https://libagar.org/man3/AG_GuiDebugger):

	`$ ./configure --enable-debug`

	- Disable support for [multithreading](https://libagar.org/man3/AG_Threads):

	`$ ./configure --disable-threads`

	- Enable beta extension libraries such as [**AU**](https://libagar.org/man3/AU), [**MAP**](https://libagar.org/man3/MAP) & [**AG_Web**](https://libagar.org/man3/AG_Web).

	`$ ./configure --enable-{au,map,web}`

3. Build and install the library:

	`$ make depend all`

	`# make install`

##### Installing from FreeBSD ports and packages

- Install the binary package: `pkg install agar`
- Compile from ports sources: `(cd /usr/ports/devel/agar && make install clean)`

### On [Linux Mint](https://linuxmint.com), [Debian](https://www.debian.org), [Ubuntu](https://ubuntu.com) & other `apt-get`-based distros ![](img/linux.png)

##### Prerequisites
- A C compiler such as [clang](https://clang.llvm.org) or [gcc](https://gcc.gnu.org) (`apt-get install clang` or `apt-get install gcc`).
- [FreeType](https://freetype.org) (`apt-get install libfreetype6-dev`).

##### Optional
- [Xorg](https://www.x.org) with OpenGL (`apt-get libgl1-mesa-dev libxinerama-dev`). Enables [**glx**](https://libagar.org/man3/AG_DriverGLX).
- [Fontconfig](https://www.freedesktop.org/wiki/Software/fontconfig/) (`apt-get install libfontconfig-dev`). Allows system fonts to be used.
- [SDL](https://libsdl.org) (`apt-get install libsdl-dev`). Enables [**sdlfb**](https://libagar.org/man3/AG_DriverSDLFB) & [**sdlgl**](https://libagar.org/man3/AG_DriverSDLGL).
- [png](http://libpng.org) (`apt-get install libpng-dev`). Enables [**PNG loading**](https://libagar.org/man3/AG_SurfaceFromPNG).
- [jpeg-turbo](https://libjpeg-turbo.org) (`apt-get install libjpeg-dev`). Enables [**JPEG loading**](https://libagar.org/man3/AG_SurfaceFromJPEG).
- [gettext](https://gnu.org/software/gettext) (`pkg install gettext`). Enables native language support.

##### Installing from source

1. [**Download**](https://libagar.org/download.html) and unpack the sources to a temporary location:

	`$ wget https://stable.hypertriton.com/agar/agar-1.6.0.tar.gz`

	`$ tar -xzf agar-1.6.0.tar.gz`

	`$ cd agar-1.6.0`

2. Run the **configure** script. Use `--help` to see available options. Some examples:

	- Install to an alternate location (default = `/usr/local`):

	`$ ./configure --prefix=$HOME`

	- Enable run-time assertions, type-safety checks & [GUI debugger](https://libagar.org/man3/AG_GuiDebugger):

	`$ ./configure --enable-debug`

	- Disable support for [multithreading](https://libagar.org/man3/AG_Threads):

	`$ ./configure --disable-threads`

	- Enable beta extension libraries such as [**AU**](https://libagar.org/man3/AU), [**MAP**](https://libagar.org/man3/MAP) & [**AG_Web**](https://libagar.org/man3/AG_Web).

	`$ ./configure --enable-{au,map,web}`

3. Build and install the library:

	`$ make depend all`

	`# make install`

4. If necessary refresh your [ld.so](http://man7.org/linux/man-pages/man8/ld.so.8.html) cache:

	`# ldconfig`

### On Linux [Fedora](https://fedoraproject.org) & other `yum`-based distros ![](img/linux.png)

##### Prerequisites
- A C compiler such as [clang](https://clang.llvm.org) or [gcc](https://gcc.gnu.org) (`yum install clang` or `yum install gcc`).
- [FreeType](https://freetype.org) (`yum install freetype-devel`).

##### Optional
- [Xorg](https://www.x.org) with OpenGL (`yum install mesa-libGL-devel`). Enables [**glx**](https://libagar.org/man3/AG_DriverGLX).
- [Fontconfig](https://www.freedesktop.org/wiki/Software/fontconfig/) (`yum install fontconfig-devel`). Allows system fonts to be used.
- [SDL](https://libsdl.org) (`yum install SDL-devel`). Enables [**sdlfb**](https://libagar.org/man3/AG_DriverSDLFB) & [**sdlgl**](https://libagar.org/man3/AG_DriverSDLGL).
- [png](http://libpng.org) (`yum install libpng-devel`). Enables [**PNG loading**](https://libagar.org/man3/AG_SurfaceFromPNG).
- [jpeg-turbo](https://libjpeg-turbo.org) (`yum install libjpeg-turbo-devel`). Enables [**JPEG loading**](https://libagar.org/man3/AG_SurfaceFromJPEG).
- [gettext](https://gnu.org/software/gettext) (`yum install gettext`). Enables native language support.

##### Installing from source

1. [**Download**](https://libagar.org/download.html) and unpack the sources to a temporary location:

	`$ wget https://stable.hypertriton.com/agar/agar-1.6.0.tar.gz`

	`$ tar -xzf agar-1.6.0.tar.gz`

	`$ cd agar-1.6.0`

2. Run the **configure** script. Use `--help` to see available options. Some examples:

	- Install to an alternate location (default = `/usr/local`):

	`$ ./configure --prefix=$HOME`

	- Enable run-time assertions, type-safety checks & [GUI debugger](https://libagar.org/man3/AG_GuiDebugger):

	`$ ./configure --enable-debug`

	- Disable support for [multithreading](https://libagar.org/man3/AG_Threads):

	`$ ./configure --disable-threads`

	- Enable beta extension libraries such as [**AU**](https://libagar.org/man3/AU), [**MAP**](https://libagar.org/man3/MAP) & [**AG_Web**](https://libagar.org/man3/AG_Web).

	`$ ./configure --enable-{au,map,web}`

3. Build and install the library:

	`$ make depend all`

	`# make install`

4. If necessary add `/usr/local/lib` to `/etc/ld.so.conf` and refresh your [ld.so](http://man7.org/linux/man-pages/man8/ld.so.8.html) cache:

	`# echo "/usr/local/lib" >> /etc/ld.so.conf`

	`# ldconfig`

### On [MacOS / OS X](https://en.wikipedia.org/wiki/MacOS) ![](img/osx.png)

##### Prerequisites
- [Xcode](https://developer.apple.com/xcode) 3.14 or later.
- [FreeType](https://freetype.org) 2.10.1 or later ([download source](https://download.savannah.gnu.org/releases/freetype/), `./configure && make install`).

##### Optional
- [SDL](https://libsdl.org) ([Download 1.2.x source](https://libsdl.org/download-1.2.php), `./configure && make install`). Enables [**sdlfb**](https://libagar.org/man3/AG_DriverSDLFB) & [**sdlgl**](https://libagar.org/man3/AG_DriverSDLGL).
- [gettext](https://gnu.org/software/gettext) ([Download source](https://ftp.gnu.org/pub/gnu/gettext/), `./configure && make install`). Enables native language support.

##### Installing from source

1. [**Download**](https://libagar.org/download.html) and unpack the sources to a temporary location:

	`$ wget https://stable.hypertriton.com/agar/agar-1.6.0.tar.gz`

	`$ tar -xzf agar-1.6.0.tar.gz`

	`$ cd agar-1.6.0`

2. Run the **configure** script. Use `--help` to see available options. Some examples:

	- Install to an alternate location (default = `/usr/local`):

	`$ ./configure --prefix=$HOME`

	- Enable run-time assertions, type-safety checks & [GUI debugger](https://libagar.org/man3/AG_GuiDebugger):

	`$ ./configure --enable-debug`

	- Disable support for [multithreading](https://libagar.org/man3/AG_Threads):

	`$ ./configure --disable-threads`

	- Enable beta extension libraries such as [**AU**](https://libagar.org/man3/AU), [**MAP**](https://libagar.org/man3/MAP) & [**AG_Web**](https://libagar.org/man3/AG_Web).

	`$ ./configure --enable-{au,map,web}`

3. Build and install the library:

	`$ make depend all`

	`# make install`

4. If necessary add `/usr/local/lib` to `/etc/ld.so.conf` and refresh your [ld.so](http://man7.org/linux/man-pages/man8/ld.so.8.html) cache:

	`# echo "/usr/local/lib" >> /etc/ld.so.conf`

	`# ldconfig`

### On [Windows](https://en.wikipedia.org/wiki/Microsoft_Windows) ![](img/win.png)

- [**Windows / MinGW-w64** build](https://libagar.org/docs/inst/win-mingw64.html)
- [**Windows / MinGW32** build](https://libagar.org/docs/inst/win-mingw.html)
- [**Windows / Visual Studio** build](https://libagar.org/docs/inst/win-vs.html)

