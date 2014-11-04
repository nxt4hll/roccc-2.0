PASS =		raga

OBJS =		raga.o color.o libra.o suif_pass.o
MAIN_OBJ =	suif_main.o
CPPS =		$(OBJS:.o=.cpp) $(MAIN_OBJ:.o=.cpp)
HDRS =		raga.h libra.h suif_pass.h

LIBS =		-lmachine -lcfg -lcfa -lbvd

include $(MACHSUIFHOME)/Makefile.common

ifdef NEIGHBOR_CENSUS
EXTRA_CXXFLAGS += -DNEIGHBOR_CENSUS
endif

ifdef IG_INCLUDES_HR
EXTRA_CXXFLAGS += -DIG_INCLUDES_HR
endif
