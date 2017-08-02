# Makefile for WaveTools
#
# Bernhard OEMER

CC = gcc
CCOPT = -O2 
LNKOPT = -O2 -lm
INSTDIR = /usr/local/bin
VERSION = 1.0
NAME = wavetools-$(VERSION)

# make 			Executables
# make install		Install to $(INSTDIR)
# make doc		Generate German Documentation (wdoc.ps)
# make clean		Remove temp. files
# make cleanall		Remove all generated files
# make uninstall	Remove installed binaries

all: exec

exec: winf wcat wcut wflt wfct wmix wview wplot

doc: wdoc.ps

clean: 
	rm -f *.o
	rm -f [A-Z]*.wav [A-Z]*.ps wdoc.log wdoc.aux wdoc.dvi

clear: clean
	rm -f winf wcat wcut wflt wfct wmix wview wplot
	rm -f wdoc.ps

# Create Executables

winf: winf.o wave.o
	$(CC) -o winf winf.o wave.o $(LNKOPT)

wcat: wcat.o wave.o
	$(CC) -o wcat wcat.o wave.o $(LNKOPT)

wcut: wcut.o wave.o
	$(CC) -o wcut wcut.o wave.o $(LNKOPT)

wflt: wflt.o wave.o
	$(CC) -o wflt wflt.o wave.o $(LNKOPT)

wfct: wfct.o wave.o
	$(CC) -o wfct wfct.o wave.o $(LNKOPT)

wmix: wmix.o wave.o
	$(CC) -o wmix wmix.o wave.o $(LNKOPT)

wview: wview.o wave.o 
	$(CC) -o wview wview.o wave.o $(LNKOPT) -lvga

wplot: wplot.o wave.o
	$(CC) -o wplot wplot.o wave.o $(LNKOPT)

wave.o: wave.c wave.h
	$(CC) $(CCOPT) -c wave.c

winf.o: winf.c wave.h
	$(CC) $(CCOPT) -c winf.c

wcat.o: wcat.c wave.h
	$(CC) $(CCOPT) -c wcat.c

wcut.o: wcut.c wave.h
	$(CC) $(CCOPT) -c wcut.c

wflt.o: wflt.c wave.h
	$(CC) $(CCOPT) -c wflt.c

wfct.o: wfct.c wave.h defaults.h
	$(CC) $(CCOPT) -c wfct.c

wmix.o: wmix.c wave.h
	$(CC) $(CCOPT) -c wmix.c

# no -O2 here, unless you want to whitness a real strange Pentium or gcc bug
wview.o: wview.c wave.h font8x8.h
	$(CC) -c wview.c 

wplot.o: wplot.c wave.h psheader.h psplot.h defaults.h
	$(CC) $(CCOPT) -c wplot.c


# Create Manual Files

doc: wdoc.ps

wdoc.ps: wdoc.dvi figures
	dvips -D 300 -r -Z -t A4 -o wdoc.ps wdoc.dvi

wdoc.aux: wdoc.tex figures
	latex wdoc

wdoc.dvi: wdoc.tex wdoc.aux figures
	latex wdoc

figures: fig.wfct.ps fig.wmix.ps fig.wflt.ps fig.math.ps

fig.wfct.ps: wplot Sinus.wav Dreieck.wav Rechteck.wav Impuls.wav
	./wplot -w6cm -f10pt -oSinus.ps Sinus.wav
	./wplot -w6cm -f10pt -oDreieck.ps Dreieck.wav
	./wplot -w6cm -f10pt -oRechteck.ps Rechteck.wav
	./wplot -w6cm -f10pt -oImpuls.ps Impuls.wav

fig.wmix.ps: wplot Schwebung.wav
	./wplot -w10cm -f10pt -l100ms -oSchwebung.ps Schwebung.wav

fig.wflt.ps: wplot Lowpass.wav Highpass.wav Bandpass.wav Meanvalue.wav
	./wplot -w6cm -f10pt -oLowpass.ps Lowpass.wav
	./wplot -w6cm -f10pt -oHighpass.ps Highpass.wav
	./wplot -w6cm -f10pt -oBandpass.ps Bandpass.wav
	./wplot -w6cm -f10pt -oMeanvalue.ps Meanvalue.wav

fig.math.ps: low1000.ps high1000.ps bnd1k200.ps

Sinus.wav: wfct
	./wfct -s4f -b16 -oSinus.wav 440Hz 10ms

Dreieck.wav: wfct
	./wfct -s4f -b16 -t -oDreieck.wav 300Hz 10ms

Rechteck.wav: wfct
	./wfct -s4f -b16 -r -p90deg -oRechteck.wav 500hz 10ms

Impuls.wav: wfct
	./wfct -s4f -b16 -i2ms -p60% -a70% -oImpuls.wav 100Hz 10ms

Schwebung.wav: wfct wmix
	(./wfct -s4f -b16 410Hz 1s; ./wfct -s4f -b16 390Hz 1s) | ./wmix -s -o Schwebung.wav - -

Lowpass.wav: wflt Impuls.wav
	./wflt -l200Hz -oLowpass.wav Impuls.wav

Highpass.wav: wflt Impuls.wav
	./wflt -f200Hz -oHighpass.wav Impuls.wav

Bandpass.wav: wflt Impuls.wav
	./wflt -b200Hz -oBandpass.wav Impuls.wav

Meanvalue.wav: wflt Impuls.wav
	./wflt -m1kHz -o Meanvalue.wav Impuls.wav


# Install

install: exec
	install -m755 -oroot -groot winf $(INSTDIR)
	install -m755 -oroot -groot wcat $(INSTDIR)
	install -m755 -oroot -groot wcut $(INSTDIR)
	install -m755 -oroot -groot wflt $(INSTDIR)
	install -m755 -oroot -groot wfct $(INSTDIR)
	install -m755 -oroot -groot wmix $(INSTDIR)
	install -m755 -oroot -groot wview $(INSTDIR)
	install -m755 -oroot -groot wplot $(INSTDIR)

uninstall: 
	cd $(INSTDIR) &&  rm -f winf wcat wcut wflt wfct wmix wview wplot

dist-src:
	mkdir wavetools-$(VERSION)
	cp README COPYING Makefile low1000.ps high1000.ps bnd1k200.ps wdoc.tex *.h *.c wavetools-$(VERSION)
	tar czf wavetools-$(VERSION).tgz --owner=0 --group=0 wavetools-$(VERSION)
	rm -r wavetools-$(VERSION)

dist-bin:
	mkdir wavetools-$(VERSION)-bin
	cp README COPYING Makefile wavetools.pdf winf wcat wcut wflt wfct wmix wview wplot wavetools-$(VERSION)-bin
	tar czf wavetools-$(VERSION)-bin.tgz --owner=0 --group=0 wavetools-$(VERSION)-bin
	rm -r wavetools-$(VERSION)-bin
	

version: cleanall
	cd ..  && tar czf $(NAME).tgz $(NAME)
	make exec
	cd .. && tar czf $(NAME).ELF.tgz \
	  $(NAME)/README  $(NAME)/wavetools.lsm \
	  $(NAME)/winf $(NAME)/wcat $(NAME)/wcut $(NAME)/wflt \
	  $(NAME)/wfct $(NAME)/wmix $(NAME)/wview $(NAME)/wplot
	make doc
	cd .. && tar czf $(NAME).doc.tgz \
	  $(NAME)/README  $(NAME)/wavetools.lsm \
	  $(NAME)/wdoc.ps


			  
