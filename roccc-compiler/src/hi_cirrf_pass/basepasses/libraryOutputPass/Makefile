SUIF_INTERNAL = defined

SRCS = libOutputPass.cc libraryOutputPass.cc main.cc removeModulePass.cpp dumpHeaderPass.cpp addModulePass.cpp exportPass.cpp cleanRepository.cpp

OBJ_FILES = libOutputPass.o libraryOutputPass.o main.o removeModulePass.o dumpHeaderPass.o addModulePass.o exportPass.o cleanRepository.o

HEADERS =

LIBS += -lsuifkernel -liokernel -lcommon -lutils -ltransforms
LIBS += -lcfenodes -lsuifnodes -lbasicnodes -lsuifpasses 
LIBS += -ltypebuilder -lcfeutils -lsuifcloning -lsuiflocation
LIBS += -lroccc_utils

TARGET_LIB = libraryOutputPass

PASS_DRIVER =  do_libraryOutputPass
MAIN_OBJ = main.o


#EXTRA_CXXFLAGS += -I./.. 

include $(NCIHOME)/Makefile.std
