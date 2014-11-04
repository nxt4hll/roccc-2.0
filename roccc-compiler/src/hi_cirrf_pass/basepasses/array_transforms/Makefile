SUIF_INTERNAL = defined

SRCS = array_transforms.cpp raw_elimination_pass.cpp scalar_replacement_pass.cpp \
renaming_pass.cpp feedback_load_elimination_pass.cpp systolic_array_generation_pass.cpp \
constant_propagation_pass.cpp main.cpp scalar_replacement_pass2.cpp constantArrayPropagationPass.cpp lutDetectionPass.cpp lutTransformationPass.cpp

OBJ_FILES = array_transforms.o raw_elimination_pass.o scalar_replacement_pass.o \
renaming_pass.o feedback_load_elimination_pass.o systolic_array_generation_pass.o \
constant_propagation_pass.o main.o scalar_replacement_pass2.o constantArrayPropagationPass.o lutDetectionPass.o lutTransformationPass.o

HEADERS =

LIBS += -lsuifkernel -liokernel -lcommon -lutils -ltransforms
LIBS += -lcfenodes -lsuifnodes -lbasicnodes -lsuifpasses 
LIBS += -ltypebuilder -lcfeutils -lsuifcloning -lsuiflocation
LIBS += -lroccc_utils -lroccc_extra_types
LIBS += -lroccc_extra_types

TARGET_LIB = array_transforms

PASS_DRIVER = do_array_transforms
MAIN_OBJ = main.o


#EXTRA_CXXFLAGS += -I./.. 

include $(NCIHOME)/Makefile.std
