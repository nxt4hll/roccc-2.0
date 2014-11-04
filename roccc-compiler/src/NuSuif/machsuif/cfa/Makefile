TARGET_LIB = cfa

CPPS :=		dom.cpp loop.cpp cfa.cpp
HDRS =		$(CPPS:.cpp=.h)
OBJS =		$(CPPS:.cpp=.o)
LIBS =		-lmachine -lcfg

#NWHDRS =	$(HDRS)
#NWDOC =		$(TARGET_LIB)
#NWSRCS =	doc-intro.nw $(NWHDRS:=.nw) doc-end.nw
#NWDICT =	doc-end.nw

#DEBRIS =	$(NWDOC).tex

include $(MACHSUIFHOME)/Makefile.common
