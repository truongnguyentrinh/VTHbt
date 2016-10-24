IDIR =inc/
CC=g++
CFLAGS=-I$(IDIR)

SDIR=src/

ODIR=obj/
LDIR =lib/

LIBS=-lm

_DEPS = VT_AppLib.h VTHbtMonitor.h VT-udp-utility.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = VT_AppLib.o VTHbtDaemon.o VTHbtMonitor.o VT-udp-utility.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: $(SDIR)%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

VTHbt: $(OBJ)
	$(CC) -pthread -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
