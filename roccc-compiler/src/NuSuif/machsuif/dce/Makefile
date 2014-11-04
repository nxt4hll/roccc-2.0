PASS =		dce

OBJS =		dce.o suif_pass.o
MAIN_OBJ =	suif_main.o
CPPS =		$(OBJS:.o=.cpp) $(MAIN_OBJ:.o=.cpp)
HDRS =		$(OBJS:.o=.h)

LIBS =		-lmachine -lcfg -lcfa -lbvd

include $(MACHSUIFHOME)/Makefile.common
