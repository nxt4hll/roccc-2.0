TARGET_LIB =	x86_halt_blpp

PAIRS =		recipe.h init.h
CPPS =		$(PAIRS:.h=.cpp)
HDRS =		$(PAIRS) x86_halt_blpp.h
OBJS =		$(CPPS:.cpp=.o)

NWCPPS =	recipe.cpp

LIBS =		-lmachine -lcfg -lhalt -lhalt_blpp -lx86 -lx86_halt

include $(MACHSUIFHOME)/Makefile.common
