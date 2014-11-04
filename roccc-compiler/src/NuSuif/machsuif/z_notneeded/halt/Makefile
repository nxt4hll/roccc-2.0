TARGET_LIB =	halt

# Subdirectories to visit
PACKAGES =	label instrument summarize

CPPS =		kinds.cpp recipe.cpp note.cpp contexts.cpp init.cpp
HDRS =		$(CPPS:.cpp=.h) halt.h
OBJS =		$(CPPS:.cpp=.o)
LIBS =		-lmachine -lcfg

#NWHDRS =	kinds.h recipe.h note.h contexts.h init.h
#NWDOC =		$(TARGET_LIB)
#NWSRCS =	doc-intro.nw doc-sec2.nw doc-sec3.nw doc-sec4.nw doc-end.nw
#NWDICT =	$(NWHDRS:=.nw) \
#		    $(addprefix $(MACHSUIFHOME)/x86_halt/, \
#				contexts.h.nw recipe.cpp.nw) \
#		    $(addprefix $(MACHSUIFHOME)/x86_halt_blpp/, \
#				recipe.cpp.nw)
#DEBRIS =	$(NWDOC).tex

include $(MACHSUIFHOME)/Makefile.common
