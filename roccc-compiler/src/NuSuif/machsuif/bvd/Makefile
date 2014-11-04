TARGET_LIB =	bvd

CPPS =		solve.cpp flow_fun.cpp liveness.cpp catalog.cpp init.cpp \
		reaching_defs.cpp
HDRS =		$(CPPS:.cpp=.h) bvd.h
OBJS =		$(CPPS:.cpp=.o)
LIBS =		-lmachine -lcfg

#NWHDRS =	$(filter-out reaching_defs.h,$(HDRS))
#NWDOC =		$(TARGET_LIB)
#NWSRCS =	doc-intro.nw $(NWHDRS:=.nw) doc-end.nw
#NWDICT =	doc-end.nw

#DEBRIS =	$(NWDOC).tex

ifneq (,$(FEWCELLS))
EXTRA_CXXFLAGS += -DREG_CELLS_MAX_32
endif

include $(MACHSUIFHOME)/Makefile.common
