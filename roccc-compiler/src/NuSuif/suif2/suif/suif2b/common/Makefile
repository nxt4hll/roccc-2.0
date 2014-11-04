#SUIF_INTERNAL = defined
OBJ_FILES = formatted.o \
	MString.o \
	text_util.o \
	suif_map.o \
	suif_hash_map.o \
	lstring.o \
	hash_table.o \
	i_integer.o \
	sparse_vector.o \
	suif_gc_obj.o \
	suif_gc_map.o

SRCS = MString.cpp \
	hash_table.cpp \
	suif_hash_map.cpp \
	formatted.cpp \
	i_integer.cpp \
	lstring.cpp \
	suif_map.cpp \
	text_util.cpp \
	sparse_vector.cpp \
	suif_gc_obj.cpp \
	suif_gc_map.cpp

HEADERS = MString.h \
	simple_stack.h \
	sparse_vector.h \
	common_forwarders.h \
	suif_copyright.h \
	formatted.h \
	suif_hash_map.h \
	hash_table.h \
	suif_indexed_list.h \
	i_integer.h \
	suif_list.h \
	suif_map.h \
	lstring.h \
	suif_vector.h \
	machine_dependent.h \
	text_util.h \
	system_specific.h \
	ref_counter.h \
	suif_gc_obj.h \
	suif_gc_map.h

TARGET_LIB = common

#	We build common statically since there seem to be linkage problems
#	with a dynamic version and, in any case, these are small enough and
#	common enough for dynamic calls to impose too much overhead

#ifneq (,$(BUILD_ALL_SHARED))
#STATIC_LIBS =
#NO_DYNAMIC_LIBS =
#else
#STATIC_LIBS = true
#NO_DYNAMIC_LIBS = true
#endif

include $(NCIHOME)/Makefile.std

