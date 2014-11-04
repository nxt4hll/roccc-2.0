#ifndef SUIFPASSES__STANDARD_MODULES_H
#define SUIFPASSES__STANDARD_MODULES_H

#include "suifkernel/module.h"
#include "suifkernel/suifkernel_forwarders.h"

class PrintModule : public Module {
public:
  PrintModule( SuifEnv* suif_env );

  virtual void execute();
  virtual void initialize();

  virtual Module* clone() const;

  static const LString get_class_name();
private:
  static void print_dispatch(Module *, std::ostream &, const ObjectWrapper &);

  OptionLiteral *_is_raw; // print with the raw format
  String _style; // print with a particular style
  String _filename; // output file. If emptyString use stdout
  OptionLiteral *_do_list; // print the list of styles
};


class SaveModule : public Module {
public:
  SaveModule( SuifEnv* suif_env );
  virtual ~SaveModule();


  virtual void initialize();

  virtual void execute();

  virtual Module* clone() const;

  static const LString get_class_name();
private:
  OptionString* _file_name_argument;
};



class ImportModule : public Module {
public:
  ImportModule( SuifEnv* suif );
  virtual ~ImportModule();

  virtual void initialize();

  virtual Module* clone() const;

  virtual void execute();

  static const LString get_class_name();

private:
  OptionLoop* _repetition;
  OptionString* _module_name_argument;
};

class RequireModule : public Module {
public:
  RequireModule( SuifEnv* suif );
  virtual ~RequireModule();

  virtual void initialize();

  virtual Module* clone() const;

  virtual void execute();

  static const LString get_class_name();

private:
  OptionLoop* _repetition;
  OptionString* _module_name_argument;
};



class XLoadModule : public Module {
public:
  XLoadModule( SuifEnv* suif_env );
  virtual ~XLoadModule();

  virtual void initialize();

  virtual void execute();

  virtual Module* clone() const;

  static const LString get_class_name();
private:
  OptionString* _file_name_argument;
};

class ListModulesModule : public Module {
public:
  ListModulesModule( SuifEnv* suif, 
		    const LString &name = "list_modules" );
  ~ListModulesModule();
  
  virtual void initialize();
  
  virtual Module* clone() const;
  
  virtual void execute();
  
  static const LString get_class_name();
protected:
  String _interface;
};

class ListInterfacesModule : public Module {
public:
  ListInterfacesModule( SuifEnv* suif, 
		     const LString &name = "list_interfaces" );
  ~ListInterfacesModule();
  
  virtual void initialize();
  
  virtual Module* clone() const;
  
  virtual void execute();
  
  static const LString get_class_name();
};


#endif









