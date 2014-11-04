# Makefile for OPI user's guide directory

NWDOC =		opi-users
DOCS =		$(NWDOC).ps

NWSRCS =	opi-users.nw appA.nw appB.nw
NWDICT =	$(MACHSUIFHOME)/peep/suif_pass.cpp.nw

DEBRIS =	*~ $(addprefix $(NWDOC).,tex aux log toc dvi)

include $(MACHSUIFHOME)/Makefile.common

zap: clean
	rm -f $(DOCS) $(NWDOC).tex
