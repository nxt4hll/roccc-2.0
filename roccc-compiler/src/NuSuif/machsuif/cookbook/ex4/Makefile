PASS =		ex4

OBJS =		ex4.o suif_pass.o
MAIN_OBJ =	suif_main.o
CPPS =		$(OBJS:.o=.cpp) $(MAIN_OBJ:.o=.cpp)

NWDICT =	$(addprefix $(MACHSUIFHOME)/cookbook/, \
		  ex3/ex3.cpp.nw)

NWHDRS =
NWCPPS =	ex4.cpp

LIBS =		-lmachine

include $(MACHSUIFHOME)/Makefile.common
