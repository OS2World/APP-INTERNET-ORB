# Makefile for Orb
# Copyright (C) Craig Berry 1996-1998
# Released under the terms of the Gnu General Public License
# See license.txt for details


.SUFFIXES:
.SUFFIXES: .cc .c .o

ifdef DOS
  EXE_SUFFIX = .exe
  OS_NAME    = dos
  SHRINK     = -djp -s
else
  SHRINK     = strip
endif

ifdef PRF
  PRF_MARK  = p
  PRF_CCOPT = -pg
endif

CFLAGS   = -O2 -g -Wall $(PRF_CCOPT)
CXXFLAGS = $(CFLAGS)
LFLAGS   = $(CFLAGS)
LK       = gcc $(LFLAGS)

APP        = orb$(PRF_MARK)$(EXE_SUFFIX)
ZIP        = orb20
ZIP_SUFFIX = .zip
ZIPX       = $(ZIP)$(OS_NAME)$(ZIP_SUFFIX)
ZIPS       = $(ZIP)s$(ZIP_SUFFIX)

OBJECTS = orb.o orbparse.o orbstate.o strassoc.o \
          errrpt.o util.o varstr.o

all: $(APP)

$(APP):     $(OBJECTS)
	$(LK) -o $@ $^

orb.o:      orb.cc errrpt.h defs.h orbstate.hpp strassoc.hpp varstr.hpp \
            orbver.h util.hpp

orbparse.o: orbparse.cc orbstate.hpp strassoc.hpp varstr.hpp orbver.h

orbstate.o: orbstate.cc errrpt.h defs.h orbstate.hpp strassoc.hpp \
            varstr.hpp util.hpp

strassoc.o: strassoc.cc strassoc.hpp

util.o:     util.cc util.hpp

varstr.o:   varstr.cc varstr.hpp

errrpt.o:   errrpt.c errrpt.h defs.h

clean:
	rm *.o

dist: $(ZIPX) $(ZIPS)

$(ZIPX): $(APP)
	$(SHRINK) $<
	$(RM)     $@
	zip       $@ $^

$(ZIPS): *.cc *.c *.hpp *.h makefile license.txt orbman.txt
	$(RM) $@
	zip   $@ *.cc *.c
	zip   $@ *.hpp *.h
	zip   $@ makefile license.txt orbman.txt

%.o : %.cc
	$(CC) $(CFLAGS) -c $< -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@


