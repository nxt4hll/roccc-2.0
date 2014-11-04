PASS =		dcessa

OBJS =		dcessa.o suif_pass.o
MAIN_OBJ =	suif_main.o
CPPS =		$(OBJS:.o=.cpp) $(MAIN_OBJ:.o=.cpp)
HDRS =		$(OBJS:.o=.h)

LIBS =		-lmachine -lsuifvm -lcfg -lcfa -lbvd -lssa

#NWCPPS =	dcessa.cpp

include $(MACHSUIFHOME)/Makefile.common
