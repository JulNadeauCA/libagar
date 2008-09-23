# auto generated - do not edit

default: all

all:\
demo.ali demo.o keyevent keyevent.ali keyevent.o keyevent_callbacks.ali \
keyevent_callbacks.o slider slider.ali slider.o slider_callbacks.ali \
slider_callbacks.o winicon winicon.ali winicon.o winicon_callbacks.ali \
winicon_callbacks.o

# -- SYSDEPS start
flags-agar-core-ada:
	@echo SYSDEPS agar-core-ada-flags run create flags-agar-core-ada 
	@(cd SYSDEPS/modules/agar-core-ada-flags && ./run)
libs-agar-core-ada:
	@echo SYSDEPS agar-core-ada-libs run create libs-agar-core-ada 
	@(cd SYSDEPS/modules/agar-core-ada-libs && ./run)
flags-agar-gui-ada:
	@echo SYSDEPS agar-gui-ada-flags run create flags-agar-gui-ada 
	@(cd SYSDEPS/modules/agar-gui-ada-flags && ./run)
libs-agar-gui-ada:
	@echo SYSDEPS agar-gui-ada-libs run create libs-agar-gui-ada 
	@(cd SYSDEPS/modules/agar-gui-ada-libs && ./run)
libs-agar-gui:
	@echo SYSDEPS agar-gui-libs run create libs-agar-gui 
	@(cd SYSDEPS/modules/agar-gui-libs && ./run)
libs-agar-core:
	@echo SYSDEPS agar-libs run create libs-agar-core 
	@(cd SYSDEPS/modules/agar-libs && ./run)
flags-sdl-ada:
	@echo SYSDEPS sdl-ada-flags run create flags-sdl-ada 
	@(cd SYSDEPS/modules/sdl-ada-flags && ./run)
libs-sdl-ada:
	@echo SYSDEPS sdl-ada-libs run create libs-sdl-ada 
	@(cd SYSDEPS/modules/sdl-ada-libs && ./run)
libs-sdl-image:
	@echo SYSDEPS sdl-image-libs run create libs-sdl-image 
	@(cd SYSDEPS/modules/sdl-image-libs && ./run)
flags-sdl-img-ada:
	@echo SYSDEPS sdl-img-ada-flags run create flags-sdl-img-ada 
	@(cd SYSDEPS/modules/sdl-img-ada-flags && ./run)
libs-sdl-img-ada:
	@echo SYSDEPS sdl-img-ada-libs run create libs-sdl-img-ada 
	@(cd SYSDEPS/modules/sdl-img-ada-libs && ./run)
libs-sdl:
	@echo SYSDEPS sdl-libs run create libs-sdl 
	@(cd SYSDEPS/modules/sdl-libs && ./run)
_sysinfo.h:
	@echo SYSDEPS sysinfo run create _sysinfo.h 
	@(cd SYSDEPS/modules/sysinfo && ./run)


agar-core-ada-flags_clean:
	@echo SYSDEPS agar-core-ada-flags clean flags-agar-core-ada 
	@(cd SYSDEPS/modules/agar-core-ada-flags && ./clean)
agar-core-ada-libs_clean:
	@echo SYSDEPS agar-core-ada-libs clean libs-agar-core-ada 
	@(cd SYSDEPS/modules/agar-core-ada-libs && ./clean)
agar-gui-ada-flags_clean:
	@echo SYSDEPS agar-gui-ada-flags clean flags-agar-gui-ada 
	@(cd SYSDEPS/modules/agar-gui-ada-flags && ./clean)
agar-gui-ada-libs_clean:
	@echo SYSDEPS agar-gui-ada-libs clean libs-agar-gui-ada 
	@(cd SYSDEPS/modules/agar-gui-ada-libs && ./clean)
agar-gui-libs_clean:
	@echo SYSDEPS agar-gui-libs clean libs-agar-gui 
	@(cd SYSDEPS/modules/agar-gui-libs && ./clean)
agar-libs_clean:
	@echo SYSDEPS agar-libs clean libs-agar-core 
	@(cd SYSDEPS/modules/agar-libs && ./clean)
sdl-ada-flags_clean:
	@echo SYSDEPS sdl-ada-flags clean flags-sdl-ada 
	@(cd SYSDEPS/modules/sdl-ada-flags && ./clean)
sdl-ada-libs_clean:
	@echo SYSDEPS sdl-ada-libs clean libs-sdl-ada 
	@(cd SYSDEPS/modules/sdl-ada-libs && ./clean)
sdl-image-libs_clean:
	@echo SYSDEPS sdl-image-libs clean libs-sdl-image 
	@(cd SYSDEPS/modules/sdl-image-libs && ./clean)
sdl-img-ada-flags_clean:
	@echo SYSDEPS sdl-img-ada-flags clean flags-sdl-img-ada 
	@(cd SYSDEPS/modules/sdl-img-ada-flags && ./clean)
sdl-img-ada-libs_clean:
	@echo SYSDEPS sdl-img-ada-libs clean libs-sdl-img-ada 
	@(cd SYSDEPS/modules/sdl-img-ada-libs && ./clean)
sdl-libs_clean:
	@echo SYSDEPS sdl-libs clean libs-sdl 
	@(cd SYSDEPS/modules/sdl-libs && ./clean)
sysinfo_clean:
	@echo SYSDEPS sysinfo clean _sysinfo.h 
	@(cd SYSDEPS/modules/sysinfo && ./clean)


sysdeps_clean:\
agar-core-ada-flags_clean \
agar-core-ada-libs_clean \
agar-gui-ada-flags_clean \
agar-gui-ada-libs_clean \
agar-gui-libs_clean \
agar-libs_clean \
sdl-ada-flags_clean \
sdl-ada-libs_clean \
sdl-image-libs_clean \
sdl-img-ada-flags_clean \
sdl-img-ada-libs_clean \
sdl-libs_clean \
sysinfo_clean \


# -- SYSDEPS end


ada-bind:\
conf-adabind conf-systype conf-adatype conf-adafflist flags-agar-core-ada \
	flags-agar-gui-ada flags-sdl-ada flags-sdl-img-ada

ada-compile:\
conf-adacomp conf-adatype conf-systype conf-adacflags conf-adafflist \
	flags-agar-core-ada flags-agar-gui-ada flags-sdl-ada flags-sdl-img-ada

ada-link:\
conf-adalink conf-adatype conf-systype conf-aldfflist libs-agar-core-ada \
	libs-agar-gui-ada libs-agar-gui libs-sdl-ada libs-sdl-img-ada libs-sdl \
	libs-sdl-image

ada-srcmap:\
conf-adacomp conf-adatype conf-systype

ada-srcmap-all:\
ada-srcmap conf-adacomp conf-adatype conf-systype

cc-compile:\
conf-cc conf-cctype conf-systype

cc-link:\
conf-ld conf-ldtype conf-systype

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

demo.ali:\
ada-compile demo.adb demo.ads
	./ada-compile demo.adb

demo.o:\
demo.ali

keyevent:\
ada-bind ada-link keyevent.ald keyevent.ali keyevent_callbacks.ali demo.ali
	./ada-bind keyevent.ali
	./ada-link keyevent keyevent.ali

keyevent.ali:\
ada-compile keyevent.adb demo.ads keyevent_callbacks.ads
	./ada-compile keyevent.adb

keyevent.o:\
keyevent.ali

keyevent_callbacks.ali:\
ada-compile keyevent_callbacks.adb keyevent_callbacks.ads
	./ada-compile keyevent_callbacks.adb

keyevent_callbacks.o:\
keyevent_callbacks.ali

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

mk-systype:\
conf-cc

slider:\
ada-bind ada-link slider.ald slider.ali slider_callbacks.ali demo.ali
	./ada-bind slider.ali
	./ada-link slider slider.ali

slider.ali:\
ada-compile slider.adb demo.ads slider_callbacks.ads
	./ada-compile slider.adb

slider.o:\
slider.ali

slider_callbacks.ali:\
ada-compile slider_callbacks.adb slider_callbacks.ads
	./ada-compile slider_callbacks.adb

slider_callbacks.o:\
slider_callbacks.ali

winicon:\
ada-bind ada-link winicon.ald winicon.ali winicon_callbacks.ali demo.ali
	./ada-bind winicon.ali
	./ada-link winicon winicon.ali

winicon.ali:\
ada-compile winicon.adb demo.ads winicon_callbacks.ads
	./ada-compile winicon.adb

winicon.o:\
winicon.ali

winicon_callbacks.ali:\
ada-compile winicon_callbacks.adb winicon_callbacks.ads
	./ada-compile winicon_callbacks.adb

winicon_callbacks.o:\
winicon_callbacks.ali

clean-all: sysdeps_clean obj_clean ext_clean
clean: obj_clean
obj_clean:
	rm -f demo.ali demo.o keyevent keyevent.ali keyevent.o keyevent_callbacks.ali \
	keyevent_callbacks.o slider slider.ali slider.o slider_callbacks.ali \
	slider_callbacks.o winicon winicon.ali winicon.o winicon_callbacks.ali \
	winicon_callbacks.o
ext_clean:
	rm -f conf-adatype conf-cctype conf-ldtype conf-systype mk-ctxt

regen:\
ada-srcmap ada-srcmap-all
	./ada-srcmap-all
	cpj-genmk > Makefile.tmp && mv Makefile.tmp Makefile
