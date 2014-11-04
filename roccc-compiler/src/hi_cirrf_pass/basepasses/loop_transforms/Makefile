SUIF_INTERNAL = defined

SRCS = loop_transforms.cpp unroll_pass.cpp \
invariant_code_motion_pass.cpp fuse_pass.cpp peel_pass.cpp normalize_pass.cpp \
forward_substitution_pass.cpp induction_var_substitution_pass.cpp \
unroll_constant_bounds_pass.cpp unswitch_pass.cpp tile_pass.cpp \
interchange_pass.cpp strip_mine_pass.cpp skew_pass.cpp main.cpp unroll_pass2.cpp cleanup_unrolled_calls.cpp handle_call_statements.cpp handle_copy_statements.cpp fullUnroll.cpp pseudoSSA.cc

OBJ_FILES = loop_transforms.o unroll_pass.o \
invariant_code_motion_pass.o fuse_pass.o peel_pass.o normalize_pass.o \
forward_substitution_pass.o induction_var_substitution_pass.o \
unroll_constant_bounds_pass.o unswitch_pass.o tile_pass.o \
interchange_pass.o strip_mine_pass.o skew_pass.o main.o unroll_pass2.o cleanup_unrolled_calls.o handle_call_statements.o handle_copy_statements.o fullUnroll.o pseudoSSA.o

HEADERS =

LIBS += -lsuifkernel -liokernel -lcommon -lutils -ltransforms
LIBS += -lcfenodes -lsuifnodes -lbasicnodes -lsuifpasses 
LIBS += -ltypebuilder -lcfeutils -lsuifcloning -lsuiflocation
LIBS += -lroccc_utils
LIBS += -lcontrol_flow_analysis -lbit_vector_dataflow_analysis
LIBS += -lglobal_transforms

TARGET_LIB = loop_transforms

PASS_DRIVER = do_loop_transforms
MAIN_OBJ = main.o


#EXTRA_CXXFLAGS += -I./.. 

include $(NCIHOME)/Makefile.std
