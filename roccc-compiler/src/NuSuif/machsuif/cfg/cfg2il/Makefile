PASS =		cfg2il

OBJS =		cfg2il.o suif_pass.o
MAIN_OBJ =	suif_main.o
CPPS =		$(OBJS:.o=.cpp) $(MAIN_OBJ:.o=.cpp)
HDRS =		$(OBJS:.o=.h)

#NWHDRS =
#NWCPPS =	cfg2il.cpp

LIBS =		-lmachine -lcfg

include $(MACHSUIFHOME)/Makefile.common
