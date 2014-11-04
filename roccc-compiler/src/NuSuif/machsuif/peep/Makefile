PASS =		peep

OBJS =		peep.o suif_pass.o
MAIN_OBJ =	suif_main.o
CPPS =		$(OBJS:.o=.cpp) $(MAIN_OBJ:.o=.cpp)
HDRS =		$(OBJS:.o=.h)

#NWCPPS =	suif_pass.cpp

LIBS =		-lmachine -lcfg -lbvd

include $(MACHSUIFHOME)/Makefile.common
