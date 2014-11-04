TARGET_LIB =	cfg

# Subdirectories to visit
PACKAGES =	cfg2il il2cfg print_cfg

HOOF =		cfg_ir
HOOF_INCL =	-I$(MACHSUIFHOME)

CPPS :=		graph.cpp node.cpp util.cpp init.cpp
HDRS =		$(CPPS:.cpp=.h) cfg.h
OBJS =		$(CPPS:.cpp=.o)
LIBS =		-lmachine -lsuifvm

#NWHOOF =	cfg_ir.hoof
#NWHDRS =	$(HDRS)
#NWDOC =		$(TARGET_LIB)
#NWSRCS =	doc-intro.nw $(NWHDRS:=.nw) cfg_ir.hoof.nw doc-end.nw
#NWDICT =	doc-end.nw

#DEBRIS =	$(NWDOC).tex

include $(MACHSUIFHOME)/Makefile.common
