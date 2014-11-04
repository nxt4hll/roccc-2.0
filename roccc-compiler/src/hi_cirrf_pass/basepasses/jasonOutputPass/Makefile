SUIF_INTERNAL = defined

SRCS = composedOutput.cc baseOutput.cc moduleOutput.cc systemOutput.cc outputPass.cc jasonOutputPass.cc main.cc removeNonPrintable.cc intrinsicPass.cpp preferencePass.cpp castPass.cc identifyDebug.cc

OBJ_FILES = composedOutput.o baseOutput.o moduleOutput.o systemOutput.o outputPass.o jasonOutputPass.o main.o removeNonPrintable.o intrinsicPass.o preferencePass.o castPass.o identifyDebug.o

HEADERS =

LIBS += -lsuifkernel -liokernel -lcommon -lutils -ltransforms
LIBS += -lcfenodes -lsuifnodes -lbasicnodes -lsuifpasses 
LIBS += -ltypebuilder -lcfeutils -lsuifcloning -lsuiflocation
LIBS += -lroccc_utils -lroccc_extra_types
LIBS += -lroccc_extra_types

TARGET_LIB = jasonOutputPass

PASS_DRIVER =  do_jasonOutputPass
MAIN_OBJ = main.o


#EXTRA_CXXFLAGS += -I./.. 

include $(NCIHOME)/Makefile.std
