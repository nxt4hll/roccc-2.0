#include "common/system_specific.h"
#include "helper.h"

String cut_off( String& start, char cut_char ) {
    int indentation = 0;
    int pos = 0;
    int length = start.size();

    for ( ; pos < length ; pos++ ) {
        char current_char = start[ pos ];
        if ( ( current_char == cut_char ) && ( !indentation ) ) break; 
        if ( current_char == '{' ) {
            indentation++;
            } 
	else if ( current_char == '}' ) {
      	    indentation--;
    	    }
  	}
    String cut = start.substr( 0, pos );
    if ( (pos < length) && (start[pos] == cut_char) ) pos++;
    start = start.Right( length-pos );
    return cut;
    }
