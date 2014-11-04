SUIF_INTERNAL = defined

SRCS = preprocessing_transforms.cpp dowhile_to_while_transform_pass.cpp \
hw_sw_boundary_mark_pass.cpp \
switch_case_to_if_stmts_pass.cpp \
dfa_state_table_expansion_pass.cpp main.cpp

OBJ_FILES = preprocessing_transforms.o dowhile_to_while_transform_pass.o \
hw_sw_boundary_mark_pass.o \
switch_case_to_if_stmts_pass.o \
dfa_state_table_expansion_pass.o main.o

HEADERS =

LIBS += -lsuifkernel -liokernel -lcommon -lutils -ltransforms
LIBS += -lcfenodes -lsuifnodes -lbasicnodes -lsuifpasses 
LIBS += -ltypebuilder -lcfeutils -lsuifcloning -lsuiflocation
LIBS += -lroccc_utils

TARGET_LIB = preprocessing_transforms

PASS_DRIVER =  do_preprocessing_transforms
MAIN_OBJ = main.o

#EXTRA_CXXFLAGS += -I./.. 

include $(NCIHOME)/Makefile.std
