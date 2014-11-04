#ifndef SUIFKERNEL_COMMANDLINE_PARSING_H
#define SUIFKERNEL_COMMANDLINE_PARSING_H

#include "suifkernel_forwarders.h"

/**
 * \file command_line_parsing.h
 * This is the interface to provide command line parsing.
 * The system is designed so that a subclass can add
 * additional command line options to the ones defined in a
 * superclass.
 *
 * The Module::_command_line is an *OptionList
 * which is a class that keeps track of the options for the module.

 * To add command line parsing to your module,
 * add your options to the Module::_command_line
 * in the initialize() routine.
 *

 * Here is an example of code for initialize
 *  \code
 *   // This will add a required field "-f filename"
 *  Module::initialize();
 *  _command_line->set_description("This pass constructs the call graph");
 *  OptionString* _file = new OptionString("-f", &_filename);
 *  Option *file_option = (new OptionList())->add(_file);
 *  _command_line->add(file_option);
 *
 *  file = fopen(_filename.c_str(), "wt");

 *  \code
 *  // This will add an optional "-verbose" switch
 *  Module::initialize();
 *  _command_line -> set_description("description of the pass");
 *  _verbose = new OptionLiteral("-verbose");
 *  OptionSelection *opt = new OptionSelection(true);
 *  opt->add(_verbose);
 *  _command_line->add(opt);
 *  \endcode

 *  \code
 *  // This will add a required "-bigendian" or "-littleendian" switch
 *  Module::initialize();
 *  _command_line -> set_description("description of the pass");
 *  _bigend = new OptionLiteral("-bigendian");
 *  _littleend = new OptionLiteral("-littleendian");
 *  OptionLoop *opt = new OptionSelection();
 *  opt->add(_bitend);
 *  opt->add(_littleend);
 *  _command_line->add(opt);
 *  \endcode

 *  \code
 *  // This will add optional "-keep" and "-verbose" switches
 *  Module::initialize();
 *  _command_line -> set_description("description of the pass");
 *  _keep = new OptionLiteral("-keep");
 *  _verbose = new OptionLiteral("-verbose");
 *  OptionSelect *select_opt = new OptionSelection();
 *  select_opt->add(_keep);
 *  select_opt->add(_verbose);
 *  OptionLoop *loop_opt = new OptionLoop(select_opt);
 *  _command_line->add(loop_opt);
 *  \endcode
 *
 */


typedef suif_vector<Option*> Options;
enum AllowEmpty { DontAllowEmptySelection, AllowEmptySelection };


/**
 * A wrapper of a value for an option.
 * Each instance of ValueClass holds one value.
 * This class is an abstract class.
 *
 * A ValueClass has the following components"
 *  parent - the parent value
 *  owning_option - the option of which this ValueClass is a value.
 */
class ValueClass {
  friend class Option;
public:
  virtual ~ValueClass();
protected:
  ValueClass( ValueClass* parent, Option* owning_option );


  ValueClass* _parent;
  Option* _owning_option;
};



class StructureValueClass : public ValueClass {
  friend class OptionLoop;
  friend class OptionSelection;
  friend class OptionList;
protected:
  StructureValueClass( ValueClass* parent, Option* owning_option );
};


/**
 * An Option that will bind to any string token
 * during parsing
 */


class StringValueClass : public ValueClass {
  friend class OptionString;
  friend class OptionMultiString;
  friend class OptionLiteral;
  friend class OptionPrefixString;
public:
  virtual const String get_string() const;
  virtual ~StringValueClass();
protected:
  StringValueClass( Token& t, ValueClass* parent, Option* owning_option );
  Token* sf_owned _token;
};




/**
 * An IntValueClass holds an integer value.
 * It is the caller's responsibility to make sure that the token used to
 *  construct this object holds a string representation of an integer.
 * This class will use atoi() to convert and will not perform any error
 *  detection or reporting.
 */
class IntValueClass : public ValueClass {
  friend class OptionInt;
public:
  virtual ~IntValueClass();
  virtual int get_int() const;
protected:
  IntValueClass( Token& t, ValueClass* parent, Option* owning_option );
  Token* _token;
};



/**
  * A StreamValueClass will simply hold an istream.
  */
class StreamValueClass : public ValueClass {
  friend class OptionStream;
public:
  virtual std::istream& get_stream() const;
protected:
  StreamValueClass( std::istream* stream,
                    ValueClass* parent,
		     Option* owning_option );
private:
  std::istream* _stream;
};

/**
 * \class Option command_line_parsing.h suifkernel/command_line_parsing.h
 * An Option represents an argument or a flag (like -I) in a unix
 *  command.
 * One can build a option tree with option instances as nodes
 *  to represent the grammar for the acceptable options.
 *  See the subclasses of Option for ways to construct the tree.
 *
 * Each option may have multiple values associated with it.
 *
 * Each option has the following component:
 *  parent - the parent option in the option tree.
 *  value  - the values associated with this option, represented as
 *           a list of ValueClass instances.
 *  group  - name associated with the collection of options
 *  argument - the name of this option, used in help message
 *  description - help message for this option.
 */
class Option {
public:
  /** parses the command line and returns true if succeeded */
   virtual bool parse_options( std::istream& input_stream,
			       bool &changed,
			       ErrorSubSystem* output_for_errors );

   virtual bool parse_options( int argc, char *argv[],
			       bool &changed,
                               ErrorSubSystem* output_for_errors );

   virtual void print_to_stream( std::ostream& print_stream ) const;
   virtual String to_string() const;
   virtual void print_debug() const;

  /**
   * returns true if the token stream parsing does not fail
   * at this option.
   *  sets the changed parameter to true if the option parsing
   *  generated some state.
   * The changed token is important in loops because
   *  OptionSelections can succeed without actually consuming
   *  any of the token stream.
   */
   virtual bool parse( TokenStream* tokens, bool &changed,
		       ValueClass* parent = 0) = 0;

   virtual int get_number_of_values() const;

   virtual void delete_values();

  /** Prepare the printable description of this option.
   * Append a printed message to command_line_string, and
   * a description to descriptions.
   */
   virtual void print( String& command_line_string,
                       suif_vector<OptionDescription>* descriptions ) const;

  /** Set the name of the option to be printed out in a help message */
   virtual void set_argument( const String& argument );

   /** Set the description which explains what the option is     */
   virtual void set_description( const String& description,
                                 const String& group = emptyString );

  virtual String get_description() const;

   virtual ~Option();

protected:
   Option( String arguments = emptyString,
           Option* parent = 0);
   Option( String arguments, String description,
           Option* parent);

//   virtual Option* get_parent() { return _parent; }

    virtual ValueClass* get_value_class( ValueClass* value,
                                         Option* context ) const;


   String _description;
   String _group;
   String _argument;
   Option* _parent;
   suif_vector<sf_owned ValueClass*>* sf_owned _values;
};



class OptionString : public Option {
public:
  /** Construct with the name of this option and
   *  a memory location to hold a string.
   * It will match any string token in the command line
   * when parse is called
   */
   OptionString( String argument = String( "string" ),
                 String* storage_for_string = 0 );

   virtual ~OptionString();

   virtual const StringValueClass* get_string( int number = 0 ) const;
   virtual StringValueClass* get_string( ValueClass* value,
                                         Option* context ) const;
  
   /** Parse a token for an integer.
    * Returns true if successful, false otherwise.
    */
   virtual bool parse( TokenStream* tokens, bool &changed, 
		       ValueClass* parent = 0);

private:
   String* _storage_for_string;
};

// This option will match any string
// that begins with the "prefix"
class OptionPrefixString : public Option {
public:
  /** Construct with the name of this option and
   *  a memory location to hold a string.
   * It will match any string token in the command line
   * when parse is called
   */
   OptionPrefixString( String prefix,
		       String argname,
		       String description,
		       String* storage_for_string = 0 );

   virtual ~OptionPrefixString();

   virtual const StringValueClass* get_string( int number = 0 ) const;
   virtual StringValueClass* get_string( ValueClass* value,
                                         Option* context ) const;

/** Parse a token for an integer.
   * Returns true if successful, false otherwise.
   */
   virtual bool parse( TokenStream* tokens, bool &changed, 
                       ValueClass* parent = 0 );

private:
  // for -Ddefine the _argument "-D" is the part to match
  String _argname;  // for -Ddefine  this would be "define"
  String* _storage_for_string;
};


/**
 * An OptionMultiString is just like an OptionString
 * but each match of it will push a new string onto the storage
 */
class OptionMultiString : public Option {
public:
/** Construct with the name of this option and
   *  a memory location to hold a string.
   */
   OptionMultiString( String argument = String( "multi-string" ),
		      suif_vector<String> *storage_for_string = 0 );

   virtual ~OptionMultiString();

   virtual const StringValueClass* get_string( int number = 0 ) const;
   virtual StringValueClass* get_string( ValueClass* value,
                                         Option* context ) const;

  /** Parse a token for an integer.
   * Returns true if successful, false otherwise.
   */
   virtual bool parse( TokenStream* tokens, bool &changed,
		       ValueClass* parent = 0 );

private:
  suif_vector<String> *_storage_for_string;
};


/**
 * An OptionInt matches an integer in the command line.
 */
class OptionInt : public Option {
public:

/** Construct with a name for this option and
   * a location to hold an int.
   */
   OptionInt( String arguments = String("int"),
              int* storage_for_integer = 0 );

   virtual ~OptionInt();

   virtual IntValueClass* get_int( int number = 0 );
   virtual IntValueClass* get_int( ValueClass* value,
                                   Option* context ) const;

   virtual bool parse( TokenStream* tokens, bool &changed,
                       ValueClass* parent = 0 );

private:
  int* _storage_for_default_value;
};



/**
 * An OptionLiteral matches a literal string in the command line.
 */
class OptionLiteral : public Option {
public:

/** Construct with a name for this option and,
   * a location for a boolean value, and
   * a boolean to set the location if a match is found.
   */
   OptionLiteral( String argument,
                  bool* storage_for_is_set = 0,
                  bool default_value = false );
   OptionLiteral( String argument,
		  String description,
                  bool* storage_for_is_set = 0,
                  bool default_value = false );
   virtual ~OptionLiteral();

/** Returns true if a successful match is made.
   * Independent of the value set to storage_for_is_set.
   */
   virtual bool is_set() const;
   virtual bool is_set ( ValueClass* value,
                                   Option* context ) const;

   virtual bool parse( TokenStream* tokens, bool &changed,
                       ValueClass* parent = 0 );

   virtual void print( String& command_line_string,
                       suif_vector<OptionDescription>* descriptions ) const;

private:
  bool* _storage_for_is_set;
  bool _default_value;
};




/**
 * An OptionSelection is a collection of sub-options.
 * It matches with one of the sub-options.
 */
class OptionSelection : public Option {
public:
  OptionSelection( bool allow_empty = false );
  OptionSelection( Option *opt1, bool allow_empty );
  // add more options with "add"
  OptionSelection( Option *opt1, Option *opt2, bool allow_empty );

  virtual ~OptionSelection();

  virtual OptionSelection* add( Option* );

  virtual bool parse( TokenStream* tokens, bool &changed,
		      ValueClass* parent );

  virtual void delete_values();

  virtual void print( String& command_line_string,
                       suif_vector<OptionDescription>* descriptions ) const;

protected:
  bool _allow_empty;
  Options* sf_owned _selection_list;
};



/**
 * An OptionList represents a sequence of sub-options.
 * It matches with all the options in the sequence.
 */
class OptionList : public Option {
public:
  OptionList();
  OptionList(Option *opt1);
  OptionList(Option *opt1, Option *opt2);

  virtual ~OptionList();

  virtual OptionList* add( Option* );

  virtual bool parse( TokenStream* tokens, bool &changed,
		       ValueClass* parent = 0 );

  virtual void delete_values();

  virtual void print( String& command_line_string,
                       suif_vector<OptionDescription>* descriptions ) const;

private:
  suif_vector<sf_owned Option*>* sf_owned _option_list;
};




/**
 * An OptionLoop represents a Kleene closure; i.e. a repeated
 *  application of a sub-option.
 */
class OptionLoop : public Option {
public:
  OptionLoop( Option* loop_contents,
              bool allow_empty = true );

  virtual ~OptionLoop();

  bool parse( TokenStream* tokens, bool &changed,
              ValueClass* parent );

  virtual void delete_values();

  virtual void print( String& command_line_string,
                       suif_vector<OptionDescription>* descriptions ) const;

private:
  Option sf_owned * _loop_contents;
  bool _allow_empty;
};



/**
 * An OptionStream matches the part of command line enclosed between
 *  a pair of braces ({}).
 */
class OptionStream : public Option {
public:
  OptionStream( String argument = emptyString,
               std::istream** storage_for_stream = 0 );
  virtual ~OptionStream();

  bool parse( TokenStream* tokens, bool &changed,
	      ValueClass* parent );

private:
  std::istream** _storage_for_stream;
};


/** A convenient function to create an option tree that matches
   * a literal followed by a string.  E.g.  "-I /usr/include".
   *
   * param prefix - the literal string.
   * param argument - the name of the string argument.
   * param storage - the location to store the string argument.
   * param description - the description of this compound option.
   */
Option* build_prefixed_string( String prefix, String argument,
			       String* storage,
			       String description = emptyString);

/** A convenient function to create an option tree that matches
   * any set of strings.  e.g.
   * "filename1 filename2 filename3"
   *
   * param argument - the name of the string argument.
   * param storage - the location to store the string arguments.
   * param description - the description of this compound option.
   */
Option* build_multi_string( String argument,
                            suif_vector<String> &storage,
                            String description = emptyString);

#endif



