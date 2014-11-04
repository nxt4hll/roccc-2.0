TARGET_LIB =	machine

HOOF =		machine_ir

PAIRS =		instr.h opnd.h types.h opcodes.h reg_info.h \
		printer.h c_printer.h code_fin.h nat_set.h note.h problems.h \
		util.h contexts.h substrate.h init.h
CPPS =		$(PAIRS:.h=.cpp)
HDRS =		$(PAIRS:.cpp=.h) machine.h pass.h copyright.h
OBJS =		$(CPPS:.cpp=.o)

#NWHOOF =	machine_ir.hoof
#NWHDRS =	$(filter-out copyright.h,$(HDRS))
#NWCPPS =	instr.cpp opnd.cpp
#NWDOC =		$(TARGET_LIB)
#NWSRCS =	doc-intro.nw $(NWHDRS:=.nw) machine_ir.hoof.nw doc-end.nw
#NWDICT =	doc-end.nw

#DEBRIS =	$(NWDOC).tex

include $(MACHSUIFHOME)/Makefile.common

ifneq (,$(USE_DMALLOC))
LIBS +=		 -ldmallocxx dmallocc.o 
endif
