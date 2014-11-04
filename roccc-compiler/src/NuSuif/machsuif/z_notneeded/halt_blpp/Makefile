TARGET_LIB =	halt_blpp

# Subdirectories to visit
PACKAGES =	blpp

CPPS =		kinds.cpp init.cpp
HDRS =		$(CPPS:.cpp=.h) halt_blpp.h
OBJS =		$(CPPS:.cpp=.o)
LIBS =		-lmachine -lhalt

#NWHDRS =	kinds.h

include $(MACHSUIFHOME)/Makefile.common
