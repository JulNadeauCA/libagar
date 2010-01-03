# auto generated - do not edit

default: all

all:\
local ag_dso.o ag_event.o agar-core-ada-conf agar-core-ada-conf.o \
agar-core-ada.a agar-core-config.ali agar-core-config.o \
agar-core-data_source.ali agar-core-data_source.o agar-core-database.ali \
agar-core-database.o agar-core-dso.ali agar-core-dso.o agar-core-error.ali \
agar-core-error.o agar-core-event.ali agar-core-event.o agar-core-init.ali \
agar-core-init.o agar-core-object.ali agar-core-object.o agar-core-thin.ali \
agar-core-thin.o agar-core.ali agar-core.o agar.ali agar.o ctxt/bindir.o \
ctxt/ctxt.a ctxt/dlibdir.o ctxt/fakeroot.o ctxt/incdir.o ctxt/repos.o \
ctxt/slibdir.o ctxt/version.o deinstaller deinstaller.o install-core.o \
install-posix.o install-win32.o install.a installer installer.o instchk \
instchk.o insthier.o types-sizes types-sizes.o

# Mkf-deinstall
deinstall: deinstaller conf-sosuffix
	./deinstaller
deinstall-dryrun: deinstaller conf-sosuffix
	./deinstaller dryrun

# Mkf-install
install: installer postinstall conf-sosuffix
	./installer
	./postinstall

install-dryrun: installer conf-sosuffix
	./installer dryrun

# Mkf-instchk
install-check: instchk conf-sosuffix
	./instchk

# Mkf-local
local:\
flags-agar-core libs-agar-core flags-c_string libs-c_string-S
	./check-deps

local_pre:
local_clean:

#----------------------------------------------------------------------
# SYSDEPS start

flags-agar-core:
	@echo SYSDEPS agar-flags run create flags-agar-core 
	@(cd SYSDEPS && ./sd-run modules/agar-flags)
libs-agar-core:
	@echo SYSDEPS agar-libs run create libs-agar-core 
	@(cd SYSDEPS && ./sd-run modules/agar-libs)
flags-c_string:
	@echo SYSDEPS c_string-flags run create flags-c_string 
	@(cd SYSDEPS && ./sd-run modules/c_string-flags)
libs-c_string-S:
	@echo SYSDEPS c_string-libs-S run create libs-c_string-S 
	@(cd SYSDEPS && ./sd-run modules/c_string-libs-S)
_sd_sysinfo.h:
	@echo SYSDEPS sd-sysinfo run create _sd_sysinfo.h 
	@(cd SYSDEPS && ./sd-run modules/sd-sysinfo)


agar-flags_clean:
	@echo SYSDEPS agar-flags clean flags-agar-core 
	@(cd SYSDEPS && ./sd-clean modules/agar-flags)
agar-libs_clean:
	@echo SYSDEPS agar-libs clean libs-agar-core 
	@(cd SYSDEPS && ./sd-clean modules/agar-libs)
c_string-flags_clean:
	@echo SYSDEPS c_string-flags clean flags-c_string 
	@(cd SYSDEPS && ./sd-clean modules/c_string-flags)
c_string-libs-S_clean:
	@echo SYSDEPS c_string-libs-S clean libs-c_string-S 
	@(cd SYSDEPS && ./sd-clean modules/c_string-libs-S)
sd-sysinfo_clean:
	@echo SYSDEPS sd-sysinfo clean _sd_sysinfo.h 
	@(cd SYSDEPS && ./sd-clean modules/sd-sysinfo)


sysdeps_clean:\
agar-flags_clean \
agar-libs_clean \
c_string-flags_clean \
c_string-libs-S_clean \
sd-sysinfo_clean \



# SYSDEPS end
#----------------------------------------------------------------------

ada-bind:\
conf-adabind conf-systype conf-adatype conf-adabflags conf-adafflist \
	flags-c_string

ada-compile:\
conf-adacomp conf-adatype conf-systype conf-adacflags conf-adafflist \
	flags-c_string

ada-link:\
conf-adalink conf-adatype conf-systype conf-adaldflags conf-aldfflist \
	libs-c_string-S

ada-srcmap:\
conf-adacomp conf-adatype conf-systype

ada-srcmap-all:\
ada-srcmap conf-adacomp conf-adatype conf-systype

ag_dso.o:\
cc-compile ag_dso.c
	./cc-compile ag_dso.c

ag_event.o:\
cc-compile ag_event.c
	./cc-compile ag_event.c

agar-core-ada-conf:\
cc-link agar-core-ada-conf.ld agar-core-ada-conf.o ctxt/ctxt.a
	./cc-link agar-core-ada-conf agar-core-ada-conf.o ctxt/ctxt.a

agar-core-ada-conf.o:\
cc-compile agar-core-ada-conf.c ctxt.h _sd_sysinfo.h
	./cc-compile agar-core-ada-conf.c

agar-core-ada.a:\
cc-slib agar-core-ada.sld ag_dso.o ag_event.o agar-core-config.o \
agar-core-data_source.o agar-core-database.o agar-core-dso.o agar-core-error.o \
agar-core-event.o agar-core-init.o agar-core-object.o agar-core-thin.o \
agar-core.o agar.o
	./cc-slib agar-core-ada ag_dso.o ag_event.o agar-core-config.o \
	agar-core-data_source.o agar-core-database.o agar-core-dso.o agar-core-error.o \
	agar-core-event.o agar-core-init.o agar-core-object.o agar-core-thin.o \
	agar-core.o agar.o

agar-core-config.ads:\
agar-core.ali

agar-core-config.o agar-core-config.ali:\
ada-compile agar-core-config.adb agar-core.ali agar-core-config.ads \
agar-core-thin.ali
	./ada-compile agar-core-config.adb

agar-core-data_source.ads:\
agar-core-thin.ali

agar-core-data_source.o agar-core-data_source.ali:\
ada-compile agar-core-data_source.adb agar-core-data_source.ads
	./ada-compile agar-core-data_source.adb

agar-core-database.ads:\
agar-core.ali agar-core-thin.ali

agar-core-database.o agar-core-database.ali:\
ada-compile agar-core-database.adb agar-core.ali agar-core-database.ads
	./ada-compile agar-core-database.adb

agar-core-dso.ads:\
agar-core.ali agar-core-thin.ali

agar-core-dso.o agar-core-dso.ali:\
ada-compile agar-core-dso.adb agar-core.ali agar-core-dso.ads
	./ada-compile agar-core-dso.adb

agar-core-error.ads:\
agar-core.ali

agar-core-error.o agar-core-error.ali:\
ada-compile agar-core-error.adb agar-core.ali agar-core-error.ads \
agar-core-thin.ali
	./ada-compile agar-core-error.adb

agar-core-event.ads:\
agar-core.ali agar-core-thin.ali

agar-core-event.o agar-core-event.ali:\
ada-compile agar-core-event.adb agar-core.ali agar-core-event.ads
	./ada-compile agar-core-event.adb

agar-core-init.ads:\
agar-core.ali agar-core-thin.ali

agar-core-init.o agar-core-init.ali:\
ada-compile agar-core-init.adb agar-core.ali agar-core-init.ads
	./ada-compile agar-core-init.adb

agar-core-object.ads:\
agar-core.ali agar-core-thin.ali agar-core-data_source.ali

agar-core-object.o agar-core-object.ali:\
ada-compile agar-core-object.adb agar-core.ali agar-core-object.ads
	./ada-compile agar-core-object.adb

# agar-core-thin.ads.mff
agar-core-thin.ads: agar-core-thin.ads.sh type-make types-sizes
	./agar-core-thin.ads.sh > agar-core-thin.ads.tmp
	mv agar-core-thin.ads.tmp agar-core-thin.ads

agar-core-thin.o agar-core-thin.ali:\
ada-compile agar-core-thin.ads
	./ada-compile agar-core-thin.ads

agar-core.o agar-core.ali:\
ada-compile agar-core.ads agar.ali agar-core.ads
	./ada-compile agar-core.ads

agar.o agar.ali:\
ada-compile agar.ads agar.ads
	./ada-compile agar.ads

cc-compile:\
conf-cc conf-cctype conf-systype conf-ccfflist flags-agar-core

cc-link:\
conf-ld conf-ldtype conf-systype conf-ldfflist libs-agar-core

cc-slib:\
conf-systype

conf-adatype:\
mk-adatype
	./mk-adatype > conf-adatype.tmp && mv conf-adatype.tmp conf-adatype

conf-cctype:\
conf-cc mk-cctype
	./mk-cctype > conf-cctype.tmp && mv conf-cctype.tmp conf-cctype

conf-ldtype:\
conf-ld mk-ldtype
	./mk-ldtype > conf-ldtype.tmp && mv conf-ldtype.tmp conf-ldtype

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
cc-slib ctxt/ctxt.sld ctxt/bindir.o ctxt/dlibdir.o ctxt/fakeroot.o \
ctxt/incdir.o ctxt/repos.o ctxt/slibdir.o ctxt/version.o
	./cc-slib ctxt/ctxt ctxt/bindir.o ctxt/dlibdir.o ctxt/fakeroot.o ctxt/incdir.o \
	ctxt/repos.o ctxt/slibdir.o ctxt/version.o

# ctxt/dlibdir.c.mff
ctxt/dlibdir.c: mk-ctxt conf-dlibdir
	rm -f ctxt/dlibdir.c
	./mk-ctxt ctxt_dlibdir < conf-dlibdir > ctxt/dlibdir.c

ctxt/dlibdir.o:\
cc-compile ctxt/dlibdir.c
	./cc-compile ctxt/dlibdir.c

# ctxt/fakeroot.c.mff
ctxt/fakeroot.c: mk-ctxt conf-fakeroot
	rm -f ctxt/fakeroot.c
	./mk-ctxt ctxt_fakeroot < conf-fakeroot > ctxt/fakeroot.c

ctxt/fakeroot.o:\
cc-compile ctxt/fakeroot.c
	./cc-compile ctxt/fakeroot.c

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
cc-link deinstaller.ld deinstaller.o insthier.o install.a ctxt/ctxt.a
	./cc-link deinstaller deinstaller.o insthier.o install.a ctxt/ctxt.a

deinstaller.o:\
cc-compile deinstaller.c install.h ctxt.h
	./cc-compile deinstaller.c

install-core.o:\
cc-compile install-core.c install.h
	./cc-compile install-core.c

install-posix.o:\
cc-compile install-posix.c install.h
	./cc-compile install-posix.c

install-win32.o:\
cc-compile install-win32.c install.h
	./cc-compile install-win32.c

install.a:\
cc-slib install.sld install-core.o install-posix.o install-win32.o
	./cc-slib install install-core.o install-posix.o install-win32.o

install.h:\
install_os.h

installer:\
cc-link installer.ld installer.o insthier.o install.a ctxt/ctxt.a
	./cc-link installer installer.o insthier.o install.a ctxt/ctxt.a

installer.o:\
cc-compile installer.c ctxt.h install.h
	./cc-compile installer.c

instchk:\
cc-link instchk.ld instchk.o insthier.o install.a ctxt/ctxt.a
	./cc-link instchk instchk.o insthier.o install.a ctxt/ctxt.a

instchk.o:\
cc-compile instchk.c ctxt.h install.h
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
conf-cc conf-ld

mk-systype:\
conf-cc conf-ld

types-sizes:\
cc-link types-sizes.ld types-sizes.o
	./cc-link types-sizes types-sizes.o

types-sizes.o:\
cc-compile types-sizes.c
	./cc-compile types-sizes.c

clean-all: sysdeps_clean local_clean obj_clean ext_clean
clean: obj_clean
obj_clean:
	rm -f ag_dso.o ag_event.o agar-core-ada-conf agar-core-ada-conf.o \
	agar-core-ada.a agar-core-config.ali agar-core-config.o \
	agar-core-data_source.ali agar-core-data_source.o agar-core-database.ali \
	agar-core-database.o agar-core-dso.ali agar-core-dso.o agar-core-error.ali \
	agar-core-error.o agar-core-event.ali agar-core-event.o agar-core-init.ali \
	agar-core-init.o agar-core-object.ali agar-core-object.o agar-core-thin.ads \
	agar-core-thin.ali agar-core-thin.o agar-core.ali agar-core.o agar.ali agar.o \
	ctxt/bindir.c ctxt/bindir.o ctxt/ctxt.a ctxt/dlibdir.c ctxt/dlibdir.o \
	ctxt/fakeroot.c ctxt/fakeroot.o ctxt/incdir.c ctxt/incdir.o ctxt/repos.c \
	ctxt/repos.o ctxt/slibdir.c ctxt/slibdir.o ctxt/version.c ctxt/version.o \
	deinstaller deinstaller.o install-core.o install-posix.o install-win32.o \
	install.a installer installer.o instchk instchk.o insthier.o types-sizes \
	types-sizes.o
ext_clean:
	rm -f conf-adatype conf-cctype conf-ldtype conf-systype mk-ctxt

regen:\
ada-srcmap ada-srcmap-all
	./ada-srcmap-all
	cpj-genmk > Makefile.tmp && mv Makefile.tmp Makefile
