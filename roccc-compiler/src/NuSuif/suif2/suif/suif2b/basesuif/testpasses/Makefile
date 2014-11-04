OBJ_FILES =  test_modules.o \

SRCS = test_modules.cpp 


TARGET_LIB = testpasses
LIBS += -lsuifkernel -liokernel -lbasicnodes -lsuifnodes \
	-lcommon -ltypebuilder -lsuifcloning
INCLDIRS += 

include $(NCIHOME)/Makefile.std

test1:
	suifdriver -e "import testpasses;testClone;testWalker"


