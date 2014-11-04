/bin/sh ./install --with-CC=gcc-4.0 \
    --with-CXX=g++-4.0 \
    --with-CXXLINK=g++-4.0 \
    --with-GC_LIBDIRS='-L/usr/lib -lgc' \
    --with-GC_INCLDIRS='-I/usr/include' \
    --with-DOT=/usr/bin/dot \
    --with-TCL_INCLDIRS='-I/usr/include' \
    --with-TCL_LIBDIRS='-L/usr/lib -L/usr/X11/lib -L/usr/X11R6/lib'
