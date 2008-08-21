# auto generated - do not edit

default: all

all:\
UNIT_TESTS/ada_size UNIT_TESTS/ada_size.ali UNIT_TESTS/ada_size.o \
UNIT_TESTS/c_size UNIT_TESTS/c_size.o agar-core-ada-conf agar-core-ada-conf.o \
agar-core-ada.a agar-core-config.ali agar-core-config.o \
agar-core-datasource.ali agar-core-datasource.o agar-core-error.ali \
agar-core-error.o agar-core-event.ali agar-core-event.o agar-core-object.ali \
agar-core-object.o agar-core-tail_queue.ali agar-core-tail_queue.o \
agar-core-threads.ali agar-core-threads.o agar-core-timeout.ali \
agar-core-timeout.o agar-core-types.ali agar-core-types.o agar-core-version.ali \
agar-core-version.o agar-core.ali agar-core.o agar.ali agar.o chk-debug \
chk-debug.o chk-lockdebug chk-lockdebug.o chk-off_t chk-off_t.o chk-threads \
chk-threads.o ctxt/bindir.o ctxt/ctxt.a ctxt/dlibdir.o ctxt/incdir.o \
ctxt/repos.o ctxt/slibdir.o ctxt/version.o deinstaller deinstaller.o inst-check \
inst-check.o inst-copy inst-copy.o inst-dir inst-dir.o inst-link inst-link.o \
install_core.o install_error.o installer installer.o instchk instchk.o \
insthier.o

# Mkf-deinstall
deinstall: deinstaller inst-check inst-copy inst-dir inst-link
	./deinstaller
deinstall-dryrun: deinstaller inst-check inst-copy inst-dir inst-link
	./deinstaller dryrun

# Mkf-install
install: installer inst-check inst-copy inst-dir inst-link postinstall
	./installer
	./postinstall

install-dryrun: installer inst-check inst-copy inst-dir inst-link
	./installer dryrun

# Mkf-instchk
install-check: instchk inst-check
	./instchk

# Mkf-test
tests:
	(cd UNIT_TESTS && make)
tests_clean:
	(cd UNIT_TESTS && make clean)

# -- SYSDEPS start
flags-agar-core:
	@echo SYSDEPS agar-flags run create flags-agar-core 
	@(cd SYSDEPS/modules/agar-flags && ./run)
libs-agar-core:
	@echo SYSDEPS agar-libs run create libs-agar-core 
	@(cd SYSDEPS/modules/agar-libs && ./run)
flags-sdl:
	@echo SYSDEPS sdl-flags run create flags-sdl 
	@(cd SYSDEPS/modules/sdl-flags && ./run)
_sysinfo.h:
	@echo SYSDEPS sysinfo run create _sysinfo.h 
	@(cd SYSDEPS/modules/sysinfo && ./run)


agar-flags_clean:
	@echo SYSDEPS agar-flags clean flags-agar-core 
	@(cd SYSDEPS/modules/agar-flags && ./clean)
agar-libs_clean:
	@echo SYSDEPS agar-libs clean libs-agar-core 
	@(cd SYSDEPS/modules/agar-libs && ./clean)
sdl-flags_clean:
	@echo SYSDEPS sdl-flags clean flags-sdl 
	@(cd SYSDEPS/modules/sdl-flags && ./clean)
sysinfo_clean:
	@echo SYSDEPS sysinfo clean _sysinfo.h 
	@(cd SYSDEPS/modules/sysinfo && ./clean)


sysdeps_clean:\
agar-flags_clean \
agar-libs_clean \
sdl-flags_clean \
sysinfo_clean \


# -- SYSDEPS end


UNIT_TESTS/ada_size:\
ada-bind ada-link UNIT_TESTS/ada_size.ald UNIT_TESTS/ada_size.ali \
agar-core-config.ali agar-core-datasource.ali agar-core-error.ali \
agar-core-event.ali agar-core-object.ali agar-core-tail_queue.ali \
agar-core-threads.ali agar-core-timeout.ali agar-core-types.ali \
agar-core-version.ali agar-core.ali agar.ali
	./ada-bind UNIT_TESTS/ada_size.ali
	./ada-link UNIT_TESTS/ada_size UNIT_TESTS/ada_size.ali

UNIT_TESTS/ada_size.ali:\
ada-compile UNIT_TESTS/ada_size.adb agar-core-datasource.ads \
agar-core-event.ads agar-core-object.ads agar-core-threads.ads \
agar-core-timeout.ads agar-core-types.ads agar-core-version.ads agar-core.ads \
agar.ads
	./ada-compile UNIT_TESTS/ada_size.adb

UNIT_TESTS/ada_size.o:\
UNIT_TESTS/ada_size.ali

UNIT_TESTS/c_size:\
cc-link UNIT_TESTS/c_size.ld UNIT_TESTS/c_size.o
	./cc-link UNIT_TESTS/c_size UNIT_TESTS/c_size.o

UNIT_TESTS/c_size.o:\
cc-compile UNIT_TESTS/c_size.c
	./cc-compile UNIT_TESTS/c_size.c

ada-bind:\
conf-adabind conf-systype conf-adatype conf-adabflags conf-adafflist flags-cwd

ada-compile:\
conf-adacomp conf-adatype conf-systype conf-adacflags conf-adafflist flags-cwd

ada-link:\
conf-adalink conf-adatype conf-systype conf-adaldflags conf-aldfflist \
	libs-agar-core

ada-srcmap:\
conf-adacomp conf-adatype conf-systype

ada-srcmap-all:\
ada-srcmap conf-adacomp conf-adatype conf-systype

agar-core-ada-conf:\
cc-link agar-core-ada-conf.ld agar-core-ada-conf.o ctxt/ctxt.a
	./cc-link agar-core-ada-conf agar-core-ada-conf.o ctxt/ctxt.a

agar-core-ada-conf.o:\
cc-compile agar-core-ada-conf.c ctxt.h _sysinfo.h
	./cc-compile agar-core-ada-conf.c

agar-core-ada.a:\
cc-slib agar-core-ada.sld agar-core-config.o agar-core-datasource.o \
agar-core-error.o agar-core-event.o agar-core-object.o agar-core-tail_queue.o \
agar-core-threads.o agar-core-timeout.o agar-core-types.o agar-core-version.o \
agar-core.o agar.o
	./cc-slib agar-core-ada agar-core-config.o agar-core-datasource.o \
	agar-core-error.o agar-core-event.o agar-core-object.o agar-core-tail_queue.o \
	agar-core-threads.o agar-core-timeout.o agar-core-types.o agar-core-version.o \
	agar-core.o agar.o

agar-core-config.ali:\
ada-compile agar-core-config.adb agar-core-config.ads
	./ada-compile agar-core-config.adb

agar-core-config.o:\
agar-core-config.ali

agar-core-datasource.ads:\
agar-core-threads.ads agar-core-types.ads

agar-core-datasource.ali:\
ada-compile agar-core-datasource.adb agar-core-datasource.ads
	./ada-compile agar-core-datasource.adb

agar-core-datasource.o:\
agar-core-datasource.ali

agar-core-error.ali:\
ada-compile agar-core-error.adb
	./ada-compile agar-core-error.adb

agar-core-error.o:\
agar-core-error.ali

agar-core-event.ads:\
agar-core-types.ads agar-core-timeout.ads agar-core-tail_queue.ads

agar-core-event.ali:\
ada-compile agar-core-event.adb agar-core-event.ads
	./ada-compile agar-core-event.adb

agar-core-event.o:\
agar-core-event.ali

# agar-core-object.ads.mff
agar-core-object.ads: agar-core-object.sh chk-debug chk-lockdebug chk-off_t chk-threads agar-core-object.ads.txt
	./agar-core-object.sh > agar-core-object.ads.tmp && mv agar-core-object.ads.tmp agar-core-object.ads

agar-core-object.ali:\
ada-compile agar-core-object.adb agar-core-object.ads
	./ada-compile agar-core-object.adb

agar-core-object.o:\
agar-core-object.ali

agar-core-tail_queue.ali:\
ada-compile agar-core-tail_queue.ads agar-core-tail_queue.ads
	./ada-compile agar-core-tail_queue.ads

agar-core-tail_queue.o:\
agar-core-tail_queue.ali

agar-core-threads.ali:\
ada-compile agar-core-threads.ads agar-core-threads.ads agar-core-types.ads
	./ada-compile agar-core-threads.ads

agar-core-threads.o:\
agar-core-threads.ali

agar-core-timeout.ads:\
agar-core-tail_queue.ads agar-core-types.ads

agar-core-timeout.ali:\
ada-compile agar-core-timeout.adb agar-core-timeout.ads
	./ada-compile agar-core-timeout.adb

agar-core-timeout.o:\
agar-core-timeout.ali

# agar-core-types.ads.mff
agar-core-types.ads: chk-off_t agar-core-types.sh
	./agar-core-types.sh > agar-core-types.ads.tmp && mv agar-core-types.ads.tmp agar-core-types.ads

agar-core-types.ali:\
ada-compile agar-core-types.ads
	./ada-compile agar-core-types.ads

agar-core-types.o:\
agar-core-types.ali

agar-core-version.ali:\
ada-compile agar-core-version.ads agar-core-version.ads \
agar-core-datasource.ads agar-core-types.ads
	./ada-compile agar-core-version.ads

agar-core-version.o:\
agar-core-version.ali

agar-core.ali:\
ada-compile agar-core.ads agar-core.ads
	./ada-compile agar-core.ads

agar-core.o:\
agar-core.ali

agar.ali:\
ada-compile agar.ads agar.ads
	./ada-compile agar.ads

agar.o:\
agar.ali

cc-compile:\
conf-cc conf-cctype conf-systype conf-ccfflist flags-agar-core flags-sdl

cc-link:\
conf-ld conf-ldtype conf-systype

cc-slib:\
conf-systype

chk-debug:\
cc-link chk-debug.ld chk-debug.o
	./cc-link chk-debug chk-debug.o

chk-debug.o:\
cc-compile chk-debug.c
	./cc-compile chk-debug.c

chk-lockdebug:\
cc-link chk-lockdebug.ld chk-lockdebug.o
	./cc-link chk-lockdebug chk-lockdebug.o

chk-lockdebug.o:\
cc-compile chk-lockdebug.c
	./cc-compile chk-lockdebug.c

chk-off_t:\
cc-link chk-off_t.ld chk-off_t.o
	./cc-link chk-off_t chk-off_t.o

chk-off_t.o:\
cc-compile chk-off_t.c
	./cc-compile chk-off_t.c

chk-threads:\
cc-link chk-threads.ld chk-threads.o
	./cc-link chk-threads chk-threads.o

chk-threads.o:\
cc-compile chk-threads.c
	./cc-compile chk-threads.c

conf-adatype:\
mk-adatype
	./mk-adatype > conf-adatype.tmp && mv conf-adatype.tmp conf-adatype

conf-cctype:\
conf-cc mk-cctype
	./mk-cctype > conf-cctype.tmp && mv conf-cctype.tmp conf-cctype

conf-ldtype:\
conf-ld mk-ldtype
	./mk-ldtype > conf-ldtype.tmp && mv conf-ldtype.tmp conf-ldtype

conf-sosuffix:\
mk-sosuffix
	./mk-sosuffix > conf-sosuffix.tmp && mv conf-sosuffix.tmp conf-sosuffix

conf-systype:\
mk-systype
	./mk-systype > conf-systype.tmp && mv conf-systype.tmp conf-systype

# ctxt/bindir.c.mff
ctxt/bindir.c: mk-ctxt conf-bindir
	rm -f ctxt/bindir.c
	./mk-ctxt ctxt_bindir < conf-bindir > ctxt/bindir.c

ctxt/bindir.o:\
cc-compile ctxt/bindir.c
	./cc-compile ctxt/bindir.c

ctxt/ctxt.a:\
cc-slib ctxt/ctxt.sld ctxt/bindir.o ctxt/dlibdir.o ctxt/incdir.o ctxt/repos.o \
ctxt/slibdir.o ctxt/version.o
	./cc-slib ctxt/ctxt ctxt/bindir.o ctxt/dlibdir.o ctxt/incdir.o ctxt/repos.o \
	ctxt/slibdir.o ctxt/version.o

# ctxt/dlibdir.c.mff
ctxt/dlibdir.c: mk-ctxt conf-dlibdir
	rm -f ctxt/dlibdir.c
	./mk-ctxt ctxt_dlibdir < conf-dlibdir > ctxt/dlibdir.c

ctxt/dlibdir.o:\
cc-compile ctxt/dlibdir.c
	./cc-compile ctxt/dlibdir.c

# ctxt/incdir.c.mff
ctxt/incdir.c: mk-ctxt conf-incdir
	rm -f ctxt/incdir.c
	./mk-ctxt ctxt_incdir < conf-incdir > ctxt/incdir.c

ctxt/incdir.o:\
cc-compile ctxt/incdir.c
	./cc-compile ctxt/incdir.c

# ctxt/repos.c.mff
ctxt/repos.c: mk-ctxt conf-repos
	rm -f ctxt/repos.c
	./mk-ctxt ctxt_repos < conf-repos > ctxt/repos.c

ctxt/repos.o:\
cc-compile ctxt/repos.c
	./cc-compile ctxt/repos.c

# ctxt/slibdir.c.mff
ctxt/slibdir.c: mk-ctxt conf-slibdir
	rm -f ctxt/slibdir.c
	./mk-ctxt ctxt_slibdir < conf-slibdir > ctxt/slibdir.c

ctxt/slibdir.o:\
cc-compile ctxt/slibdir.c
	./cc-compile ctxt/slibdir.c

# ctxt/version.c.mff
ctxt/version.c: mk-ctxt VERSION
	rm -f ctxt/version.c
	./mk-ctxt ctxt_version < VERSION > ctxt/version.c

ctxt/version.o:\
cc-compile ctxt/version.c
	./cc-compile ctxt/version.c

deinstaller:\
cc-link deinstaller.ld deinstaller.o insthier.o install_core.o install_error.o \
ctxt/ctxt.a
	./cc-link deinstaller deinstaller.o insthier.o install_core.o install_error.o \
	ctxt/ctxt.a

deinstaller.o:\
cc-compile deinstaller.c install.h
	./cc-compile deinstaller.c

inst-check:\
cc-link inst-check.ld inst-check.o install_error.o
	./cc-link inst-check inst-check.o install_error.o

inst-check.o:\
cc-compile inst-check.c install.h
	./cc-compile inst-check.c

inst-copy:\
cc-link inst-copy.ld inst-copy.o install_error.o
	./cc-link inst-copy inst-copy.o install_error.o

inst-copy.o:\
cc-compile inst-copy.c install.h
	./cc-compile inst-copy.c

inst-dir:\
cc-link inst-dir.ld inst-dir.o install_error.o
	./cc-link inst-dir inst-dir.o install_error.o

inst-dir.o:\
cc-compile inst-dir.c install.h
	./cc-compile inst-dir.c

inst-link:\
cc-link inst-link.ld inst-link.o install_error.o
	./cc-link inst-link inst-link.o install_error.o

inst-link.o:\
cc-compile inst-link.c install.h
	./cc-compile inst-link.c

install_core.o:\
cc-compile install_core.c install.h
	./cc-compile install_core.c

install_error.o:\
cc-compile install_error.c install.h
	./cc-compile install_error.c

installer:\
cc-link installer.ld installer.o insthier.o install_core.o install_error.o \
ctxt/ctxt.a
	./cc-link installer installer.o insthier.o install_core.o install_error.o \
	ctxt/ctxt.a

installer.o:\
cc-compile installer.c install.h
	./cc-compile installer.c

instchk:\
cc-link instchk.ld instchk.o insthier.o install_core.o install_error.o \
ctxt/ctxt.a
	./cc-link instchk instchk.o insthier.o install_core.o install_error.o \
	ctxt/ctxt.a

instchk.o:\
cc-compile instchk.c install.h
	./cc-compile instchk.c

insthier.o:\
cc-compile insthier.c ctxt.h install.h
	./cc-compile insthier.c

mk-adatype:\
conf-adacomp conf-systype

mk-cctype:\
conf-cc conf-systype

mk-ctxt:\
mk-mk-ctxt
	./mk-mk-ctxt

mk-ldtype:\
conf-ld conf-systype conf-cctype

mk-mk-ctxt:\
conf-cc

mk-sosuffix:\
conf-systype

mk-systype:\
conf-cc

clean-all: sysdeps_clean tests_clean obj_clean ext_clean
clean: obj_clean
obj_clean:
	rm -f UNIT_TESTS/ada_size UNIT_TESTS/ada_size.ali UNIT_TESTS/ada_size.o \
	UNIT_TESTS/c_size UNIT_TESTS/c_size.o agar-core-ada-conf agar-core-ada-conf.o \
	agar-core-ada.a agar-core-config.ali agar-core-config.o \
	agar-core-datasource.ali agar-core-datasource.o agar-core-error.ali \
	agar-core-error.o agar-core-event.ali agar-core-event.o agar-core-object.ads \
	agar-core-object.ali agar-core-object.o agar-core-tail_queue.ali \
	agar-core-tail_queue.o agar-core-threads.ali agar-core-threads.o \
	agar-core-timeout.ali agar-core-timeout.o agar-core-types.ads \
	agar-core-types.ali agar-core-types.o agar-core-version.ali agar-core-version.o \
	agar-core.ali agar-core.o agar.ali agar.o chk-debug chk-debug.o chk-lockdebug \
	chk-lockdebug.o chk-off_t chk-off_t.o chk-threads chk-threads.o ctxt/bindir.c \
	ctxt/bindir.o ctxt/ctxt.a ctxt/dlibdir.c ctxt/dlibdir.o ctxt/incdir.c \
	ctxt/incdir.o ctxt/repos.c ctxt/repos.o ctxt/slibdir.c ctxt/slibdir.o \
	ctxt/version.c ctxt/version.o deinstaller deinstaller.o inst-check inst-check.o \
	inst-copy inst-copy.o inst-dir inst-dir.o inst-link inst-link.o install_core.o \
	install_error.o
	rm -f installer installer.o instchk instchk.o insthier.o
ext_clean:
	rm -f conf-adatype conf-cctype conf-ldtype conf-sosuffix conf-systype mk-ctxt

regen:\
ada-srcmap ada-srcmap-all
	./ada-srcmap-all
	cpj-genmk > Makefile.tmp && mv Makefile.tmp Makefile
