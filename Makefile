#	$Csoft$

SUBDIR=	 libfobj fobjcomp fobjdump engine
SUBDIR+= geggy

all: all-subdir
clean: clean-subdir

include mk/csoft.subdir.mk
