SUIF_INTERNAL = defined

SRCS = s2c_pass.cpp
TARGET_LIB = s2c

OBJ_FILES = s2c_pass.o 

LIBS += -lcommon -lsmgn -liokernel -lsuifkernel -lbasicnodes \
        -lsuifnodes -lmacro_suifobj_adapter -lsuifpasses -lcfenodes

INCLDIRS += -I$(NCIHOME)/suif/suif2b/tools/smgn -I$(BASETYPEDIR) -I../iokernel -I../suifkernel -I../basicnodes -I../suifnodes -I../macroObjAdapter -I../suifpasses

include $(NCIHOME)/Makefile.std

s2ctest:
	LD_LIBRARY_PATH=$(LIBDIR):$(LD_LIBRARY_PATH) ; \
	export LD_LIBRARY_PATH ; \
	$(BINDIR)/driver -f s2c.suif2 > hello2.txt
	cat hello2.txt
	rm hello2.txt
