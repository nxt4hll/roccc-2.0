SUIF_INTERNAL = defined

SRCS = global_transforms.cpp constant_propagation_n_folding_pass.cpp \
copy_propagation_pass.cpp cse_pass.cpp dead_code_elimination_pass.cpp \
unrchable_code_elimination_pass.cpp div_by_const_elimination_pass2.cpp \
reverse_copy_propagation_pass.cpp miniScalarReplacement.cpp combine_summation_pass.cpp \
mult_by_const_elimination_pass.cpp if_conversion_pass2.cpp cleanup_store_statements.cpp cleanup_load_pass.cpp miniConstantPropagation.cpp \
scalar_renaming_pass.cpp code_hoisting_pass.cpp code_sinking_pass.cpp \
main.cpp scalar_renaming_pass2.cpp copy_propagation_pass2.cpp leftoverRemoval.cpp temporal_cse_pass2.cpp output_identification_pass.cpp predication_pass.cpp constant_propagation_n_folding_pass2.cpp transform_systems_to_modules.cpp remove_unused_variables.cpp pointer_conversion.cpp evalTransform.cpp redundancy_pass.cpp mark_redundant.cpp redundant_to_redundant.cpp redundant_cleanup.cpp available_elimination.cpp cleanup_boolsels.cpp summation_pass.cpp inlining_pass.cpp reference_cleanup_pass.cpp inlineAllModules.cpp identify_parameters_pass.cpp remove_unsupported_statements.cpp
# temporal_cse_pass.cpp main.cpp 


OBJ_FILES = global_transforms.o constant_propagation_n_folding_pass.o \
copy_propagation_pass.o cse_pass.o dead_code_elimination_pass.o \
unrchable_code_elimination_pass.o div_by_const_elimination_pass2.o \
reverse_copy_propagation_pass.o miniScalarReplacement.o combine_summation_pass.o \
mult_by_const_elimination_pass.o if_conversion_pass2.o cleanup_store_statements.o cleanup_load_pass.o miniConstantPropagation.o \
scalar_renaming_pass.o code_hoisting_pass.o code_sinking_pass.o \
main.o scalar_renaming_pass2.o  copy_propagation_pass2.o leftoverRemoval.o temporal_cse_pass2.o output_identification_pass.o predication_pass.o constant_propagation_n_folding_pass2.o transform_systems_to_modules.o remove_unused_variables.o pointer_conversion.o evalTransform.o redundancy_pass.o mark_redundant.o redundant_to_redundant.o redundant_cleanup.o available_elimination.o cleanup_boolsels.o summation_pass.o inlining_pass.o reference_cleanup_pass.o inlineAllModules.o identify_parameters_pass.o remove_unsupported_statements.o
# temporal_cse_pass.o main.o

HEADERS =

LIBS += -lsuifkernel -liokernel -lcommon -lutils -ltransforms
LIBS += -lcfenodes -lsuifnodes -lbasicnodes -lsuifpasses 
LIBS += -ltypebuilder -lcfeutils -lsuifcloning -lsuiflocation
LIBS += -lroccc_utils -lroccc_extra_types
LIBS += -lroccc_extra_types
LIBS += -lgcc_preprocessing_transforms

TARGET_LIB = global_transforms

PASS_DRIVER =  do_global_transforms
MAIN_OBJ = main.o


#EXTRA_CXXFLAGS += -I./.. 

include $(NCIHOME)/Makefile.std
