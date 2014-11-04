BINSCRIPTS = suif2c print_driver link_suif \
		suif1to2 suif2to1 cpp2suif suif2a pass_driver \
		f2suif cppsuif2cpp c2spre \
		suifshell.csh

# The rest of these are legacy scripts.
BINSCRIPTS += deadcode_driver ecr_driver pgcc_driver \
		s1to2_driver s1_driver suif1_driver \
		s1link2_driver

BINSCRIPTS += do_print do_printsuif
BINSCRIPTS += pass_driver_bk

BUILDPACKAGE_RULE = define

buildpackage: install_scripts

all: install_scripts

include $(NCIHOME)/Makefile.std
