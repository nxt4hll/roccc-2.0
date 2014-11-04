PACKAGES = NuSuif hi_cirrf_pass #lo_cirrf_pass scripts


ALL_RULE =	defined		# Avoid build_tools rule.
BUILD_HEADERS_RULE = defined	# Avoid rule for .../include/PACKAGE/.

all:
	$(MAKE) INCLUDE_DEPS=yes buildpackage

include $(NCIHOME)/Makefile.std
