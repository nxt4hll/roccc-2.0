SUIF_INTERNAL = defined

SRCS = utility_transforms.cpp flatten_statement_lists_pass.cpp \
flatten_scope_statements_pass.cpp strip_annotes_pass.cpp \
mem_dp_boundary_mark_pass.cpp remove_loop_label_loc_stmts_pass.cpp \
bit_resize_datapath_constants_pass.cpp main.cpp normalize_statement_lists.cpp

OBJ_FILES = utility_transforms.o flatten_statement_lists_pass.o \
flatten_scope_statements_pass.o strip_annotes_pass.o \
mem_dp_boundary_mark_pass.o remove_loop_label_loc_stmts_pass.o \
bit_resize_datapath_constants_pass.o main.o normalize_statement_lists.o

HEADERS =

LIBS += -lsuifkernel -liokernel -lcommon -lutils -ltransforms
LIBS += -lcfenodes -lsuifnodes -lbasicnodes -lsuifpasses 
LIBS += -ltypebuilder -lcfeutils -lsuifcloning -lsuiflocation
LIBS += -lroccc_utils

TARGET_LIB = utility_transforms

PASS_DRIVER =  do_utility_transforms
MAIN_OBJ = main.o

#EXTRA_CXXFLAGS += -I./.. 

include $(NCIHOME)/Makefile.std
