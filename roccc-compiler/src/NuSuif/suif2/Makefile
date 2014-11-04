PACKAGES = suif/suif2b #driverscripts

ALL_RULE=defined

all: sanity_checks build_tools buildpackage

sanity_checks:
	@if test ! -d $(NCIHOME)/solib; then $(MAKE) setup; fi



DOXYGEN = doxygen doxygen.config

docs:
	${DOXYGEN}



include Makefile.std
