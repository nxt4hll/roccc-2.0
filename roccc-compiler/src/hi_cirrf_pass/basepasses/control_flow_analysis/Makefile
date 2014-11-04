SUIF_INTERNAL = defined

SRCS = control_flow_analysis.cpp solve_pass.cpp loop_info_pass.cpp main.cpp

OBJ_FILES = control_flow_analysis.o solve_pass.o loop_info_pass.o main.o

HEADERS =

LIBS += -lsuifkernel -liokernel -lcommon -lutils -ltransforms
LIBS += -lcfenodes -lsuifnodes -lbasicnodes -lsuifpasses 
LIBS += -ltypebuilder -lcfeutils -lsuifcloning -lsuiflocation
LIBS += -lroccc_utils

TARGET_LIB = control_flow_analysis

PASS_DRIVER =  do_control_flow_analysis
MAIN_OBJ = main.o


#EXTRA_CXXFLAGS += -I./.. 

include $(NCIHOME)/Makefile.std
