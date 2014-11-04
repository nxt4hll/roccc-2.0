#include "common/system_specific.h"
#include <iostream>
#include "command_line_parsing.h"

#include "iokernel/helper.h"

#include "token_stream.h"
//#include "error_macros.h"
#include "common/suif_vector.h"

#include <stdlib.h>
#include <errno.h>
using namespace std;


struct OptionDescription {

  OptionDescription(String group = emptyString,
		    String argument = emptyString,
		    String description = emptyString ) :
    _group( group ),
    _argument( argument ),
    _description( description )
    {
    }

  String _group;        // e.g.: printing
  String _argument;     //       -f file-name
  String _description;  //       the output file
};



ValueClass::ValueClass( ValueClass* parent, Option* owning_option ) :
  _parent( parent ),
  _owning_option( owning_option ) {
}

ValueClass::~ValueClass() {
}


StructureValueClass::StructureValueClass( ValueClass* parent,
                                          Option* owning_option ) :
          ValueClass( parent, owning_option ) {
}


StringValueClass::StringValueClass( Token& t, ValueClass* parent, Option* owning_option ) :
           ValueClass( parent, owning_option ) ,_token( new Token( t ) ) {
}

StringValueClass::~StringValueClass() {
  delete _token;
}

const String StringValueClass::get_string() const {
  return _token->_token;
}


IntValueClass::IntValueClass( Token& t, ValueClass* parent,
                              Option* owning_option ) :
           ValueClass( parent, owning_option ) , _token( new Token( t ) ) {
}

IntValueClass::~IntValueClass() {
  delete _token;
}

int IntValueClass::get_int() const {
  return atoi( _token->_token );
}



Option::Option( String argument,
                Option* parent ) :
  _description( emptyString ),
  _group( emptyString ),
  _argument( argument ),
  _parent( parent ),
  _values( new suif_vector<ValueClass*> )  {
}

Option::Option( String argument,
		String description,
                Option* parent ) :
  _description( description ),
  _group( emptyString ),
  _argument( argument ),
  _parent( parent ),
  _values( new suif_vector<ValueClass*> )  {
}



Option::~Option() {
  delete_list_and_elements( _values );
}


void Option::set_description( const String& description,
                              const String& group ) {
  _description = description;
  _group = group;
}

String Option::get_description() const {
  return(_description);
}


void Option::set_argument( const String& argument ) {
  _argument = argument;
}


bool Option::parse_options( istream& input_stream,
			    bool &changed,
			    ErrorSubSystem* output_for_errors ) {
  IstreamCharSource cs(&input_stream);
  TokenStream t( &cs );
  return parse( &t, changed ) && t.is_empty();
}

bool Option::parse_options( int argc, char *argv[],
			    bool &changed,
			    ErrorSubSystem* output_for_errors ) {
  TokenStream t( argc, argv );
  return parse( &t, changed ) && t.is_empty();
}

void Option::delete_values() {
  if ( _values ) {
    int size = _values->size();

    while ( size ) {
      delete (*_values)[size-1];
      _values->pop_back();
      size--;
    }
  }
}

int Option::get_number_of_values() const {
     return _values->size();
}


void Option::print_to_stream( ostream& output_stream ) const {
  String str = to_string();
  output_stream << str;
}
void Option::print_debug() const {
  String str = to_string();
  cerr << str;
}


String Option::to_string() const {
  String output;

  String command_line;
  suif_vector<OptionDescription> descriptions;

  print( command_line, &descriptions );
  output = command_line + "\n";

  output += "Description:\n";

  for ( size_t i = 0; i < descriptions.size(); i++ ) {
    //ignored for the moment    output_stream<<descriptions[ i ]._group<<endl;
    String arg_desc = descriptions[ i ]._argument;
    if (arg_desc != emptyString)
      arg_desc += " ";
    arg_desc += descriptions[ i ]._description;

    if (arg_desc != emptyString)
      arg_desc += "\n";
    output += arg_desc;
  }
  return(output);
}







ValueClass* Option::get_value_class(
                          ValueClass* value,
                          Option* context ) const {
  ValueClass* common_value_class = value;
  while ( common_value_class &&
          common_value_class->_owning_option != context ) {
    common_value_class = common_value_class -> _parent;
  }

  if ( !common_value_class ) return 0;

  // iterate over all value
  int number = get_number_of_values();
  for ( int i = 0 ; i < number ; i++ ) {
    ValueClass* current = (*_values)[i];
    ValueClass* v = current;
    while ( v && v != common_value_class ) v = v->_parent;
    if ( v ) {
       return v;
    }
  }
  return 0;
}



// OptionInt
// OptionString
// OptionLiteral
void Option::print(  String& command_line_string,
		     suif_vector<OptionDescription> *descriptions ) const {
  String argument;

  argument.append( "<" );

  argument.append( _argument );
  //  printf("%d %s\n", argument.length(), argument.c_str());
  argument.append( ">" );
  //  printf("%d %s\n", argument.length(), argument.c_str());
  command_line_string.append( argument );

  if ( !_description.is_empty() ) {
     OptionDescription description( _group, argument, _description );
     descriptions->push_back( description );
  }
}






OptionString::OptionString( String argument,
                            String* storage_for_string ) :
          Option( argument ),
          _storage_for_string( storage_for_string ) {
}


OptionString::~OptionString() {
}


/*
 * An OptionString will not match pass the command boundary
 */
bool OptionString::parse( TokenStream* tokens, 
			  bool &changed,
			  ValueClass* parent  ) {
  Token t;
  if (tokens->is_at_end_of_command())
    return false;
  bool has_succeeded =  tokens->get_token( t );
  if ( has_succeeded ) {
     _values -> push_back( new StringValueClass( t, parent, this ) );
     if ( _storage_for_string ) {
       *_storage_for_string = t._token;
     }
     changed = true;
  }
  return has_succeeded;
}

const StringValueClass* OptionString::get_string( int number ) const {
  return (StringValueClass*)(*_values)[ number ];
}

StringValueClass* OptionString::get_string(
                          ValueClass* value,
                          Option* context ) const {
  return (StringValueClass*)get_value_class( value, context );
}




OptionPrefixString::OptionPrefixString( String prefix,
					String argname,
					String description,
					String* storage_for_string ) :
  Option( prefix, description, NULL ),
  _storage_for_string( storage_for_string ) {
}


OptionPrefixString::~OptionPrefixString() {
}


/*
 * An OptionPrefixString will not match pass the command boundary
 */
bool OptionPrefixString::parse( TokenStream* tokens, 
				bool &changed,
				ValueClass* parent  ) {
  Token t;
  if (tokens->is_at_end_of_command())
    return false;

  if (tokens->peek_token( t )) {
    String new_val = t._token;
    new_val.truncate_at_pos(_argument.length());
    if (new_val != _argument)
      return false;

    // success
    tokens->get_token(t);
    _values -> push_back( new StringValueClass( t, parent, this ) );
    if ( _storage_for_string ) {
      *_storage_for_string = t._token;
    }
    changed = true;
  }
  return true;
}

const StringValueClass* OptionPrefixString::get_string( int number ) const {
  return (StringValueClass*)(*_values)[ number ];
}

StringValueClass* OptionPrefixString::get_string(
                          ValueClass* value,
                          Option* context ) const {
  return (StringValueClass*)get_value_class( value, context );
}

OptionMultiString::OptionMultiString( String argument,
				      suif_vector<String> *storage_for_string ) :
          Option( argument ),
          _storage_for_string( storage_for_string ) {
}


OptionMultiString::~OptionMultiString() {
}


/*
 * An OptionMultiString will not match pass the command boundary
 */
bool OptionMultiString::parse( TokenStream* tokens, 
			       bool &changed,
			       ValueClass* parent  ) {
  Token t;
  if (tokens->is_at_end_of_command())
    return false;
  bool has_succeeded =  tokens->get_token( t );
  if ( has_succeeded ) {
     _values -> push_back( new StringValueClass( t, parent, this ) );
     if ( _storage_for_string ) {
       _storage_for_string->push_back(t._token);
     }
     changed = true;
  }
  return has_succeeded;
}

const StringValueClass* OptionMultiString::get_string( int number ) const {
  return (StringValueClass*)(*_values)[ number ];
}

StringValueClass* OptionMultiString::get_string(
                          ValueClass* value,
                          Option* context ) const {
  return (StringValueClass*)get_value_class( value, context );
}




OptionInt::OptionInt( String argument,
                      int* storage_for_integer ) :
    Option( argument ),
    _storage_for_default_value( storage_for_integer ) {
}


OptionInt::~OptionInt() {
}


IntValueClass* OptionInt::get_int( int number ) {
  return (IntValueClass*)(*_values)[number];
}

IntValueClass* OptionInt::get_int(
                          ValueClass* value,
                          Option* context ) const {
  return (IntValueClass*)get_value_class( value, context );
}




bool OptionInt::parse( TokenStream* tokens, 
		       bool &changed,
		       ValueClass* parent ) {
  Token t;
  char * str_end; 
  if ( !tokens->get_token( t ) ) return false;
  errno = 0;
  strtol( t._token.c_str(), &str_end, 10 );
  if (str_end != (t._token.c_str()+t._token.length())) {
    tokens->push_back( t );
    return false;
  }
  IntValueClass* int_value = new IntValueClass( t, parent, this );
  _values -> push_back( int_value );
  if ( _storage_for_default_value ) {
    *_storage_for_default_value = int_value->get_int();
  }
  changed = true;
  return true;
}


OptionLiteral::OptionLiteral(
                  String argument,
                  bool* storage_for_is_set,
                  bool default_value ) :
  Option( argument ),
  _storage_for_is_set( storage_for_is_set ),
  _default_value( default_value )
{
}

OptionLiteral::OptionLiteral(
			     String argument,
			     String description,
			     bool* storage_for_is_set,
			     bool default_value ) :
  Option( argument, description, NULL ),
  _storage_for_is_set( storage_for_is_set ),
  _default_value( default_value )
{
}


OptionLiteral::~OptionLiteral() {
}


bool OptionLiteral::parse( TokenStream* tokens, 
			   bool &changed,
			   ValueClass* parent ) {
  Token t;
  if ( !tokens->get_token( t ) ) return false;

  if ( t._token != _argument ) {
    tokens->push_back( t );
    return false;
  }

  _values -> push_back( new StringValueClass( t, parent, this ) );

  if ( _storage_for_is_set ) {
    *_storage_for_is_set = _default_value;
  }
  changed = true;
  return true;
}


bool OptionLiteral::is_set() const {
  return ( get_number_of_values() != 0 );
}

bool OptionLiteral::is_set ( ValueClass* value,
				     Option* context ) const {
  return (bool)get_value_class( value, context );
}



void OptionLiteral::print(String& command_line_string,
			  suif_vector<OptionDescription>* descriptions) const
{
  command_line_string.append( _argument );
  if ( !_description.is_empty() ) {
    OptionDescription description( _group, _argument, _description );
    descriptions->push_back( description );
  }
}











OptionSelection::OptionSelection( bool allow_empty) :
    Option( emptyString ),
    _allow_empty( allow_empty ),
    _selection_list( new Options ) {
}

OptionSelection::OptionSelection( Option *opt1, bool allow_empty ) :
    Option( emptyString ),
    _allow_empty( allow_empty ),
    _selection_list( new Options ) {
  add(opt1);
}

OptionSelection::OptionSelection( Option *opt1, Option *opt2, 
				  bool allow_empty ) :
    Option( emptyString ),
    _allow_empty( allow_empty ),
    _selection_list( new Options ) {
  add(opt1);
  add(opt2);
}

OptionSelection::~OptionSelection() {
  delete_list_and_elements( _selection_list );
}

OptionSelection* OptionSelection::add( Option* o ) {
  _selection_list -> push_back( o );
  return this;
}


bool OptionSelection::parse( TokenStream* tokens, 
			     bool &changed,
			     ValueClass* parent ) {
  StructureValueClass* selection_list = new StructureValueClass(parent, this);
  _values -> push_back( selection_list );
  size_t cases = _selection_list->size();
  for ( size_t i = 0 ; i < cases ; i++ ) {
    bool sub_changed = false;
    if ( (*_selection_list)[i]->parse( tokens, sub_changed,selection_list ) ) {
      if (sub_changed)
	changed = true;
      return true;
    }
  }
  return _allow_empty;
}

void OptionSelection::delete_values() {
  Option::delete_values();
  int cases = _selection_list->size();
  for ( int i = 0 ; i < cases ; i++ ) {
    (*_selection_list)[i]->delete_values();
  }
}



void OptionSelection::print(  String& command_line_string,
			      suif_vector<OptionDescription>* descriptions ) const {



  int cases = _selection_list->size();
  if ( cases == 0 ) return; // don't print anything if the selection contains nothing
  command_line_string.append( " [ " );
  for ( int i = 0 ; i < cases ; i++ ) {
    (*_selection_list)[i]->print( command_line_string, descriptions );
    if ( (i+1) < cases ) command_line_string.append( " | " );
    }

  command_line_string.append( " ] " );
  if ( !_description.is_empty() ) {
     OptionDescription description( _group, _argument, _description );
     descriptions->push_back( description );
  }
}







OptionList::OptionList() :
    Option(),
    _option_list( new suif_vector<Option*> ) {
}

OptionList::OptionList(Option *opt1) :
    Option(),
    _option_list( new suif_vector<Option*> ) {
  add(opt1);
}
OptionList::OptionList(Option *opt1, Option *opt2) :
    Option(),
    _option_list( new suif_vector<Option*> ) {
  add(opt1);
  add(opt2);
}

OptionList::~OptionList() {
  delete_list_and_elements( _option_list );
}

OptionList* OptionList::add( Option* option ) {
  _option_list -> push_back( option );
  return this;
}


bool OptionList::parse( TokenStream* tokens, 
			bool &changed,
			ValueClass* parent ) {
  StructureValueClass* list_contents = 
    new StructureValueClass( parent, this );
  bool sub_changed = false;
  _values -> push_back( list_contents );
  size_t size = _option_list->size();
  for ( size_t i = 0; i < size ; i++ ) {
    // This does not look right.  Shouldn't we put back any
    // parsed tokens if this fails?  Is that done somewhere else?
    // @@@ BUGBUG??
    if ( !( *_option_list)[i]->parse( tokens, sub_changed, list_contents ) ) {
      // push back all tokens.
      return false;
    }
  }
  if (sub_changed)
    changed = true;
  return true;
}


void OptionList::delete_values() {
  Option::delete_values();
  int size = _option_list->size();
  for ( int i = 0; i < size ; i++ ) {
    ( *_option_list)[i]->delete_values();
  }
}


/*
 * If this option has a description, then a new entry will be added to
 *   descriptions.  This new entry will have 
 *    argument - the command_line_string of all its components
 *    description - the same as this description.
 */
void OptionList::print(String& command_line_string,
                       suif_vector<OptionDescription>* descriptions ) const {
  String argument;
  int index = 0;
  if ( !_description.is_empty() ) {
     index = descriptions -> size();
     descriptions->push_back(OptionDescription(_group, _argument,
					       _description ) );
  }
  size_t size = _option_list->size();
  for ( size_t i = 0; i < size ; i++ ) {
    ( *_option_list)[i]->print( argument, descriptions );
    argument.append(" ");
  }
  command_line_string.append( argument );
  if ( !_description.is_empty() ) {
    (*descriptions)[index]._argument = argument;
  }
}






OptionLoop::OptionLoop( Option* loop_contents,
                        bool allow_empty ) :
    Option(),
    _loop_contents( loop_contents ),
    _allow_empty( allow_empty ) {
}

OptionLoop::~OptionLoop() {
  delete _loop_contents;
}


// BUG - this assume allow_empty is true.
//
bool OptionLoop::parse( TokenStream* tokens, bool &changed,
			ValueClass* parent ) {
  StructureValueClass* loop_instance = new StructureValueClass( parent, this );
  _values -> push_back( loop_instance );

  bool change_here = false;
  do {
    bool local_changed = false;
    if (_loop_contents->parse( tokens, local_changed, loop_instance )) {
      if (!local_changed)
	break;
      change_here = true;
    } else {
      break;
    }
  } while (1);

  if (change_here) {
    changed = true;
    return(true);
  }
  return (_allow_empty);
}


void OptionLoop::delete_values() {
  Option::delete_values();
  _loop_contents->delete_values();
}

void OptionLoop::print(  String& command_line_string,
                       suif_vector<OptionDescription>* descriptions ) const {
  command_line_string.append( " { " );
  String argument;
  _loop_contents->print( argument, descriptions );
  command_line_string.append( argument );
  command_line_string.append( " } " );
  if ( !_description.is_empty() ) {
     OptionDescription description( _group, argument, _description );
     descriptions->push_back( description );
  }
}







OptionStream::OptionStream( String argument,
                            istream** storage_for_stream ) :
  Option( argument ),
  _storage_for_stream( storage_for_stream )  {
}

OptionStream::~OptionStream() {
}

bool OptionStream::parse( TokenStream* tokens, 
			  bool &changed,
			  ValueClass* parent ) {
  Token t;
  if (!tokens -> get_token( t ))
    return false;
  if ( t._token != String("{") ) {
     tokens -> push_back( t );
     return false;
  }
  istream* input_stream =  tokens->get_stream();
  StreamValueClass* value = new StreamValueClass( input_stream, parent, this );
  if ( _storage_for_stream ) {
     *_storage_for_stream = input_stream;
  }
  _values -> push_back( value );
  changed = true;
  return true;
}




StreamValueClass::StreamValueClass( istream* stream,
				    ValueClass* parent,
				    Option* owning_option ) :
  ValueClass( parent, owning_option ),
  _stream(stream)
{
}

istream& StreamValueClass::get_stream() const {
  return *_stream;
}



Option* build_prefixed_string( String prefix,
                               String argument,
                               String* storage,
                               String description ) {
  OptionList* list = new OptionList;
  list -> add( new OptionLiteral( prefix ) );
  list-> add( new OptionString( argument, storage ) );
  list -> set_description( description );
  return list;
}

Option* build_multi_prefixed_string( String prefix,
				     String argument,
				     suif_vector<String> *storage,
				     String description ) {
  OptionList* list = new OptionList;
  list -> add( new OptionLiteral( prefix ) );
  list-> add( new OptionMultiString( argument, storage ) );
  list -> set_description( description );
  return list;
}

