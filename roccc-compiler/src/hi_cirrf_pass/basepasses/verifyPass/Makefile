SUIF_INTERNAL = defined

SRCS = identificationPass.cc verifyPass.cc verify20Pass.cc main.cc verifySystolicArray.cc markSystemToSystemPass.cpp preVerifyPass.cpp

OBJ_FILES = identificationPass.o verifyPass.o verify20Pass.o main.o verifySystolicArray.o markSystemToSystemPass.o preVerifyPass.o

HEADERS =

LIBS += -lsuifkernel -liokernel -lcommon -lutils -ltransforms
LIBS += -lcfenodes -lsuifnodes -lbasicnodes -lsuifpasses 
LIBS += -ltypebuilder -lcfeutils -lsuifcloning -lsuiflocation
LIBS += -lroccc_utils

TARGET_LIB =verifyRoccc

PASS_DRIVER =  do_verifyRoccc
MAIN_OBJ = main.o


#EXTRA_CXXFLAGS += -I./.. 

include $(NCIHOME)/Makefile.std
