#ifndef SUIFKERNEL__PRINT_SUBSYSTEM_H
#define SUIFKERNEL__PRINT_SUBSYSTEM_H

#include "subsystem.h"
#include "suifkernel_forwarders.h"
#include "common/suif_vector.h"
#include <stdio.h>

/**
 * A class for initializing print strings by table.
 */
struct PrintSpecClass {
  const char *class_name;
  const char *print_spec;
};

/** This is a map of MetaClass names to print specification strings
 *
 * Because there may be more than one interpreter, each interpreter
 * is named by it's module name and these tables may be looked
 * up by that.
 */


class PrintStringRepository {
  
public:
  PrintStringRepository();
  ~PrintStringRepository();
  String get_print_ref_string(const LString &key) const;
  String get_print_string(const LString &key) const;

  void set_print_ref_string(const LString &key, const String &value);
  void set_print_string(const LString &key, const String &value);

  void set_print_ref_string_by_table(const PrintSpecClass *table,
				     size_t size);
  void set_print_string_by_table(const PrintSpecClass *table,
				 size_t size);

  bool has_print_ref_string(const LString &key) const;
  bool has_print_string(const LString &key) const;

private:
  suif_hash_map<LString, String> *_print_full;
  suif_hash_map<LString, String> *_print_ref;
};


typedef void ( *PrintDispatch )( Module *, std::ostream &, const ObjectWrapper &);

/** The print subsystem is the default printer for SUIF
 *
 * When a default printer is initialized, it should
 * set the default print style to its module name.
 *
 * If no default print style has been set we
 * use the SuifObject::print(FormattedText &)
 * function for printing suif objects and just print an
 * error for everything else.
 *
 * The print subsystem also contains maps indexed by
 * style of specification strings for each metaclass.
 */

class PrintSubSystem : public SubSystem {
public:
  PrintSubSystem( SuifEnv* suif_env );
  virtual ~PrintSubSystem();
  /**
   * Most explicit print interface.
   * The user chooses the "style"
   * and the output stream
   * The default implementation will find a module with the
   * name "style", make sure it implements the print interface
   * then dispatch to the print routine.
   * The default implementation of ALL of the other
   * print methods will ultimately use this method.
   */
  virtual void print( const LString &style, 
		      std::ostream& output, 
		      const ObjectWrapper &obj) const;
  virtual void print( const LString &style, 
		      std::ostream& output, 
		      SuifObject *obj) const;
  virtual String print_to_string(const LString &style,
				 const ObjectWrapper &obj) const;
  virtual String print_to_string(const LString &style,
				 SuifObject *obj) const;

  /**
   * print with the default style to the specified stream
   */
  virtual void print(std::ostream& output, 
		      const ObjectWrapper &obj) const;

  virtual void print(FILE *fp, const ObjectWrapper &obj) const;

  virtual String print_to_string(const ObjectWrapper &obj) const;

  /**
   * Print any object that participates in the metaclass system
   * to the default output stream with the default style
   */
  virtual void print(const ObjectWrapper &obj ) const;
  /**
   * Print a SuifObject to the default output stream with the default style
   */
  virtual void print(SuifObject *obj) const;
  /**
   * Print a SuifObject to a string with the default style
   */
  virtual String print_to_string(SuifObject *obj) const;
  
  
  virtual std::ostream& get_default_stream() const;

  virtual void set_default_print_style(const LString &style);
  virtual LString get_default_print_style() const;

  virtual PrintStringRepository *retrieve_string_repository(const LString &style);

private:
  LString _style;
  typedef suif_hash_map<LString, PrintStringRepository*> StringMap;
  StringMap *_string_map;

  PrintSubSystem(const PrintSubSystem &);
  PrintSubSystem& operator=(const PrintSubSystem &);
};


#endif
