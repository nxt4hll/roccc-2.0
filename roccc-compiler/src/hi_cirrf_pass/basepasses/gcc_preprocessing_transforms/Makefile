SUIF_INTERNAL = defined

SRCS = gcc_preprocessing_transforms.cpp main.cpp ForLoopPreprocessing.cpp RobyPreprocessingPass.cpp InitializationPass.cpp

OBJ_FILES = gcc_preprocessing_transforms.o main.o ForLoopPreprocessing.o RobyPreprocessingPass.o InitializationPass.o

HEADERS =

LIBS += -lsuifkernel -liokernel -lcommon -lutils -ltransforms
LIBS += -lcfenodes -lsuifnodes -lbasicnodes -lsuifpasses 
LIBS += -ltypebuilder -lcfeutils -lsuifcloning -lsuiflocation
LIBS += -lroccc_utils

TARGET_LIB = gcc_preprocessing_transforms

PASS_DRIVER =  do_preprocessing_transforms
MAIN_OBJ = main.o

#EXTRA_CXXFLAGS += -I./.. 

include $(NCIHOME)/Makefile.std
