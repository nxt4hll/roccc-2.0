TARGET_LIB =	x86

CPP_HDRS =	instr.h opcodes.h reg_info.h printer.h code_gen.h code_fin.h \
		contexts.h init.h fpstack.h
HDRS =		$(CPP_HDRS) x86.h
CPPS =		$(CPP_HDRS:.h=.cpp) \
		translate.cpp translate_cvt.cpp translate_ldst.cpp
OBJS =		$(CPPS:.cpp=.o)
LIBS =		-lmachine -lsuifvm -lcfg

include $(MACHSUIFHOME)/Makefile.common
