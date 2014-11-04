# The primary interfaces is tos.h

SRCS = adlist_tos.cpp array_tos.cpp cdlist_tos.cpp dlist_tos.cpp \
	referenced_item.cpp slist_tos.cpp special_ro_tos.cpp tos.cpp \
	\
	alist_index.cpp hash_index.cpp tree_string_index.cpp ts_ptr_index.cpp \
	index.cpp brick.cpp

HEADERS = adlist_tos.h array_tos.h cdlist_tos.h const_cast.h \
	dlist_tos.h referenced_item.h slist_tos.h special_ro_tos.h tos.h \
	\
	alist_index.h hash_index.h tree_string_index.h ts_ptr_index.h \
	index.h brick.h zero.h
OBJ_FILES = adlist_tos.o array_tos.o cdlist_tos.o dlist_tos.o \
	referenced_item.o slist_tos.o special_ro_tos.o tos.o \
	\
	alist_index.o hash_index.o tree_string_index.o ts_ptr_index.o \
	index.o

LIBS += -lcommon
LIBS += -lion


TARGET_LIB = tos

#EXTRA_CXXFLAGS += -I../../basetypes
#EXTRA_CXXFLAGS += -I../../basesuif
#EXTRA_CXXFLAGS += -I..
EXTRA_CXXFLAGS += -DINLINE_ALL_TEMPLATES

include $(NCIHOME)/Makefile.std
