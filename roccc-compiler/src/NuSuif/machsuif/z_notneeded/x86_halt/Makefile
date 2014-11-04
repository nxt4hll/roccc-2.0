TARGET_LIB =	x86_halt

PAIRS =		recipe.h contexts.h init.h
CPPS =		$(PAIRS:.h=.cpp)
HDRS =		$(PAIRS) x86_halt.h
OBJS =		$(CPPS:.cpp=.o)

NWHDRS =	contexts.h
NWCPPS =	recipe.h recipe.cpp init.cpp

LIBS =		-lmachine -lsuifvm -lcfg -lx86 -lhalt

include $(MACHSUIFHOME)/Makefile.common
