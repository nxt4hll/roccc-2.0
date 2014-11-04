TARGET_LIB =	x86_ppro

PAIRS =		instr.h opcodes.h printer.h code_gen.h contexts.h init.h
CPPS =		$(PAIRS:.h=.cpp) translate.cpp
HDRS =		$(PAIRS) x86_ppro.h
OBJS =		$(CPPS:.cpp=.o)

NWHDRS =	contexts.h opcodes.h printer.h
NWCPPS =	contexts.cpp init.cpp

LIBS =		-lmachine -lsuifvm -lx86

include $(MACHSUIFHOME)/Makefile.common
