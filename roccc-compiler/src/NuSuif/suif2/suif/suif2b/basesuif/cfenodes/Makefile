HOOF = cfe
OBJ_FILES =  $(HOOF).o

HOOF_MAC = $(SMGNDIR)/suif.mac
HOOF_DEP =   ../basicnodes/basic.hoof
HOOF_INCL = -I$(NCIHOME)/suif/suif2b/basesuif
SMGN_INCLUDES =	$(SMGNDIR)/searchable_list_builders.mac \
		$(SMGNDIR)/indexed_list_builders.mac \
		$(SMGNDIR)/list_builders.mac \
		$(SMGNDIR)/vector_builders.mac
HOOF_GEN =  $(HOOF).h  $(HOOF)_factory.h $(HOOF).cpp $(HOOF)_forwarders.h

SRCS = $(HOOF).cpp

HEADERS = $(HOOF).h  $(HOOF)_factory.h $(HOOF)_forwarders.h

TARGET_LIB = cfenodes

INCLDIRS +=
LIBS += -lsuifkernel -liokernel -lbasicnodes -lsuifnodes -lcommon

include $(NCIHOME)/Makefile.std