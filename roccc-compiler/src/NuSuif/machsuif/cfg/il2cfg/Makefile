PASS =		il2cfg

OBJS =		il2cfg.o suif_pass.o
MAIN_OBJ =	suif_main.o
CPPS =		$(OBJS:.o=.cpp) $(MAIN_OBJ:.o=.cpp)
HDRS =		$(OBJS:.o=.h)

#NWHDRS =
#NWCPPS =	il2cfg.cpp

LIBS =		-lmachine -lcfg

include $(MACHSUIFHOME)/Makefile.common
