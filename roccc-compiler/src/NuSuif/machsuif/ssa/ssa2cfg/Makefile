PASS =		ssa2cfg

OBJS =		ssa2cfg.o suif_pass.o
MAIN_OBJ =	suif_main.o
CPPS =		$(OBJS:.o=.cpp) $(MAIN_OBJ:.o=.cpp)
HDRS =		$(OBJS:.o=.h)

LIBS =		-lmachine -lcfg -lssa

include $(MACHSUIFHOME)/Makefile.common
