# Makefile for Machine-SUIF cookbook directory

# Subdirectories to visit
PACKAGES =	ex1 ex2 ex3 ex4 ex5

# Local document
NWDOC =		cookbook
NWSRCS =	cookbook.nw \
		ex1.nw ex2.nw ex3.nw ex4.nw ex5.nw ex6.nw \
		cookbook-end.nw
DOCS =		$(NWDOC).ps $(NWDOC).html

NWDICT =	ex1/ex1.h.nw ex1/ex1.cpp.nw \
		ex2/ex2.cpp.nw \
		ex3/ex3.h.nw ex3/ex3.cpp.nw \
		ex4/ex4.cpp.nw \
		ex5/ex5.cpp.nw \
		$(addprefix $(MACHSUIFHOME)/cfg/, \
		  cfg2il/cfg2il.cpp.nw il2cfg/il2cfg.cpp.nw)

include $(MACHSUIFHOME)/Makefile.common

zap: clean
	rm -f $(DOCS) $(NWDOC).tex
