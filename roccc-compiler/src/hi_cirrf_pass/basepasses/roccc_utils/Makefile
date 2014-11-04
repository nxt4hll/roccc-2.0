SUIF_INTERNAL = defined

SRCS = annote_utils.cpp IR_utils.cpp loop_utils.cpp type_utils.cpp \
symbol_utils.cpp print_utils.cpp data_dependence_utils.cpp string_utils.cpp \
control_flow_utils.cpp bit_vector_annote_utils.cpp \
bit_vector_data_flow_utils.cpp main.cpp bitVector2.cpp warning_utils.cpp innermostUtilities.cpp miscUtilities.cpp functionUtilities.cpp statementUtilities.cpp variableUtilities.cpp identificationUtilities.cpp equivalentUtilities.cpp usesAndDefsUtilities.cpp replaceUtilities.cpp statementListUtilities.cpp arrayUtilities.cpp typeUtilities.cpp loopUtilities.cpp parameterUtilities.cpp verifier.cpp

OBJ_FILES = annote_utils.o IR_utils.o loop_utils.o type_utils.o \
symbol_utils.o print_utils.o data_dependence_utils.o string_utils.o \
control_flow_utils.o bit_vector_annote_utils.o \
bit_vector_data_flow_utils.o main.o bitVector2.o warning_utils.o innermostUtilities.o miscUtilities.o functionUtilities.o statementUtilities.o variableUtilities.o identificationUtilities.o equivalentUtilities.o usesAndDefsUtilities.o replaceUtilities.o statementListUtilities.o arrayUtilities.o typeUtilities.o loopUtilities.o parameterUtilities.o verifier.o

HEADERS = annote_utils.h list_utils.h IR_utils.h loop_utils.h \
type_utils.h symbol_utils.h print_utils.h data_dependence_utils.h \
string_utils.h control_flow_utils.h bit_vector_annote_utils.h \
bit_vector_data_flow_utils.h bitVector2.h warning_utils.h warning_level.h roccc2.0_utils.h functionUtilities.h statementUtilities.h variableUtilities.h identificationUtilities.h equivalentUtilities.h usesAndDefsUtilities.h replaceUtilities.h statementListUtilities.h innermostUtilities.h miscUtilities.h arrayUtilities.h typeUtilities.h loopUtilities.h parameterUtilities.h verifier.h
 
LIBS += -lsuifkernel -liokernel -lcommon -lutils -ltransforms
LIBS += -lcfenodes -lsuifnodes -lbasicnodes -lsuifpasses 
LIBS += -ltypebuilder -lcfeutils -lsuifcloning -lsuiflocation
LIBS += -lroccc_extra_types

TARGET_LIB = roccc_utils

PASS_DRIVER =  do_roccc_utils
MAIN_OBJ = main.o

#EXTRA_CXXFLAGS += -I./.. 

include $(NCIHOME)/Makefile.std
