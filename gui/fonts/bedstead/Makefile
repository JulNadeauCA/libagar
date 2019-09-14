FONTBASES = bedstead bedstead-extended bedstead-semicondensed \
            bedstead-condensed bedstead-extracondensed bedstead-ultracondensed

SFDFILES = $(addsuffix .sfd, $(FONTBASES))
OTFFILES = $(addsuffix .otf, $(FONTBASES))

DISTFILES = bedstead.c Makefile COPYING NEWS \
	$(OTFFILES) \
	bedstead-10.bdf bedstead-20.bdf \
	bedstead-10-df.png bedstead-20-df.png \
	bedstead-complement.pdf

all: $(DISTFILES) \
     sample.png title.png extended.png

.PHONY: experimental
experimental: bedstead-chiseltip.otf bedstead-plotter-thin.otf \
 bedstead-plotter-light.otf bedstead-plotter-medium.otf \
 bedstead-plotter-bold.otf plotter.png

bedstead.sfd: bedstead
	./bedstead > bedstead.sfd

bedstead-extended.sfd: bedstead
	./bedstead --extended > bedstead-extended.sfd

bedstead-semicondensed.sfd: bedstead
	./bedstead --semi-condensed > bedstead-semicondensed.sfd

bedstead-condensed.sfd: bedstead
	./bedstead --condensed > bedstead-condensed.sfd

bedstead-extracondensed.sfd: bedstead
	./bedstead --extra-condensed > bedstead-extracondensed.sfd

bedstead-ultracondensed.sfd: bedstead
	./bedstead --ultra-condensed > bedstead-ultracondensed.sfd

bedstead-oc.sfd: bedstead
	./bedstead --plotter > bedstead-oc.sfd

bedstead-plotter-%.sfd: strokefont.py bedstead-oc.sfd
	fontforge -lang=py -script strokefont.py plotter-$* bedstead-oc.sfd \
		bedstead-plotter-$*.sfd

bedstead-chiseltip.sfd: strokefont.py bedstead-oc.sfd
	fontforge -lang=py -script strokefont.py chiseltip bedstead-oc.sfd \
		bedstead-chiseltip.sfd

%-10.bdf %-20.bdf: %.sfd
	fontforge -lang=ff \
	    -c 'Open($$1); BitmapsAvail([10, 20]); Generate($$2, "bdf")' \
	    $< $*.

%.otf: %.sfd
	fontforge -lang=ff \
	    -c 'Open($$1); Generate($$2)' \
	    $< $*.otf

%.png: %.ps $(OTFFILES) Fontmap
	gs -P -q -dSAFER -sDEVICE=pnggray -dTextAlphaBits=4 -o $@ $<

bedstead-%-df.png: df.ps bedstead.otf Fontmap
	gs -P -q -dSAFER -dsize=$* -sDEVICE=png16m -o $@ $<

bedstead-complement.ps: bedstead
	./bedstead --complement > bedstead-complement.ps

bedstead-complement.pdf: bedstead-complement.ps bedstead.otf Fontmap
	ps2pdf -P $< $@

.PHONY: clean
clean:
	rm -f bedstead *.sfd *.otf *.bdf *.png *.pdf bedstead-complement.ps

.PHONY: dist

dist: $(DISTFILES)
	rm -rf bedstead-$$(sed -n 's/^Version: //p' < bedstead.sfd)
	mkdir bedstead-$$(sed -n 's/^Version: //p' < bedstead.sfd)
	ln $(DISTFILES) \
	    bedstead-$$(sed -n 's/^Version: //p' < bedstead.sfd)
	zip -r bedstead-$$(sed -n 's/^Version: //p' < bedstead.sfd).zip \
	    bedstead-$$(sed -n 's/^Version: //p' < bedstead.sfd)
	rm -r bedstead-$$(sed -n 's/^Version: //p' < bedstead.sfd)
