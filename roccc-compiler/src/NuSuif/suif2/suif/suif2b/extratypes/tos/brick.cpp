#include "brick.h"
#include "basicnodes/basic_factory.h"
#include "basicnodes/basic.h"

Brick::Brick( SuifEnv *env,const String& value ) : 
  _object(create_str_brick(env,value))
{}

Brick::Brick( SuifEnv *env,const char * value) : 
  _object(create_str_brick(env,String(value)))
{}


Brick::Brick( SuifEnv *env,const int value ) : 
  _object(create_integer_brick(env,IInteger(value)))
{}

Brick::Brick( SuifEnv *env,const IInteger & value ) :
  _object(create_integer_brick(env,value))
{}


    
Brick::Brick( SuifEnv *env,SuifObject * obj, bool is_owned ) 
{
  if (is_owned) 
    _object = create_owned_suif_object_brick(env,obj);
  else
    _object = create_suif_object_brick(env,obj);
}

Brick::Brick() : _object(NULL) {}

int Brick::get_kind() const { 
    if (_object == NULL)
	return EmptyBrick;
    if (is_kind_of<StrBrick>(_object))
    	return StringBrick;
    if (is_kind_of<IntegerBrick>(_object))
        return IIntegerBrick;
    if (is_kind_of<OwnedSuifObjectBrick>(_object))
        return OwnedObject;
    if (is_kind_of<SuifObjectBrick>(_object))
        return ReferencedObject;
    suif_assert(false);
    return -1; // fake out compiler
    }

bool Brick::is_string() const { 
    return ((_object != NULL) && is_kind_of<StrBrick>(_object));
    }

void Brick::set_string( SuifEnv *env,const String &value ) {
  clear(); 
  _object = create_str_brick(env,String(value)); 
  }

IInteger Brick::get_i_integer() const {
  suif_assert(is_i_integer());
  return to<IntegerBrick>(_object)->get_value();
  }

bool Brick::is_i_integer() const {
    return ((_object != NULL) && is_kind_of<IntegerBrick>(_object));
    }

void Brick::set_i_integer( SuifEnv *env,const IInteger &value ) {
  clear(); 
  _object = create_integer_brick(env,IInteger(value));
  }

bool Brick::is_owned_object() const {
    return ((_object != NULL) && is_kind_of<OwnedSuifObjectBrick>(_object));
    }

void Brick::set_owned_object( SuifEnv *env,SuifObject* owned_object ) {
  clear(); 
  _object = create_owned_suif_object_brick(env,owned_object);
  }

SuifObject* Brick::get_owned_object() const {
  suif_assert(is_owned_object());
  return to<OwnedSuifObjectBrick>(_object)->get_object();
  }

bool Brick::is_referenced_object() const {
    return ((_object != NULL) && is_kind_of<SuifObjectBrick>(_object));
    }

void Brick::set_referenced_object( SuifEnv *env,SuifObject* referenced_object ) {
  clear(); 
  _object = create_suif_object_brick(env,referenced_object);
  }

SuifObject* Brick::get_referenced_object() const {
  suif_assert(is_referenced_object());
  return to<SuifObjectBrick>(_object)->get_object();
  }

SuifObject* Brick::get_object() const {
  if (is_referenced_object())
	return get_referenced_object();
  else
	return get_owned_object(); // willa assert if not
  }

bool Brick::is_object() const {
  return is_referenced_object() || is_owned_object(); }

Brick Brick::deep_clone( SuifEnv* suif_env ) const
 {
    Brick return_brick;
    //    suif_env->get_clone_subsystem()->
    //         deepClone( &this, brick_meta_class, &return_brick._storage );
    return return_brick;
  }

Brick Brick::shallow_clone( SuifEnv* suif_env ) const {
  Brick return_brick;
  //  suif_env->get_clone_subsystem()->
  //    shallowClone( this, brick_meta_class, &return_brick );
  return return_brick;
}

void Brick::clear() {
    // delete it unless it is a referenced object
    if ((_object != NULL) && !is_kind_of<SuifObjectBrick>(_object))
        delete _object;
    _object = NULL;
}

Brick &Brick::operator=(const Brick &other) {
  clear();
  if (other.is_owned_object()) // we must clone it
    _object = other.get_owned_object()->shallow_clone(); 
		// is shallow clone enough?
  else
    _object = other._object;
  return(*this);
  }

bool Brick::operator==(const Brick &other) const {
  int kind = get_kind();
  if (kind != other.get_kind()) return false;
  switch (kind ) {
    case EmptyBrick:  
        return(true);
    case StringBrick: 
        return (get_string() == other.get_string());
    case IIntegerBrick:
        return (get_i_integer() == other.get_i_integer());
    case OwnedObject:
        return (get_owned_object() == other.get_owned_object());
    case ReferencedObject:
        return (get_referenced_object() == other.get_referenced_object());
    }
  suif_assert(0);
  return false;
  }

void Brick::print(FormattedText &x) const {
  int kind = get_kind();
  switch (kind ) {
    case EmptyBrick: 
      {   
        x.start_block("value =");
        x.set_value("NULL");
        x.end_block();
      }
      break;
    case StringBrick: 
      {
        x.start_block("String ");
        x.set_value(get_string());
        x.end_block();
      }
      break;
    case IIntegerBrick:
      {
        x.start_block("IInteger ");
        x.set_value(get_i_integer());
        x.end_block();
      }
      break;
    case OwnedObject:
      {
        x.start_block("OwnedObject ");
        SuifObject *obj = get_owned_object();
        if (obj) obj->print(x);
        x.end_block();
      }
      break;
  
    case ReferencedObject:
      {
        x.start_block("ReferencedObject ");
        SuifObject *obj = get_referenced_object();
        if (obj) obj->print(x);
        x.end_block();
      }
      break;
    default:
      suif_assert(0);
    }
  }

Brick::~Brick() {
  clear();
}

