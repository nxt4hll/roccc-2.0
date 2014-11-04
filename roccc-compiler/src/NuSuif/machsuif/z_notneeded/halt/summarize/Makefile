PASS =		summarize

OBJS =		summarize.o suif_pass.o
MAIN_OBJ =	suif_main.o
CPPS =		$(OBJS:.o=.cpp) $(MAIN_OBJ:.o=.cpp)
HDRS =		$(OBJS:.o=.h)

LIBS =		-lmachine -lcfg -lhalt

include $(MACHSUIFHOME)/Makefile.common
