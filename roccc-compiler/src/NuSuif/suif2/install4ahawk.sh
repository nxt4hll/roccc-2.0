/bin/sh ./install --with-CC=gcc4 \
    --with-CXX=g++4 \
    --with-CXXLINK=g++4 \
    --with-GC_LIBDIRS='-L/usr/local/gcc-4.0.2/lib -lgc' \
    --with-GC_INCLDIRS='-I/usr/local/gcc-4.0.2/include' \
    --with-DOT=/usr/bin/dot \
    --with-TCL_INCLDIRS='-I/usr/local/include' \
    --with-TCL_LIBDIRS='-L/usr/local/lib -L/usr/X11/lib -L/usr/X11R6/lib'
