#
# CMake Buildsystem for agartest.
#
if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
	message(FATAL_ERROR "Prevented in-tree build. Please create a build directory outside of the agartest source code and run \"cmake -S ${CMAKE_SOURCE_DIR} -B .\" from there")
endif()
cmake_minimum_required(VERSION 3.12.0)

project(AGARTEST
	VERSION 1.7.1
	DESCRIPTION "Test Suite for the Agar GUI System"
	HOMEPAGE_URL "https://libagar.org"
	LANGUAGES C CXX OBJC)
#
# Standard CMake tests.
#
include(CheckLibraryExists)
include(CheckIncludeFiles)
include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckOBJCSourceCompiles)
include(CheckTypeSize)
include(CheckStructHasMember)
include(CMakeDependentOption)
include(FindPkgConfig)
include(GNUInstallDirs)
#
# Imported BSDBuild tests.
#
set(CMAKE_MODULE_PATH "${AGARTEST_SOURCE_DIR}")
set(CONFIG_DIR "${AGARTEST_BINARY_DIR}/config")
file(MAKE_DIRECTORY ${CONFIG_DIR})
include("${AGARTEST_SOURCE_DIR}/CMakeChecks.cmake")

# Detect known platforms.
BB_Detect_Platform()

message(STATUS "    _       _       _     ___")
message(STATUS "  / _ \\   / _ \\   / _ \\  |  _ \\")
message(STATUS " | |_| | | (_| | | |_| | | |_) |")
message(STATUS " |_| |_|  \\__, | |_| |_| |_| |_|")
message(STATUS "          |___/                 ")
message(STATUS "")
message(STATUS "agartest version ${AGARTEST_VERSION} in ${AGARTEST_SOURCE_DIR}")

option(AGARTEST_WARNINGS "Use recommended compiler warnings" OFF)
option(AGARTEST_NLS "Enable internationalization and localization" OFF)

BB_Save_Define_Value(VERSION ${AGARTEST_VERSION})

set(ORIG_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})

#
# agartest sources.
#
set(SOURCE_FILES
	${AGARTEST_SOURCE_DIR}/agartest.c
	${AGARTEST_SOURCE_DIR}/buttons.c
	${AGARTEST_SOURCE_DIR}/charsets.c
	${AGARTEST_SOURCE_DIR}/checkbox.c
	${AGARTEST_SOURCE_DIR}/compositing.c
	${AGARTEST_SOURCE_DIR}/configsettings.c
	${AGARTEST_SOURCE_DIR}/console.c
	${AGARTEST_SOURCE_DIR}/customwidget.c
	${AGARTEST_SOURCE_DIR}/customwidget_mywidget.c
	${AGARTEST_SOURCE_DIR}/fixedres.c
	${AGARTEST_SOURCE_DIR}/focusing.c
	${AGARTEST_SOURCE_DIR}/fonts.c
	${AGARTEST_SOURCE_DIR}/fspaths.c
	${AGARTEST_SOURCE_DIR}/glview.c
	${AGARTEST_SOURCE_DIR}/imageloading.c
	${AGARTEST_SOURCE_DIR}/keyevents.c
	${AGARTEST_SOURCE_DIR}/loader.c
	${AGARTEST_SOURCE_DIR}/maximized.c
	${AGARTEST_SOURCE_DIR}/minimal.c
	${AGARTEST_SOURCE_DIR}/network.c
	${AGARTEST_SOURCE_DIR}/objsystem.c
	${AGARTEST_SOURCE_DIR}/objsystem_animal.c
	${AGARTEST_SOURCE_DIR}/objsystem_mammal.c
	${AGARTEST_SOURCE_DIR}/pane.c
	${AGARTEST_SOURCE_DIR}/palette.c
	${AGARTEST_SOURCE_DIR}/radio.c
	${AGARTEST_SOURCE_DIR}/rendertosurface.c
	${AGARTEST_SOURCE_DIR}/scrollbar.c
	${AGARTEST_SOURCE_DIR}/scrollview.c
	${AGARTEST_SOURCE_DIR}/sockets.c
	${AGARTEST_SOURCE_DIR}/surface.c
	${AGARTEST_SOURCE_DIR}/table.c
	${AGARTEST_SOURCE_DIR}/textbox.c
	${AGARTEST_SOURCE_DIR}/textdlg.c
	${AGARTEST_SOURCE_DIR}/threads.c
	${AGARTEST_SOURCE_DIR}/timeouts.c
	${AGARTEST_SOURCE_DIR}/unitconv.c
	${AGARTEST_SOURCE_DIR}/user.c
	${AGARTEST_SOURCE_DIR}/widgets.c
	${AGARTEST_SOURCE_DIR}/windows.c)

set(DATA_FILES
	${AGARTEST_SOURCE_DIR}/agar.png
	${AGARTEST_SOURCE_DIR}/agar64.png
	${AGARTEST_SOURCE_DIR}/agar-index.png
	${AGARTEST_SOURCE_DIR}/agar.bmp
	${AGARTEST_SOURCE_DIR}/agar-1.bmp
	${AGARTEST_SOURCE_DIR}/agar-2.bmp
	${AGARTEST_SOURCE_DIR}/agar-3.bmp
	${AGARTEST_SOURCE_DIR}/agar-4.bmp
	${AGARTEST_SOURCE_DIR}/axe.bmp
	${AGARTEST_SOURCE_DIR}/axe.png
	${AGARTEST_SOURCE_DIR}/helmet.png
	${AGARTEST_SOURCE_DIR}/helmet.bmp
	${AGARTEST_SOURCE_DIR}/helmet-socket.bmp
	${AGARTEST_SOURCE_DIR}/loss.txt
	${AGARTEST_SOURCE_DIR}/menubg.bmp
	${AGARTEST_SOURCE_DIR}/pepe.jpg
	${AGARTEST_SOURCE_DIR}/sword.bmp
	${AGARTEST_SOURCE_DIR}/sword-socket.bmp
	${AGARTEST_SOURCE_DIR}/sq-agar.bmp
	${AGARTEST_SOURCE_DIR}/sq-agar.png
	${AGARTEST_SOURCE_DIR}/champden.png)

#
# Specify the installation directories.
#
set(AGARTEST_PREFIX ${CMAKE_INSTALL_PREFIX})
set(AGARTEST_DATADIR "${AGARTEST_PREFIX}/share/agartest")
set(AGARTEST_LOCALEDIR "${AGARTEST_DATADIR}/locale")

BB_Save_Define_Value(PREFIX ${AGARTEST_PREFIX})
BB_Save_Define_Value(DATADIR ${AGARTEST_DATADIR})
BB_Save_Define_Value(LOCALEDIR ${AGARTEST_LOCALEDIR})

# Check for a C compiler (and extended compiler attributes).
Check_Cc()

# Check for the Agar library.
Check_Agar()
if(NOT HAVE_AGAR)
	message(FATAL_ERROR "The Agar library was not found.")
endif()

# XXX TODO
#Check_Agar_Math()
#Check_Agar_AU()
set(HAVE_AGAR_MATH OFF)
set(HAVE_AGAR_AU OFF)
BB_Save_Undef(HAVE_PORTAUDIO)
BB_Save_Undef(HAVE_AGAR_MATH)
BB_Save_Undef(HAVE_AGAR_AU)
BB_Save_Undef(HAVE_RAND48)

if(AGARTEST_NLS)
	Check_Gettext()
	if(HAVE_GETTEXT)
		BB_Save_Define(ENABLE_NLS)
	else()
		BB_Save_Undef(ENABLE_NLS)
	endif()
else()
	Disable_Gettext()
	BB_Save_Undef(ENABLE_NLS)
endif()

if(HAVE_AGAR_MATH)
	set(SOURCE_FILES ${SOURCE_FILES}
		${AGARTEST_SOURCE_DIR}/bezier.c
		${AGARTEST_SOURCE_DIR}/bezier_widget.c
		${AGARTEST_SOURCE_DIR}/math.c
		${AGARTEST_SOURCE_DIR}/plotting.c
		${AGARTEST_SOURCE_DIR}/string.c)
endif()

if(HAVE_AGAR_AU)
	set(SOURCE_FILES ${SOURCE_FILES}
		${AGARTEST_SOURCE_DIR}/audio.c)
endif()

message(STATUS "")
message(STATUS "agartest build successfully configured with the following options:")
message(STATUS "")
message(STATUS "Platform:     ${CMAKE_SYSTEM}")
message(STATUS "Compiler:     ${CMAKE_C_COMPILER}")
message(STATUS "Agar Version: ${AGAR_VERSION} (Library Version: ${AGAR_LIBRARY_VERSION})")

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	message(STATUS "Build Type:   DEBUG")
	message(STATUS "")
	message(STATUS "CMAKE_C_FLAGS_DEBUG:   ${CMAKE_C_FLAGS_DEBUG}")
	message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
else()
	message(STATUS "Build Type:   RELEASE")
	message(STATUS "")
endif()
message(STATUS "CFLAGS:        ${CMAKE_C_FLAGS}")
message(STATUS "EXTRA_CFLAGS:  ${EXTRA_CFLAGS}")
message(STATUS "EXTRA_LDFLAGS: ${EXTRA_LDFLAGS}")
message(STATUS "EXTRA_LIBS:    ${EXTRA_LIBS}")
message(STATUS "AGAR_CFLAGS:   ${AGAR_CFLAGS}")
message(STATUS "AGAR_LIBS:     ${AGAR_LIBS}")
message(STATUS "")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

add_executable(agartest WIN32 MACOSX_BUNDLE ${SOURCE_FILES})

#
# Definitions, Options and Include Directories.
#
target_compile_definitions(agartest PRIVATE
	_USE_AGAR_STD
	_USE_AGAR_QUEUE
	_USE_AGAR_TYPES
	_USE_AGAR_ATTRIBUTES
	_USE_AGAR_NULLABILITY)

if(CYGWIN)
	set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -mno-cygwin")
	check_c_source_compiles("int main(int argc, char **argv) {}" HAVE_GCC_NO_CYGWIN)
	set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
	if(HAVE_GCC_NO_CYGWIN)
		target_compile_options(agartest PRIVATE "-mno-cygwin")
	endif()
	target_include_directories(agartest PRIVATE "/usr/include/mingw")
endif()

target_include_directories(agartest PRIVATE "${AGARTEST_BINARY_DIR}")

if(AGARTEST_WARNINGS)
	target_compile_options(agartest PRIVATE "-Wall" "-Werror" "-Wmissing-prototypes"
	    "-Wno-switch")
	if(DARWIN)
		target_compile_options(agartest PRIVATE "-Wno-deprecated-declarations")
	endif()
endif()

if(WINDOWS_STORE)
	target_compile_definitions(agartest PRIVATE AGARTEST_BUILDING_WINRT=1)
	target_compile_options(agartest PRIVATE "-ZW")
endif()

if(MSVC)
	target_compile_definitions(agartest PRIVATE _CRT_SECURE_NO_WARNINGS)
	target_compile_definitions(agartest PRIVATE _CRT_SECURE_NO_DEPRECATE)
	target_compile_options(agartest PRIVATE "/MP")
endif()

#
# Libraries to link against.
#
target_compile_options(agartest PRIVATE ${AGAR_CFLAGS})
target_link_libraries(agartest LINK_PRIVATE ${AGAR_LIBS})

if(AGARTEST_NLS AND HAVE_GETTEXT)
	target_compile_options(agartest PRIVATE ${GETTEXT_CFLAGS})
	target_link_libraries(agartest LINK_PRIVATE ${GETTEXT_LIBS})
endif()
if(HAVE_AGAR_MATH)
	target_compile_options(agartest PRIVATE ${AGAR_MATH_CFLAGS})
	target_link_libraries(agartest LINK_PRIVATE ${AGAR_MATH_LIBS})
endif()
if(HAVE_AGAR_AU)
	target_compile_options(agartest PRIVATE ${AGAR_AU_CFLAGS})
	target_link_libraries(agartest LINK_PRIVATE ${AGAR_AU_LIBS})
endif()

#
# Installation.
#
install(PROGRAMS ${AGARTEST_BINARY_DIR}/agartest TYPE BIN)
install(FILES ${DATA_FILES} DESTINATION "${AGARTEST_DATADIR}")
install(FILES ${AGARTEST_SOURCE_DIR}/agartest.1 DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")

#
# Uninstallation.
#
if(NOT TARGET uninstall)
	configure_file(
		"${AGARTEST_SOURCE_DIR}/cmake_uninstall.cmake.in"
		"${AGARTEST_BINARY_DIR}/cmake_uninstall.cmake"
		IMMEDIATE @ONLY)
	add_custom_target(uninstall
		COMMAND ${CMAKE_COMMAND} -P ${AGARTEST_BINARY_DIR}/cmake_uninstall.cmake)
endif()
