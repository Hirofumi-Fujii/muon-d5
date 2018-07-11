# makefile for muon
#
# For MinGW modify the EXEEXT
CXX = g++
#CXXFLAGS = -Wall -DWIN32 -D_WIN32
CXXFLAGS = -Wall
LIBS =
OBJS =
EXEEXT =
#EXEEXT = .exe

.SUFFIXES: .cpp .o

all: coinimga$(EXEEXT) coinimgb$(EXEEXT) coinimgc$(EXEEXT) coinimgf$(EXEEXT) gencoinlistKEK$(EXEEXT) \
	accd5comb$(EXEEXT) genaccd5$(EXEEXT) genflux$(EXEEXT)  gentrx$(EXEEXT) \
	projaccd5$(EXEEXT)

coinimga$(EXEEXT): coinimga.o coinimgopt.o coinrecord.o gzfstream.o gzipfilebuf.o fit2dline.o hist2d.o ncpng.o mytimer.o $(OBJS)
	$(CXX) -s -o $@ coinimga.o coinimgopt.o coinrecord.o gzfstream.o gzipfilebuf.o fit2dline.o hist2d.o ncpng.o mytimer.o \
		 $(OBJS) -lz $(LIBS)

coinimgb$(EXEEXT): coinimgb.o coinimgopt.o coinrecord.o gzfstream.o gzipfilebuf.o fit2dline.o hist2d.o ncpng.o mytimer.o $(OBJS)
	$(CXX) -s -o $@ coinimgb.o coinimgopt.o coinrecord.o gzfstream.o gzipfilebuf.o fit2dline.o hist2d.o ncpng.o mytimer.o \
		$(OBJS) -lz $(LIBS)

coinimgc$(EXEEXT): coinimgc.o coinimgopt.o coinrecord.o gzfstream.o gzipfilebuf.o fit2dline.o hist2d.o ncpng.o mytimer.o $(OBJS)
	$(CXX) -s -o $@ coinimgc.o coinimgopt.o coinrecord.o gzfstream.o gzipfilebuf.o fit2dline.o hist2d.o ncpng.o mytimer.o \
		$(OBJS) -lz $(LIBS)

coinimgf$(EXEEXT): coinimgf.o coinimgopt.o coinrecord.o gzfstream.o gzipfilebuf.o fit2dline.o hist2d.o ncpng.o mytimer.o $(OBJS)
	$(CXX) -s -o $@ coinimgf.o coinimgopt.o coinrecord.o gzfstream.o gzipfilebuf.o fit2dline.o hist2d.o ncpng.o mytimer.o \
		 $(OBJS) -lz $(LIBS)

gencoinlistKEK$(EXEEXT): gencoinlistKEK.o
	$(CXX) -s -o $@ gencoinlistKEK.o

genaccd5$(EXEEXT): genaccd5.o hist2d.o ncpng.o mytimer.o jokisch.o miyake.o $(OBJS)
	$(CXX) -s -o $@ genaccd5.o hist2d.o ncpng.o mytimer.o jokisch.o miyake.o $(OBJS) $(LIBS)

accd5comb$(EXEEXT): accd5comb.o hist2d.o ncpng.o mytimer.o jokisch.o miyake.o $(OBJS)
	$(CXX) -s -o $@ accd5comb.o hist2d.o ncpng.o mytimer.o jokisch.o miyake.o $(OBJS) $(LIBS)

genflux$(EXEEXT): genflux.o genfluxopt.o csvarray.o hist2d.o ncpng.o mytimer.o $(OBJS)
	$(CXX) -s -o $@ genflux.o genfluxopt.o csvarray.o hist2d.o ncpng.o mytimer.o $(OBJS) $(LIBS)

gentrx$(EXEEXT): gentrx.o gentrxopt.o csvarray.o hist2d.o ncpng.o mytimer.o $(OBJS)
	$(CXX) -s -o $@ gentrx.o gentrxopt.o csvarray.o hist2d.o ncpng.o mytimer.o $(OBJS) $(LIBS)

projaccd5$(EXEEXT): projaccd5.o projaccopt.o mytimer.o hist2d.o ncpng.o $(OBJS)
	$(CXX) -s -o $@ projaccd5.o projaccopt.o mytimer.o hist2d.o ncpng.o $(OBJS) $(LIBS)

# Suffix rule
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

# Cleanup rule
clean:
	$(RM) $(OBJS)

# dependencies
coinimga.o:	coinimgopt.h coinrecord.h fit2dline.h hist2d.h gzfstream.h ncpng.h mytimer.h 
coinimgb.o:	coinimgopt.h coinrecord.h fit2dline.h hist2d.h gzfstream.h ncpng.h mytimer.h 
coinimgc.o:	coinimgopt.h coinrecord.h fit2dline.h hist2d.h gzfstream.h ncpng.h mytimer.h 
coinimgf.o:	coinimgopt.h coinrecord.h fit2dline.h hist2d.h gzfstream.h ncpng.h mytimer.h
genaccd5.o:	hist2d.h ncpng.h mytimer.h jokisch.h miyake.h
accd5comb.o:	hist2d.h ncpng.h mytimer.h jokisch.h miyake.h
genflux.o:	genfluxopt.h csvarray.h hist2d.h ncpng.h mytimer.h
gentrx.o:	gentrxopt.h csvarray.h hist2d.h ncpng.h mytimer.h
projaccd5.o:	projaccopt.h hist2d.h ncpng.h mytimer.h
jokisch.o:	jokisch.h
miyake.o:	miyake.h
coinimgopt.o:	coinimgopt.h
coinrecord.o:	coinrecord.h
fit2dline.o:	fit2dline.h
gzstream.o:	gzfstream.h gzipfilebuf,h
gzipfilebuf.o:	gzipfilebuf.h
hist1d.o:	hist1d.h
hist2d.o:	hist2d.h ncpng.h
ncpng.o:	ncpng.h
mytimer.o:	mytimer.h
csvarray.o:	csvarray.h
genfluxopt.o:	genfluxopt.h
gentrxopt.o:	gentrxopt.h
projaccopt.o:	projaccopt.h
#-- end of makefile
