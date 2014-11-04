# Makefile for extenders's guide directory

NWDOC =		opi-extenders
DOCS =		$(NWDOC).ps

NWSRCS =	opi-extenders.nw
NWDICT :=	$(addprefix $(MACHSUIFHOME)/machine/, \
		  instr.cpp.nw opnd.h.nw opnd.cpp.nw note.h.nw contexts.h.nw) \
		$(addprefix $(MACHSUIFHOME)/alpha/, \
		  contexts.h.nw) \
		$(addprefix $(MACHSUIFHOME)/x86_ppro/, \
		  contexts.h.nw contexts.cpp.nw opcodes.h.nw printer.h.nw \
		  init.cpp.nw) \
		$(addprefix $(MACHSUIFHOME)/bvd/, bvd.h.nw)


DEBRIS =	*~ $(addprefix $(NWDOC)., tex aux log toc dvi)

include $(MACHSUIFHOME)/Makefile.common

zap: clean
	rm -f $(DOCS) $(NWDOC).tex
