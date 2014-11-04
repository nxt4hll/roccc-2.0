SUIF_INTERNAL = defined
SRCS = expression_dismantlers.cpp statement_dismantlers.cpp transforms.cpp main.cpp procedure_walker_utilities.cpp symbol_walkers.cpp region_passes.cpp padding.cpp function_dismantlers.cpp symbol_transforms.cpp array_dismantlers.cpp

HEADERS = region_passes.h statement_dismantlers.h procedure_walker_utilities.h symbol_walkers.h function_dismantlers.h symbol_transforms.h array_dismantlers.h

OBJ_FILES = expression_dismantlers.o statement_dismantlers.o transforms.o procedure_walker_utilities.o symbol_walkers.o region_passes.o padding.o function_dismantlers.o symbol_transforms.o array_dismantlers.o

LIBS += -lsuifnodes -lsuifkernel -liokernel -lbasicnodes -lcfenodes -lcommon \
        -lsuifpasses -ltypebuilder -lcfeutils -lutils  -lsuifcloning -lsuiflocation \
#        -lsgraph.lib -lsgraph_algs.lib -lecr_alias.lib -lion.lib -lbit_vector.lib

TARGET_LIB = transforms

#EXTRA_CXXFLAGS += -I../basesuif/suifpasses
#EXTRA_CXXFLAGS += -I../basesuif/suifnodes
#EXTRA_CXXFLAGS += -I../basetypes
#EXTRA_CXXFLAGS += -I./..

PASS_DRIVER = do_transforms
MAIN_OBJ = main.o

include $(NCIHOME)/Makefile.std