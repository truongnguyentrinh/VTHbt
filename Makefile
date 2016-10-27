IDIR =inc
CC=g++
CFLAGS=-I$(IDIR)

SDIR=src/

ODIR=obj
LDIR =lib/

LIBS=-lm

_DEPS = VT_AppLib.h VTHbtMonitor.h VT-udp-utility.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = VT_AppLib.o VTHbtDaemon.o VTHbtMonitor.o VT-udp-utility.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_OBJ_FOR_APP_LIB = VT_AppLib.o VT-udp-utility.o
OBJ_FOR_APP_LIB = $(patsubst %,$(ODIR)/%,$(_OBJ_FOR_APP_LIB))

_OBJ_TEST_APP = VT_AppLib.o VT-udp-utility.o testApp.o
OBJ_TEST_APP = $(patsubst %,$(ODIR)/%,$(_OBJ_TEST_APP))

$(ODIR)/%.o: $(SDIR)%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

VTHbt: $(OBJ)
	$(CC) -pthread -o $@ $^ $(CFLAGS) $(LIBS)

testApp: $(OBJ_TEST_APP)
	$(CC) -pthread -o $@ $^ $(CFLAGS) $(LIBS)

lib/libAppUtil.a: $(OBJ_FOR_APP_LIB)
	ar rcs $@ $^

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
