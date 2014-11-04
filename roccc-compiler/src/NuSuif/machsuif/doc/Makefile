# Makefile for Machine SUIF documentation directory

# Subdirectories with documentation
PACKAGES =	opi-users opi-extenders

# Machine SUIF library directories containing documents, and paths to
# the typeset forms of those documents.
#
DOCLIBDIRS =	cookbook machine suifvm cfg cfa bvd halt ssa

# Local document.  $(DOCS) is a prerequisite for buildpackage, which is the
# goal induced by the default (`all') rule.
#
NWDOC =		overview
NWSRCS =	$(NWDOC).nw
DOCS =		$(NWDOC).ps $(NWDOC).html \
		$(DOCLIBDIRS:=.ps) $(DOCLIBDIRS:=.html)

include $(MACHSUIFHOME)/Makefile.common

$(DOCLIBDIRS:=.ps) $(DOCLIBDIRS:=.html):
	rm -f $@;  ln -s $(MACHSUIFHOME)/$(basename $@)/$@ .

zap: clean
	rm -f $(DOCS) $(NWDOC).tex
