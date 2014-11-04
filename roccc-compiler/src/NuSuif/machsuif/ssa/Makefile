TARGET_LIB =	ssa

# Subdirectories to visit
PACKAGES =	cfg2ssa ssa2cfg

HOOF =		ssa_ir
HOOF_INCL =	-I$(MACHSUIFHOME)

OBJS =		opi.o ssa_core.o ssa_core_ptr.o init.o
CPPS =		$(OBJS:.o=.cpp)
HDRS =		$(CPPS:.cpp=.h) ssa.h

LIBS =		-lmachine -lcfg -lcfa -lbvd

#NWDOC =		$(TARGET_LIB)
#NWHDRS =	ssa_core.h opi.h
#NWSRCS =	ssa.nw
#NWDICT = 	ssa_core.h.nw opi.h.nw $(MACHSUIFHOME)/dcessa/dcessa.cpp.nw
#DEBRIS =	$(NWDOC).tex

include $(MACHSUIFHOME)/Makefile.common
