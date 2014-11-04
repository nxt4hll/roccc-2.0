SUIF_INTERNAL = defined


OBJ_FILES =  meta_class.o \
meta_class_iter.o \
aggregate_meta_class.o \
stl_meta_class.o \
object_factory.o \
list_meta_class.o \
lstring_meta_class.o \
pointer_meta_class.o \
integer_meta_class.o \
string_meta_class.o \
object.o \
i_integer_meta_class.o \
object_stream.o \
clone_stream.o \
synchronizer.o \
binary_streams.o \
union_meta_class.o \
virtual_iterator.o \
helper.o \
iokernel_messages.o \
walker.o \
object_wrapper.o \
pointer_wrapper.o \
aggregate_wrapper.o \
field_wrapper.o \
field_description.o \
list_wrapper.o

SRCS = meta_class.cpp \
meta_class_iter.cpp \
aggregate_meta_class.cpp \
stl_meta_class.cpp \
object_factory.cpp \
list_meta_class.cpp \
lstring_meta_class.cpp \
pointer_meta_class.cpp \
integer_meta_class.cpp \
string_meta_class.cpp \
object.cpp \
i_integer_meta_class.cpp \
object_stream.cpp \
clone_stream.cpp \
synchronizer.cpp \
binary_streams.cpp \
union_meta_class.cpp \
virtual_iterator.cpp \
iokernel_messages.cpp \
helper.cpp \
walker.cpp \
object_wrapper.cpp \
pointer_wrapper.cpp \
aggregate_wrapper.cpp \
field_wrapper.cpp \
field_description.cpp \
list_wrapper.cpp

HEADERS = aggregate_meta_class.h      object.h                  i_integer_meta_class.h \
        binary_streams.h              \
        clone_stream.h             object_stream.h            integer_meta_class.h \
        stl_meta_class.h            list_meta_class.h \
        synchronizer.h            lstring_meta_class.h \
        helper.h                  union_meta_class.h          metaclass_visitor.h \
        meta_class.h               meta_class_iter.h \
	virtual_iterator.h         \
        object_factory.h              pointer_meta_class.h \
        cast.h		string_meta_class.h walker.h \
	iokernel_forwarders.h \
	iokernel_messages.h \
	object_wrapper.h \
	pointer_wrapper.h \
	aggregate_wrapper.h \
	field_wrapper.h \
	field_description.h \
	list_wrapper.h

TARGET_LIB = iokernel
LIBS += -lcommon
#PREPROCESSORFLAGS += -DUNIX_DLL_INTERFACE

#EXTRA_CXXFLAGS += -Weffc++
#EXTRA_CXXFLAGS += -DIOKERNEL_FILE_SIZE_OPT
# EXTRA_CXXFLAGS += -DDEBUG

include $(NCIHOME)/Makefile.std
