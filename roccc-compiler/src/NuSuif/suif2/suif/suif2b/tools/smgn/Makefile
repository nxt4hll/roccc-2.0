HEADERS = grammar.h macro.h macroBase.h parser.h rem_comments.h value_types.h
SRCS = grammar.cpp macro.cpp macroBase.cpp parser.cpp rem_comments.cpp smgn.cpp rename.cpp
OBJ_FILES = grammar.o macro.o macroBase.o parser.o rem_comments.o

SUIF_INTERNAL = defined

MAIN_OBJ = smgn.o
PROGRAM = smgn

TARGET_LIB = smgn
LIB_OBJ = macro.o macroBase.o

MAIN2_OBJ = rename.o
PROGRAM2 = renamer

LIBS += -lcommon

include $(NCIHOME)/Makefile.std
