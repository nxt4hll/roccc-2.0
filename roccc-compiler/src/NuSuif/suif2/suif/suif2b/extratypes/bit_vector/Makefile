PASS_DRIVER = do_bit_vector
MAIN_OBJ = standard_main.o

OBJ_FILES = bit_vector.o bvector.o cross_map.o init.o

SRCS = bit_vector.cpp bvector.cpp cross_map.cpp init.cpp standard_main.cpp

TARGET_LIB = bit_vector

HEADERS = bit_vector_forwarders.h bit_vector.h \
	cross_map.h cross_map_forwarders.h bit_vector_set.h \
	bvector.h cset.h

INCLDIRS += 
LIBS += -lion -lcommon -liokernel -lsuifkernel

include $(NCIHOME)/Makefile.std

