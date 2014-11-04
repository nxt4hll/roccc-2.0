##########################################################################################
# Common tmake project file, intended to be included when running tmake from make2dsp.pl
##########################################################################################

TEMPLATE        = app
# Additional preprocessor symbols to define
DEFINES		= \
	WIN32_LEAN_AND_MEAN \
	WIN32_DLL_INTERFACE \
	MSVC \
	NO_COVARIANCE \
	NO_IMPLICIT_USING  \
	INLINE_ALL_TEMPLATES \
	NAF_NO_GC

INCLUDEPATH 	= \
	$(GC_DIR) \
	$(NCIHOME)\suif\suif2b \
	$(NCIHOME)\suif\suif2b\basesuif \
	$(NCIHOME)\suif\suif2b\basepasses \
	$(NCIHOME)\suif\suif2b\ipanalysis \
	$(NCIHOME)\suif\suif2b\extratypes \
	$(NCIHOME)\suif\suif2b\naf_analyses \
	$(NCIHOME)\suif\suif2b\dataflow \
        $(NCIHOME)\suif\suif2b\region_problems \
        $(NCIHOME)\suif\suif2b\baseregion \
	$(NCIHOME)\suif\suif2b\basenaf

win32:TMAKE_LFLAGS += /libpath:$(NCIHOME)\solib
win32:TMAKE_CFLAGS += /Gm /GX /ZI /Od /GR /YX /GZ /MDd /Gi

PRE_LINK 	= $(NCIHOME)\bin\stub.exe $(OutDir)

CLEAN_FILES 	= .\Debug\*.dmp .\Debug\*.def

GC_INCLDIRS 	= $(GC_DIR)
OMEGA_INCLDIRS 	= $(OMEGA_DIR)\basic\include,$(OMEGA_DIR)\omega_lib\include