PASS_DRIVER = do_alias_lattice
MAIN_OBJ = standard_main.o

OBJ_FILES = bit_lattice.o \
	unary.o \
	nary.o \
	interval_lattice.o \
	indexed_lattice.o \
	hindexed_lattice.o \
	lattice_utils.o \
	init_lattice.o \
	smash_lattice.o \
	const_lattice.o

SRCS = bit_lattice.cpp \
	unary.cpp \
	nary.cpp \
	const_lattice.cpp \
	nary_bit_lattice.cpp \
	interval_lattice.cpp \
	indexed_lattice.cpp \
	hindexed_lattice.cpp \
	lattice_utils.cpp \
	init_lattice.cpp \
	smash_lattice.cpp \
	standard_main.cpp


HEADERS = bit_lattice.h \
	unary.h \
	nary.h \
	const_lattice.h \
	nary_bit_lattice.h \
	interval_lattice.h \
	lattice_utils_forwarders.h \
	indexed_lattice.h \
	hindexed_lattice.h \
	init_lattice.h \
	smash_lattice.h

TARGET_LIB = lattice_utils

LIBS += -lbit_vector -lsuifkernel -lcommon

include $(NCIHOME)/Makefile.std
