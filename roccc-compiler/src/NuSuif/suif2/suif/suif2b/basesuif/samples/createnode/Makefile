OBJ_FILES =  init.o create_suif_complex_input.o

SRCS = init.cpp create_suif_complex_input.cpp main.cpp

TARGET_LIB = createnode

# PROGRAM = create_suif_complex_input
LIBS += -liokernel -lsuifkernel -lsuifnodes -lbasicnodes -lcommon \
	-lsuifpasses -ltypebuilder -lsuifcloning

PASS_DRIVER = do_createnode
MAIN_OBJ = main.o

include $(NCIHOME)/Makefile.std

test1:
	LD_LIBRARY_PATH=$(LIBDIR):$(LD_LIBRARY_PATH) ; \
		$(BINDIR)/driver -f t2.suif2 > hello2.txt
	cat hello2.txt
	rm hello2.txt
test2:
	LD_LIBRARY_PATH=$(LIBDIR):$(LD_LIBRARY_PATH) ; \
		$(BINDIR)/driver -f testcreatenode.suif2 > hello2.txt
	cat hello2.txt
	rm hello2.txt
test3:
	LD_LIBRARY_PATH=$(LIBDIR):$(LD_LIBRARY_PATH) ; \
		$(BINDIR)/driver -f t3.suif2 > hello2.txt
	cat hello2.txt
	rm hello2.txt

