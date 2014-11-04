PASS =		m2a

OBJS =		m2a.o suif_pass.o
MAIN_OBJ =	suif_main.o
CPPS =		$(OBJS:.o=.cpp) $(MAIN_OBJ:.o=.cpp)
HDRS =		$(OBJS:.o=.h)

LIBS =		-lmachine -lsuifvm

include $(MACHSUIFHOME)/Makefile.common
