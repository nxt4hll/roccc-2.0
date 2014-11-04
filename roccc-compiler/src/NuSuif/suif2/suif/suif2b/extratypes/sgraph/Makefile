PASS_DRIVER =	do_sgraph
MAIN_OBJ =	standard_main.o

TARGET_LIB = 	sgraph

# The primary interfaces here are:
# sgraph and ngraph

# These provide the interfaces
# ngraph.cpp is included because ngraph.h includes it.
HEADERS =	sgraph.h sgraph_iter.h sgraph_forwarders.h \
		sgraph_list.h sgraph_bit.h sgraph_bit_iter.h \
		sgraph_iter_impl.h sgraph_bit_subgraph.h \
		ngraph.h ngraph_base.h


# implementations are sgraph_bit or sgraph_list
SRCS =		sgraph.cpp sgraph_iter.cpp \
		sgraph_bit.cpp sgraph_list.cpp \
		sgraph_bit_iter.cpp \
		sgraph_bit_subgraph.cpp \
		sgraph_iter_impl.cpp \
		standard_main.cpp \
		ngraph.cpp ngraph_base.cpp

OBJ_FILES =	$(SRCS:.cpp=.o)

LIBS += -lsuifkernel -liokernel -lcommon
LIBS += -lion -lbit_vector

EXTRA_CXXFLAGS += -DINLINE_ALL_TEMPLATES

#EXTRA_CXXFLAGS += -Weffc++


include $(NCIHOME)/Makefile.std

