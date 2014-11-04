TARGET_LIB =	suifvm

CPPS =		opcodes.cpp instr.cpp printer.cpp c_printer.cpp \
		code_gen.cpp contexts.cpp init.cpp
#HDRS =		$(NWHDRS) c_printer_defs.h
HDRS =		$(CPPS:.cpp=.h) suifvm.h c_printer_defs.h
OBJS =		$(CPPS:.cpp=.o)

LIBS +=		-lmachine

#NWHDRS =	$(CPPS:.cpp=.h) suifvm.h
#NWDOC =		$(TARGET_LIB)
#NWSRCS =	doc-intro.nw $(NWHDRS:=.nw) doc-end.nw
#NWDICT =	doc-end.nw

#DEBRIS =	$(NWDOC).tex

include $(MACHSUIFHOME)/Makefile.common
