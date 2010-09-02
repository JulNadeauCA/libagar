TOP=	..
include ${TOP}/Makefile.config

SUBDIR=	fixedres \
	objsystem \
	focusing \
	loader \
	table \
	textbox \
	themes \
	maximized \
	unitconv \
	sockets \
	glview \
	cplusplus \
	plotting \
	scrollbar \
	keyevents \
	customwidget \
	customeventloop \
	windows \
	threads \
	minimal \
	textdlg \
	scrollview \
	console \
	chinese \
	rendertosurface \
	reinit \
	modalwindowhandler \
	configsettings \
	sdl \
	timeouts \
	pane \
	imageloading

all: all-subdir
clean: prereq clean-subdir
cleandir: prereq cleandir-subdir cleandir-cache
deinstall: deinstall-subdir
depend: depend-subdir

install:
	@echo "*"
	@echo "* You probably don't want to install the demos on your system,"
	@echo "* they are meant to be executed from the source directory."
	@echo "*"
	@exit 1

configure:
	@if [ -e ".configurecache" ]; then \
		echo "rm -fR .configurecache"; \
		rm -fR .configurecache; \
	fi
	mkdir .configurecache
	@for D in ${SUBDIR}; do \
		echo "==> $$D"; \
		(cd $$D && ./configure --cache=../.configurecache); \
		if [ $$? != 0 ]; then \
			echo "* "; \
			echo "* If the previous test failed due to Agar not being found,"; \
			echo "* make sure you have already installed Agar on your system"; \
			echo "* and that the agar-config binary is in your PATH."; \
			echo "* "; \
			echo "* To install Agar, go to the top-level directory and run:"; \
			echo "*     $ make all install"; \
			echo "* "; \
			exit 1; \
		fi; \
	done

cleandir-cache:
	rm -fR .configurecache

configure-scripts:
	for D in ${SUBDIR}; do \
		echo "==> $$D"; \
		(cd $$D && ${MAKE} configure); \
	done

prereq:
	for D in ${SUBDIR}; do \
		echo "==> $$D"; \
		touch $$D/Makefile.config; \
		touch $$D/.depend; \
	done

proj:
	for D in ${SUBDIR}; do \
		echo "==> $$D"; \
		(cd $$D && touch Makefile.config && ${MAKE} proj); \
	done

proj-vs2005:
	for D in ${SUBDIR}; do \
		echo "==> $$D"; \
		(cd $$D && unzip ProjectFiles/vs2005-windows.zip); \
	done

.PHONY: clean cleandir cleandir-cache install deinstall depend configure configure-scripts prereq proj proj-vs2005

include ${TOP}/mk/build.common.mk
include ${TOP}/mk/build.subdir.mk
