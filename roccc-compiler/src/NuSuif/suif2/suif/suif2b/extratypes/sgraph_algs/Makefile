PASS_DRIVER =	do_sgraph_algs
MAIN_OBJ =	standard_main.o

TARGET_LIB = 	sgraph_algs

# The primary interfaces here are:
# sgraph and ngraph

HEADERS =	sgraph_algs.h cfgraph_algs.h

SRCS =		sgraph_algs.cpp cfgraph_algs.cpp standard_main.cpp
OBJ_FILES =	sgraph_algs.o   cfgraph_algs.o

LIBS += -lsuifkernel -liokernel -lcommon
LIBS += -lion -lsgraph -lbit_vector

#EXTRA_CXXFLAGS += -I../../extratypes
#EXTRA_CXXFLAGS += -I..
EXTRA_CXXFLAGS += -DINLINE_ALL_TEMPLATES

#EXTRA_CXXFLAGS += -Weffc++


include $(NCIHOME)/Makefile.std

