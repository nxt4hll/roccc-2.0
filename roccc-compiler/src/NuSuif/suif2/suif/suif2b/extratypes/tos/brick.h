#ifndef BASICNODES__BRICK_H
#define BASICNODES__BRICK_H

#include <iokernel/iokernel_forwarders.h>
#include <common/i_integer.h>
#include <common/formatted.h>
#include <basicnodes/basic_forwarders.h>

// Ownership of a SuifObject only effects
// cloning, not deletion
//
// This is a proxy for the Brick class from SUIFX

class Brick {
public:
  friend class SuifEnv;

  static const LString &get_class_name();

  typedef enum {EmptyBrick = 0, StringBrick = 1, IIntegerBrick = 2,
		OwnedObject = 3, ReferencedObject = 4 } brick_enum;

  Brick( SuifEnv *env,const String& value );
  Brick( SuifEnv *env,const char * value);
  Brick( SuifEnv *env,const int value );
  Brick( SuifEnv *env,const IInteger & value );
  Brick( SuifEnv *env,SuifObject * obj,bool owned);
  Brick();


  Brick &operator=(const Brick &other);
  bool operator==(const Brick &other) const;

  int get_kind() const;
  bool is_string() const;
  void set_string( SuifEnv *env,const String &string );
  String get_string() const;

  bool is_i_integer() const;
  void set_i_integer(SuifEnv *env, const IInteger &i_integer );
  IInteger get_i_integer() const;


  bool is_owned_object() const;
  void set_owned_object(SuifEnv *env, SuifObject* object );
  SuifObject* get_owned_object() const;

  bool is_referenced_object() const;
  void set_referenced_object( SuifEnv *env,SuifObject* object );
  SuifObject* get_referenced_object() const;

  // These are shortcuts when the user doesn't
  // care about ownership of a SuifObject
  bool is_object() const;
  SuifObject* get_object() const;

  Brick deep_clone( SuifEnv* suif_env ) const;
  Brick shallow_clone( SuifEnv* suif_env ) const;

  void print(FormattedText &) const;
  ~Brick();


  
private:
  void clear();

private:
  SuifObject *_object;
};

#endif /* BRICK_H */
