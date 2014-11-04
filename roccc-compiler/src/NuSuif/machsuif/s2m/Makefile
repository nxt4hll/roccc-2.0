PASS =		s2m

OBJS =		s2m.o
MAIN_OBJ =	suif_main.o
CPPS =		$(OBJS:.o=.cpp) $(MAIN_OBJ:.o=.cpp)
HDRS =		$(OBJS:.o=.h)

LIBS =		-lmachine -lsuifvm

EXTRA_CFLAGS +=	-DCFE_STORE_VAR_FIXED

include $(MACHSUIFHOME)/Makefile.common
