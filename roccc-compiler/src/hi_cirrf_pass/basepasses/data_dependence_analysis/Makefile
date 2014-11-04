SUIF_INTERNAL = defined

SRCS = data_dependence_analysis.cpp preprocessing_pass.cpp \
main.cpp

OBJ_FILES = data_dependence_analysis.o preprocessing_pass.o \
main.o

HEADERS =

LIBS += -lsuifkernel -liokernel -lcommon -lutils -ltransforms
LIBS += -lcfenodes -lsuifnodes -lbasicnodes -lsuifpasses 
LIBS += -ltypebuilder -lcfeutils -lsuifcloning -lsuiflocation
LIBS += -lroccc_utils -lroccc_extra_types
LIBS += -lroccc_extra_types

TARGET_LIB = data_dependence_analysis

PASS_DRIVER =  do_data_dependence_analysis
MAIN_OBJ = main.o


#EXTRA_CXXFLAGS += -I./.. 

include $(NCIHOME)/Makefile.std
