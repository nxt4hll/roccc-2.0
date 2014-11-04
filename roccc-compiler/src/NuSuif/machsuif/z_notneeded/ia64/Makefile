TARGET_LIB =	ia64

PAIRS =		contexts.h instr.h opcodes.h reg_info.h printer.h code_gen.h \
		code_fin.h init.h
HDRS =		$(PAIRS) ia64.h
CPPS =		$(PAIRS:.h=.cpp) \
		translate.cpp translate_cvt.cpp translate_ldst.cpp \
		translate_call.cpp
OBJS =		$(CPPS:.cpp=.o)

LIBS =		-lmachine -lsuifvm -lcfg

include $(MACHSUIFHOME)/Makefile.common
