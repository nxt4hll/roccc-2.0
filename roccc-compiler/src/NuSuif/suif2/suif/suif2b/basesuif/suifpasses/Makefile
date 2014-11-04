#SUIF_INTERNAL = defined
OBJ_FILES = passes.o \
drivers.o \
suifpasses.o \
standard_modules.o \
dispatcher.o

SRCS = passes.cpp drivers.cpp suifpasses.cpp standard_modules.cpp dispatcher.cpp

HEADERS = dispatcher.h \
	passes.h \
	suifpasses.h \
	drivers.h \
	standard_modules.h

TARGET_LIB = suifpasses
LIBS += $(DYNAMIC_LINKING_LIBRARY) -lcommon -liokernel -lsuifkernel -lbasicnodes
PREPROCESSORFLAGS += -DUNIX_DLL_INTERFACE
INCLDIRS +=  

include $(NCIHOME)/Makefile.std






