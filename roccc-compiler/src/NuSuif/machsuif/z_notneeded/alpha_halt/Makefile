TARGET_LIB =	alpha_halt

PAIRS =		recipe.h contexts.h init.h
CPPS =		$(PAIRS:.h=.cpp)
HDRS =		$(PAIRS) alpha_halt.h
OBJS =		$(CPPS:.cpp=.o)

NWHDRS =	
NWCPPS =	

LIBS =		-lmachine -lsuifvm -lcfg -lalpha -lhalt

include $(MACHSUIFHOME)/Makefile.common
