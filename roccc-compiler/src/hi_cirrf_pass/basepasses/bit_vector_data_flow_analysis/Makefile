SUIF_INTERNAL = defined

SRCS = bit_vector_dataflow_analysis.cpp \
solve_available_expressions_pass.cpp availability_chain_builder_pass.cpp main.cpp solve_pass2.cpp ud_du_chain_builder_pass2.cpp solve_feedback_variables_pass_2.0.cpp solve_feedback_variables_pass_2.0_new.cpp solve_feedback_calls.cpp

OBJ_FILES = bit_vector_dataflow_analysis.o \
solve_available_expressions_pass.o availability_chain_builder_pass.o main.o solve_pass2.o ud_du_chain_builder_pass2.o solve_feedback_variables_pass_2.0.o solve_feedback_variables_pass_2.0_new.o solve_feedback_calls.o

HEADERS =

LIBS += -lsuifkernel -liokernel -lcommon -lutils -ltransforms
LIBS += -lcfenodes -lsuifnodes -lbasicnodes -lsuifpasses 
LIBS += -ltypebuilder -lcfeutils -lsuifcloning -lsuiflocation
LIBS += -lroccc_utils -lroccc_extra_types

TARGET_LIB = bit_vector_dataflow_analysis

PASS_DRIVER =  do_bit_vector_dataflow_analysis
MAIN_OBJ = main.o


#EXTRA_CXXFLAGS += -I./.. 

include $(NCIHOME)/Makefile.std
