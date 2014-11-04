#SUIF_INTERNAL = defined

HOOF_MAC = $(SMGNDIR)/suif.mac
HOOF_DEP =   ../basicnodes/basic.hoof
HOOF_INCL = -I$(NCIHOME)/suif/suif2b/basesuif
SMGN_INCLUDES =	$(SMGNDIR)/searchable_list_builders.mac \
		$(SMGNDIR)/indexed_list_builders.mac \
		$(SMGNDIR)/list_builders.mac \
		$(SMGNDIR)/vector_builders.mac 
HOOF = suif
HOOF_GEN =  \
	$(HOOF).h  $(HOOF)_factory.h \
	$(HOOF).cpp $(HOOF)_forwarders.h

OBJ_FILES =  $(HOOF).o

SRCS = $(HOOF).cpp

HEADERS = $(HOOF).h  $(HOOF)_factory.h $(HOOF)_forwarders.h

TARGET_LIB = suifnodes

INCLDIRS += 
LIBS += -lsuifkernel -liokernel -lbasicnodes -lcommon

include $(NCIHOME)/Makefile.std
