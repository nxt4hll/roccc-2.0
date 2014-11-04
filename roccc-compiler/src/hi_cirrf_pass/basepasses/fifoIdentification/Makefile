SUIF_INTERNAL = defined

SRCS = fifoIdentification.cc fifoId.cc main.cc fifoIdentificationSystolicArray.cc transform_unrolled_arrays.cpp

OBJ_FILES = fifoIdentification.o fifoId.o main.o fifoIdentificationSystolicArray.o transform_unrolled_arrays.o

HEADERS =

LIBS += -lsuifkernel -liokernel -lcommon -lutils -ltransforms
LIBS += -lcfenodes -lsuifnodes -lbasicnodes -lsuifpasses 
LIBS += -ltypebuilder -lcfeutils -lsuifcloning -lsuiflocation
LIBS += -lroccc_utils

TARGET_LIB =fifoIdentification

PASS_DRIVER =  do_fifoIdentification
MAIN_OBJ = main.o


#EXTRA_CXXFLAGS += -I./.. 

include $(NCIHOME)/Makefile.std
