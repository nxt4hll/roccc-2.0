#ifndef IOKERNEL__HELPER_H
#define IOKERNEL__HELPER_H

#include "iokernel_forwarders.h"
#include <stdio.h>

String cut_off( String& start, char cut_char );

template<class T> void delete_list_and_elements( T* &t ) {
 if ( t ) {
    typename T::iterator current = t->begin(),
                          end = t->end();
    while ( current != end ) {
       delete (*current);
       current++;
    }
  }
  delete t;
  t = 0;
}

template<class T> void delete_list_elements( T* t ) {
 if ( t ) {
    typename T::iterator current = t->begin(),
                          end = t->end();
    while ( current != end ) {
       delete (*current);
       current++;
    }
    t->clear();
  }
}

template<class T> void delete_map_and_value( T* &t ) {
 if ( t ) {
    typename T::iterator current = t->begin(),
                          end = t->end();
    while ( current != end ) {
       delete (*current).second;
       current++;
    }
  }
  delete t;
  t = 0;
}

template<class T> void delete_map_values( T* t ) {
 if ( t ) {
    typename T::iterator current = t->begin(), end = t->end();
    while ( current != end ) {
       delete (*current).second;
       current++;
    }
  }
  t->clear();
}

#endif
