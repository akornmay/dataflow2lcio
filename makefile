# define compile command

CC = g++

CFLAGS  = -O3 -g -Wno-deprecated -Wl,-no-as-needed -lgsl -lgslcblas -lm -llcio -lsio -lz 
LDFLAGS = -L/usr/local/lib -L$(LCIO)/lib
IFLAGS = -I$(LCIO)/include

OBJ 	= dataflow2lcio.cc EUTelConvertSim.cpp
HEAD    = EUTelConvertSim.h

ROOTCFLAGS    =$(shell $(ROOTSYS)/bin/root-config --cflags)
ROOTLIBS      =$(shell $(ROOTSYS)/bin/root-config --libs)
ROOTGLIBS     =$(shell $(ROOTSYS)/bin/root-config --glibs) 


CFLAGS       += $(ROOTCFLAGS)

all: $(OBJ) $(HEAD) makefile
	$(CC) $(OBJ) -o dataflow2lcio $(CFLAGS) $(LDFLAGS) $(ROOTGLIBS) $(IFLAGS)

clean:	
	rm -f *.o dataflow2lcio *~ Analyse

