PASS =		make_2opnd

OBJS =		make_2opnd.o suif_pass.o
MAIN_OBJ =	suif_main.o
CPPS =		$(OBJS:.o=.cpp) $(MAIN_OBJ:.o=.cpp)
HDRS =		$(OBJS:.o=.h)

LIBS =		-lmachine -lcfg

include $(MACHSUIFHOME)/Makefile.common
