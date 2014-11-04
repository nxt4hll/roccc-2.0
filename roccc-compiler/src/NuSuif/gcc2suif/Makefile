PACKAGES = driverscripts

SRCS = $(OBJ_FILES:.o=.cc)
#HEADERS = $(OBJ_FILES:.o=.h) 
OBJ_FILES = ir.tab.o lexer.o baseNode.o declNodes.o stmtNodes.o exprNodes.o nodes.o typeNodes.o cstNodes.o option.o function.o program.o type.o suifGenerator.o

#MAIN_OBJ  = lexer.o
PROGRAM = parser

YACC_GENERATED_FILES = ir.tab.cc ir.tab.hh
LEX_GENERATED_FILES = lexer.cc

#parser: ${OBJECTS}
#	${CXX} -Wall -g ${OBJECTS} -o parser ${EXTRALIBS}
#
#ir.tab.o: ir.y
#	bison -d ir.y
#	${CC} -Wall -c -g -I${SUIF_INCLUDES} ir.tab.c
#
#lex.yy.o: lexer.fl
#	flex lexer.fl
#	${CC} -Wall -c -g -I${SUIF_INCLUDES} lex.yy.c
#

LIBS = -lsuifkernel -lcommon -ltypebuilder -lcfenodes
LIBS += -liokernel -lutils -lsuifnodes -lbasicnodes -lsuifpasses #-ltransforms

# Added by Jason to work with Mac OS X
#LIBS += -liokernel -lutils -ltransforms -lsuifnodes -lbasicnodes -lsuifpasses

include $(NCIHOME)/Makefile.std
