#SUIF_INTERNAL = defined

HOOF_MAC = $(SMGNDIR)/suif.mac
HOOF_GEN = basic.cpp basic_forwarders.h basic.h basic_factory.h
SMGN_INCLUDES = $(SMGNDIR)/searchable_list_builders.mac \
                $(SMGNDIR)/indexed_list_builders.mac \
                $(SMGNDIR)/list_builders.mac \
                $(SMGNDIR)/vector_builders.mac 
HOOF = basic

SRCS = $(HOOF).cpp $(HOOF)_constants.cpp
OBJ_FILES =  $(HOOF).o $(HOOF)_constants.o
HEADERS = \
	$(HOOF)_constants.h $(HOOF)_forwarders.h \
	$(HOOF).h $(HOOF)_factory.h

TARGET_LIB = basicnodes

INCLDIRS += 
LIBS += -lsuifkernel -liokernel -lcommon

include $(NCIHOME)/Makefile.std
