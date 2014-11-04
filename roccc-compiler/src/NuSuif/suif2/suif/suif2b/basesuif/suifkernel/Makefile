#SUIF_INTERNAL = defined
OBJ_FILES = real_object_factory.o \
	suif_env.o \
	dll_subsystem.o \
	error_subsystem.o \
	io_subsystem.o \
	io_subsystem_default_impl.o \
	module_subsystem.o \
	walking_maps_subsystem.o \
	print_subsystem.o \
	subsystem.o \
	module.o \
	suif_object.o \
	token_stream.o \
	command_line_parsing.o \
	suif_clone_stream.o \
	iter.o \
	suifkernel_messages.o \
	utilities.o \
	walking_maps.o \
	visitor_map.o \
	group_walker.o \
	suif_walker.o \
	all_walker.o \
	char_source.o \
	suif_exception.o \
	message_buffer.o

SRCS =  real_object_factory.cpp \
	suif_env.cpp \
	dll_subsystem.cpp \
	error_subsystem.cpp \
	io_subsystem.cpp \
	io_subsystem_default_impl.cpp \
	module_subsystem.cpp \
	walking_maps_subsystem.cpp \
	print_subsystem.cpp \
	subsystem.cpp \
	module.cpp \
	suif_object.cpp \
	token_stream.cpp \
	command_line_parsing.cpp \
	suif_clone_stream.cpp \
	iter.cpp \
	suifkernel_messages.cpp \
	utilities.cpp \
	walking_maps.cpp \
	visitor_map.cpp \
	group_walker.cpp \
	suif_walker.cpp \
	all_walker.cpp \
	char_source.cpp \
	suif_exception.cpp \
	message_buffer.cpp

HEADERS = \
	char_source.h \
	subsystem.h \
	command_line_parsing.h \
	suif_clone_stream.h \
	dll_subsystem.h \
	suif_env.h \
	error_subsystem.h \
	suif_object.h \
	forwarders.h \
	suif_walker.h \
	all_walker.h \
	group_walker.h \
	suifkernel_forwarders.h \
	io_subsystem.h \
	suifkernel_messages.h \
	io_subsystem_default_impl.h \
	token_stream.h \
	iter.h \
	utilities.h \
	module.h \
	cascading_map.h \
	visitor_map.h \
	module_subsystem.h \
	walking_maps.h \
	print_subsystem.h \
	walking_maps_subsystem.h \
	real_object_factory.h \
	suif_exception.h \
	message_buffer.h

TARGET_LIB = suifkernel
LIBS += $(DYNAMIC_LINKING_LIBRARY) -liokernel -lcommon
PREPROCESSORFLAGS += -DUNIX_DLL_INTERFACE
INCLDIRS += 

include $(NCIHOME)/Makefile.std
