SUIF_INTERNAL = defined

SRCS = bit_vector_map.cpp bit_vector.cpp array_info.cpp main.cpp 

OBJ_FILES = bit_vector_map.o bit_vector.o array_info.o main.o

HEADERS = bit_vector_map.h bit_vector.h array_info.h list_utils.h
 
LIBS += -lsuifkernel -liokernel -lcommon -lutils -ltransforms
LIBS += -lcfenodes -lsuifnodes -lbasicnodes -lsuifpasses 
LIBS += -ltypebuilder -lcfeutils -lsuifcloning -lsuiflocation
# LIBS += -lroccc_utils

TARGET_LIB = roccc_extra_types

PASS_DRIVER =  do_roccc_extra_types
MAIN_OBJ = main.o

#EXTRA_CXXFLAGS += -I./.. 

include $(NCIHOME)/Makefile.std
