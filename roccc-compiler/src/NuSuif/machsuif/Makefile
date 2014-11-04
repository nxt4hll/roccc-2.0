PACKAGES =	machine suifvm cfg cfa bvd dce
PACKAGES += s2m peep
PACKAGES +=	ssa dcessa
PACKAGES += scripts
PACKAGES += m2a


#PACKAGES +=	gen make_2opnd 
#PACKAGES += m2c raga m2a fin

##PACKAGES +=	doc cookbook 
## alpha x86 x86_ppro ia64 
##PACKAGES +=	insch libpath select bb_sched gather_prof_res

#PACKAGES +=	halt #alpha_halt x86_halt x86_ppro_halt
#PACKAGES +=	halt_blpp #x86_halt_blpp

ALL_RULE =	defined		# Avoid build_tools rule.
BUILD_HEADERS_RULE = defined	# Avoid rule for .../include/PACKAGE/.

all:
	$(MAKE) INCLUDE_DEPS=yes buildpackage

include $(NCIHOME)/Makefile.std
