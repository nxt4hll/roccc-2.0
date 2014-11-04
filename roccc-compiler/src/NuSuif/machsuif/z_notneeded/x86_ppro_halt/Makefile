TARGET_LIB =	x86_ppro_halt

PAIRS =		contexts.h init.h
CPPS =		$(PAIRS:.h=.cpp)
HDRS =		$(PAIRS) x86_ppro_halt.h
OBJS =		$(CPPS:.cpp=.o)

NWHDRS =	
NWCPPS =	

LIBS =		-lmachine -lsuifvm -lcfg -lx86 -lx86_ppro -lx86_halt

include $(MACHSUIFHOME)/Makefile.common
