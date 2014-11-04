SUIF_INTERNAL = defined
HEADERS = symbol_utils.h expression_utils.h array_utils.h \
	cloning_utils.h value_block_utils.h type_utils.h \
	trash_utils.h type_pair_stack.h \
	print_utils.h dcast.h semantic_helper.h \
	simple_module.h time_utils.h fold_table.h node_builder.h \
	code_cloner.h \
	enum.h statement_utils.h small_set.h object_set_annote.h \
        annote_utils.h

SRCS = init.cpp \
        symbol_utils.cpp expression_utils.cpp array_utils.cpp \
	cloning_utils.cpp value_block_utils.cpp type_utils.cpp \
	trash_utils.cpp type_pair_stack.cpp \
	print_utils.cpp semantic_helper.cpp \
	simple_module.cpp time_utils.cpp fold_table.cpp node_builder.cpp \
	code_cloner.cpp statement_utils.cpp standard_main.cpp \
        annote_utils.cpp

OBJ_FILES = init.o \
        symbol_utils.o expression_utils.o array_utils.o \
	cloning_utils.o value_block_utils.o type_utils.o \
	trash_utils.o type_pair_stack.o \
	print_utils.o semantic_helper.o \
	simple_module.o time_utils.o fold_table.o node_builder.o \
	code_cloner.o statement_utils.o annote_utils.o

LIBS += -lsuifnodes -lsuifkernel -liokernel -lbasicnodes -lcommon \
	-ltypebuilder -lsuifcloning

TARGET_LIB = utils

PASS_DRIVER = dummy_utils
MAIN_OBJ = standard_main.o


include $(NCIHOME)/Makefile.std
