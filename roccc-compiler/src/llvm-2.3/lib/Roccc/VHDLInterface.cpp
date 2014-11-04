#include "rocccLibrary/VHDLInterface.h"

#include <assert.h>
#include <sstream>
#include <algorithm>
#include <iomanip>

#include "rocccLibrary/Version.h"
#include "rocccLibrary/InternalWarning.h"

#ifdef LLVM_VALUE_H
//workaround to find signed/unsigned
#include "rocccLibrary/IsValueSigned.h"
#endif


namespace VHDLInterface {

//copied from http://www.jb.man.ac.uk/~slowe/cpp/itoa.html
std::string itoa(int value, unsigned int base) {
	const char digitMap[] = "0123456789abcdef";
	std::string buf;
	// Guard:
	if (base == 0 || base > 16) {
		assert(0 and "Invalid base!");
		return buf;
	}
	unsigned int _value = static_cast<unsigned int>(value);
	// Check for case when input is zero:
	if (_value == 0) return "0";
	// Translating number to string with base:
	do {
		buf = digitMap[ _value % base ] + buf;
  } while (_value /= base);
	if( value < 0 )
	{
	  assert( buf[0] == '1' );
	  //truncate off unneccesary leading '1's
	  for(unsigned i = 0; i < buf.length(); ++i)
	  {
	    if( buf[i] == '0' )
	      return buf.substr(i-1);
	  }
	  //its negative 0; just return 1.
	  return "1";
	}
	assert(buf.length() <= 32);
	return buf;
}

bool isValidVHDLIdentifierName(std::string name)
{
  if( name[0] == '_' )
  {
    INTERNAL_ERROR("\'" << name << "\' is not a valid VHDL identifier; cannot start with \'_\'!\n");
    return false;
  }
  if( name[0] == '0' or
      name[0] == '1' or
      name[0] == '2' or
      name[0] == '3' or
      name[0] == '4' or
      name[0] == '5' or
      name[0] == '6' or
      name[0] == '7' or
      name[0] == '8' or
      name[0] == '9' )
  {
    INTERNAL_ERROR("\'" << name << "\' is not a valid VHDL identifier; cannot start with a number!\n");
    return false;
  }
  if(name.find(" ") != std::string::npos)
  {
    INTERNAL_ERROR("\'" << name << "\' is not a valid VHDL identifier; cannot include spaces!\n");
    return false;
  }
  if(name.find("__") != std::string::npos)
  {
    INTERNAL_ERROR("\'" << name << "\' is not a valid VHDL identifier; cannot include double underscore!\n");
    return false;
  }
  if( name.length() == 0 )
  {
    INTERNAL_ERROR("\'" << name << "\' is not a valid VHDL identifier!\n");
    return false;
  }
  return true;
}

void Value::setOwner(ValueOwner* v) //from Value
{
  if( owner != NULL and !owner->canTransferOwnershipOfTo(this,v) )
  {
    assert( v );
    INTERNAL_ERROR(owner->getName() << " already owns " << getName() << ", and setOwner(" << v->getName() << ") called!\n");
  }
  assert( (owner == NULL or owner->canTransferOwnershipOfTo(this,v)) and "Cannot set owner on already owned Value!" );
  owner = v;
}
ValueOwner* Value::getOwner() //from Value
{
  return owner;
}
Value::Value() : owner(NULL)
{
}
std::string Value::generateDefaultCode()
{
  return generateCode(getSize());
}
std::string Value::generateAssignmentOperator()
{
  return "<=";
}
std::string Value::getName() //from Value
{
  return getInternalName();
}
std::string Value::getInternalName() //from Value
{
  assert(0 and "Cannot getInternalName() on a nameless value!");
}
bool Value::isOwned()
{
  return (getOwner() != NULL);
}
Value::~Value()
{
}

Attribute::Attribute(std::string n) : name(n)
{
}
Attribute::~Attribute()
{
}
std::string Attribute::getName()
{
  return name;
}
VHDLAttribute::VHDLAttribute(std::string n, std::string t) : Attribute(n), type(t), isDeclared(false)
{
}
std::string VHDLAttribute::generateDeclarationCode(Value* v, std::string value)
{
  std::stringstream ss;
  if( !isDeclared )
  {
    ss << "attribute " << getName() << " : " << type << ";\n";
    isDeclared = true;
  }
  assert(v);
  ss << "attribute " << getName() << " of " << v->getName() << " : signal is " << value << ";\n";
  return ss.str();
}
Attributeable::Attributeable()
{
}
Attributeable::~Attributeable()
{
}
void Attributeable::addAttribute(Attribute* a, std::string v)
{
  assert(a);
  attributes[a] = v;
}
std::string Attributeable::getAttributeValue(Attribute* att)
{
  if( att == NULL or attributes.find(att) == attributes.end() )
    return "";
  return attributes[att];
}
std::string Attributeable::generateAttributeCode()
{
  hasCalledGenerate = true;
  std::stringstream ss;
  for(std::map<Attribute*, std::string>::iterator AI = attributes.begin(); AI != attributes.end(); ++AI)
  {
    VHDLAttribute* attribute = dynamic_cast<VHDLAttribute*>(&*AI->first);
    if( attribute )
    {
      Value* v_this = dynamic_cast<Value*>(this);
      assert(v_this);
      ss << attribute->generateDeclarationCode(v_this, AI->second);
    }
  }
  return ss.str();
}


std::ostream& operator << (std::ostream& os, Value* v)
{
  assert( v );
  os << v->generateDefaultCode();
  return os;
}

Variable::Variable(int s, Variable::T* v) : size(s), value(v), is_declared(false)
{
  assert( size > 0 );
}
bool Variable::isUnsigned()
{
#ifdef LLVM_VALUE_H
  if( getLLVMValue() )
  {
    return isValueUnsigned(getLLVMValue());
  }
  else
  {
    INTERNAL_WARNING(getName() << " has no llvm value, assumed unsigned!\n");
    return true;
  }
#else
  ImplementCustomSolutionHere();
#endif
}
void Variable::setSize(int s)
{
  size = s;
}
int Variable::getSize() //from Value
{
  return size;
}
Variable::T* Variable::getLLVMValue()
{
  return value;
}
std::string Variable::generateCode(int size) //from Value
{
  if( !is_declared )
  {
    INTERNAL_ERROR(getName() << " has not been declared!\n");
    assert( 0 and "Must generateDeclarationCode() before using variable!" );
  }
  if( !getOwner() )
  {
    Port* p = dynamic_cast<Port*>(this);
    if( !p or p->getType() == Port::OUTPUT )
    {
      INTERNAL_ERROR(getName() << " is not being driven!\n");
      assert( 0 and "Variable is not being driven! Did you make sure to setOwner()?" );
    }
  }
  assert( size > 0 or getSize() == size );
  std::stringstream ss;
  std::string name = getName();
  if( getSize() > size )
  {
    if( size == 1 )
      ss << name << "(0)";
    else
      ss << name << "(" << size - 1 << " downto 0)";
  }
  else if ( getSize() < size )
  {
    INTERNAL_MESSAGE("Forced to extend variable " << getName());
    ss << "(";
    if( isUnsigned() )
    {
      INTERNAL_MESSAGE(", using unsigned.\n");
      if( getSize() + 1 == size )
        ss << "\'";
      else
        ss << "\"";
      for(int i = getSize(); i < size; ++i)
        ss << "0";
      if( getSize() + 1 == size )
        ss << "\'";
      else
        ss << "\"";
      ss << " & ";
    }
    else
    {
      INTERNAL_MESSAGE(", using signed.\n");
      //equivalent to:
      //bitwise_concat(BitRange::get(this,getSize()-1,getSize()-1), this);
      for(int i = getSize(); i < size; ++i)
      {
        if( getSize() == 1 )
          ss << name;
        else
          ss << name << "(" << getSize() - 1 << ")";
        ss << " & ";
      }
    }
    ss << name << ")";
  }
  else
    ss << name;
  return ss.str();
}
Value* Variable::generateResetValue()
{
  return ConstantInt::get(0);
}
void Variable::declare()
{
  is_declared = true;
}

class BitRangeImpl : public Variable, public HasReplaceAllUsesOfWith {
  Variable* val;
  int high;
  int low;
  struct Triple {
    Variable* val;
    int high;
    int low;
    Triple(Variable* v, int h, int l) : val(v), high(h), low(l) {}
    bool operator < (const Triple rhs) const
    {
      if( val < rhs.val )
        return true;
      if( high < rhs.high )
        return true;
      if( low < rhs.low )
        return true;
      return false;
    }
  };
  static std::map<Triple, BitRangeImpl*> pool;
  bool rangeOverlaps(int h1, int l1, int h2, int l2);
protected:
  virtual void setOwner(ValueOwner* v); //from Value
  virtual HasReplaceAllUsesOfWith* cloneImpl() //from HasReplaceAllUsesOfWith
  {
    //TODO: implement
    return this;
  }
public:
  BitRangeImpl(Variable* v, int h, int l);
  virtual  int getSize(); //from Value
  virtual std::string generateCode(int size); //from Value
  virtual std::string getName(); //from Value
  virtual std::string getInternalName(); //from Value
  virtual void setReadFrom(); //from Value
  virtual void setWrittenTo(); //from Value
  virtual void replaceAllUsesOfWith(Value* of, Value* with) //from HasReplaceAllUsesOfWith
  {
    //TODO: implement
  }
  friend VWrap BitRange::get(Variable* v, int h, int l);
  friend class BitRangeOwner;
};

BitRangeImpl::BitRangeImpl(Variable* v, int h, int l) : Variable(v->getSize(), v->getLLVMValue()), val(v), high(h), low(l)
{
  assert( v );
  assert( v->getSize() > 1 ); //only std_logic_vectors can be bitranged
  assert( high >= low );
  assert( low >= 0 );
  assert( high < v->getSize() );
  assert( dynamic_cast<BitRange*>(v) == NULL and "Cannot bitrange a bitrange!");
}
bool BitRangeImpl::rangeOverlaps(int h1, int l1, int h2, int l2)
{
  if( (l1 <= h2 and h2 <= h1) or
      (l2 <= h1 and h1 <= h2) )
  {
    return true;
  }
  return false;
}
VWrap BitRange::get(Variable* v, int h, int l)
{
  if( v->getSize() == 1 and h == 0 and l == 0 )
    return VWrap(v);
  if( BitRangeImpl::pool.find(BitRangeImpl::Triple(v,h,l)) != BitRangeImpl::pool.end() )
    return VWrap(BitRangeImpl::pool.find(BitRangeImpl::Triple(v,h,l))->second);
  BitRangeImpl* ret = new BitRangeImpl(v,h,l);
  BitRangeImpl::pool[BitRangeImpl::Triple(v,h,l)] = ret;
  return VWrap(ret);
}
 int BitRangeImpl::getSize() //from Value
{
  return high - low + 1;
}
std::string BitRangeImpl::generateCode(int size) //from Value
{
  int h = high;
  int l = low;
  if( size < getSize() )
  {
    h = size - 1 + l;
    INTERNAL_WARNING("Shrinking explicit BitRange!\n" <<
                      "  BitRange  (" << high << ", " << low << "), size = " << getSize() << "\n"
                      "  shrunk to (" << h << ", " << l << "), size = " << size << "\n");
  }
  std::stringstream ss;
  if( getSize() < size )
  {
    if( getSize() + 1 == size )
      ss << "\'";
    else
      ss << "\"";
    for(int i = getSize(); i < size; ++i)
      ss << "0";
    if( getSize() + 1 == size )
      ss << "\'";
    else
      ss << "\"";
    ss << " & ";
  }
  assert( h >= l and l >= 0 );
  if( h == l )
    ss << val << "(" << h << ")";
  else
    ss << val << "(" << h << " downto " << l << ")";
  return ss.str();
}
std::string BitRangeImpl::getName() //from Value
{
  return val->getName();
}
std::string BitRangeImpl::getInternalName() //from Value
{
  return val->getInternalName();
}
void BitRangeImpl::setReadFrom() //from Value
{
  val->setReadFrom();
}
void BitRangeImpl::setWrittenTo() //from Value
{
  val->setWrittenTo();
}
class BitRangeOwner : public ValueOwner {
public:
  BitRangeOwner() : ValueOwner() {}
  virtual std::string getName(){return "BITRANGE_OWNER";}
  bool canTransferOwnershipOfTo(Value* v, ValueOwner* vo); //from ValueOwner
};
bool BitRangeOwner::canTransferOwnershipOfTo(Value* v, ValueOwner* vo)
{
  assert( owns(v) );
  assert( vo );
  for(std::map<BitRangeImpl::Triple, BitRangeImpl*>::iterator PI = BitRangeImpl::pool.begin(); PI != BitRangeImpl::pool.end(); ++PI)
  {
    if( v == PI->second->val and PI->second->isOwned() and !vo->owns(PI->second) )
    {
      return false;
    }
  }
  return true;
}
void BitRangeImpl::setOwner(ValueOwner* v) //from Value
{
  static BitRangeOwner BITRANGE_OWNER;
  if( val->getOwner() != v )
    BITRANGE_OWNER.add(val);
  Value::setOwner(v);
  for(std::map<Triple, BitRangeImpl*>::iterator PI = pool.begin(); PI != pool.end(); ++PI)
  {
    if( val == PI->second->val and rangeOverlaps(high,low, PI->first.high, PI->first.low) )
    {
      if( PI->second->isOwned() and !Value::getOwner()->owns(PI->second) )
      {
        INTERNAL_ERROR("Error while setting owner " << v->getName() << " = " << v << ":\n"
               << getName() << "(" << high << ", " << low << ") {with owner " << Value::getOwner()->getName() << " = " << Value::getOwner() << "} is not same owner as "
               << getName() << "(" << PI->first.high << ", " << PI->first.low << ") {with owner " << PI->second->getOwner()->getName() << " = " << PI->second->getOwner() << "}\n"
               << "In " <<  getName() << "(" << high << ", " << low << ")->getOwner():\n");
        for(std::vector<Value*>::iterator val = Value::getOwner()->vals.begin(); val != Value::getOwner()->vals.end(); ++val)
        {
          if(BitRangeImpl* br = dynamic_cast<BitRangeImpl*>(*val))
          {
            br = br;
            INTERNAL_MESSAGE("  " << br->getName() << "(" << br->high << ", " << br->low << ")\n");
          }
          else
          {
            INTERNAL_MESSAGE("  " << (*val)->getName() << "\n");
          }
        }
        INTERNAL_MESSAGE("In " << getName() << "(" << PI->first.high << ", " << PI->first.low << ")getOwner():\n");
        for(std::vector<Value*>::iterator val = PI->second->getOwner()->vals.begin(); val != PI->second->getOwner()->vals.end(); ++val)
        {
          if(BitRangeImpl* br = dynamic_cast<BitRangeImpl*>(*val))
          {
            br = br;
            INTERNAL_MESSAGE("  " << br->getName() << "(" << br->high << ", " << br->low << ")\n");
          }
          else
          {
            INTERNAL_MESSAGE("  " << (*val)->getName() << "\n");
          }
        }
      }
      assert( !PI->second->isOwned() or Value::getOwner()->owns(PI->second) );
    }
  }
}

std::map<BitRangeImpl::Triple, BitRangeImpl*> BitRangeImpl::pool;
  
Port::Port(ComponentDeclaration* p, std::string n, int s, Port::TYPE t, Variable::T* v, bool ext) : Variable(s, v), parent(p), name(n), type(t), disp_ext(ext)
{
  if( !isValidVHDLIdentifierName(name) )
  {
    assert(0 and "Invalid VHDL identifier in port declaration.");
  }
  assert( p );
}
Port::TYPE Port::getType()
{
  return type;
}
std::string Port::getName() //from Value
{
  if( disp_ext )
    return getInternalName() + ((type == INPUT)?"_in":"_out");
  return getInternalName();
}
std::string Port::getInternalName() //from Value
{
  return name;
}
std::string Port::generateDeclarationCode() //from Port
{
  Variable::declare();
  std::stringstream ss;
  ss << generateDefaultCode() << " : ";
  ss << ((getType() == INPUT) ? "in" : "out");
  assert( getSize() > 0 and "Port cannot have size of 0!" );
  if(getSize() == 1)
    ss << " STD_LOGIC";
  else
    ss << " STD_LOGIC_VECTOR(" << getSize()-1 << " downto 0)";
  return ss.str();
}
ComponentDeclaration* Port::getParent()
{
  return parent;
}
void Port::setReadFrom() //from Value
{
  if( type != INPUT )
    INTERNAL_ERROR("Cannot read from " << getName() << "!\n");
  assert( type == INPUT );
}
void Port::setWrittenTo() //from Value
{
  assert( type == OUTPUT );
}

Generic::Generic(ComponentDeclaration* p, std::string n, Value* v) : parent(p), default_value(v), name(n)
{
  if( !isValidVHDLIdentifierName(name) )
  {
    assert(0 and "Invalid VHDL identifier in generic declaration.");
  }
  assert(p);
}
std::string Generic::generateDeclarationCode() //from Generic
{
  std::stringstream ss;
  ss << generateDefaultCode() << " : " << generateType();
  if( default_value )
    ss << " := " << default_value->generateDefaultCode();
  return ss.str();
}
ComponentDeclaration* Generic::getParent()
{
  return parent;
}
std::string Generic::getInternalName() //from Value
{
  return name;
}
Value* Generic::getDefaultValue()
{
  return default_value;
}
void Generic::setReadFrom() //from Value
{
  //nothing, for now
}
void Generic::setWrittenTo() //from Value
{
  assert(0 and "Cannot write to a Generic!");
}
std::string IntegerGeneric::generateType() //from Generic
{
  std::stringstream ss;
  ss << "integer";
  return ss.str();
}
IntegerGeneric::IntegerGeneric(ComponentDeclaration* p, std::string n, int v) : Generic(p, n, IntegerTypedValue::get(v)), value(v)
{
}
int IntegerGeneric::getSize() //from Value
{
  return 0;//itoa(value,2).length();
}
std::string IntegerGeneric::generateCode(int size) //from Value
{
  //assert( size > 0 or getSize() == size );
  std::stringstream ss;
  ss << "conv_std_logic_vector(" << getName() << ", " << size << ")";
  return ss.str();
}
std::string IntegerGeneric::generateDeclarationCode() //from Generic
{
  std::stringstream ss;
  ss << getName() << " : " << generateType() << " := " << value;
  return ss.str();
}
class IntegerTypedValueImpl : public Value {
  int val;
public:
  IntegerTypedValueImpl(int v);
  virtual  int getSize(); //from Value
  virtual std::string generateCode(int size); //from Value
  virtual void setReadFrom(); //from Value
  virtual void setWrittenTo(); //from Value
};
IntegerTypedValueImpl::IntegerTypedValueImpl(int v) : val(v)
{
}
Wrap IntegerTypedValue::get(int v)
{
  return Wrap(new IntegerTypedValueImpl(v));
}
int IntegerTypedValueImpl::getSize() //from Value
{
  return 0;//itoa(val,2).length();
}
std::string IntegerTypedValueImpl::generateCode(int size) //from Value
{
  std::stringstream ss;
  ss << val;
  return ss.str();
}
void IntegerTypedValueImpl::setReadFrom() //from Value
{
}
void IntegerTypedValueImpl::setWrittenTo() //from Value
{
  assert(0 and "Cant write to a integer typed value!");
}
Value* IntegerGeneric::getTypeMatchedInteger(int v)
{
  return new IntegerTypedValueImpl(v);
}
  
Port* ComponentDeclaration::addPort(Port* p)
{
  assert( p and "addPort passed NULL pointer!" );
  for(std::vector<Generic*>::iterator GI = generics.begin(); GI != generics.end(); ++GI)
  {
    if( p->getInternalName() == (*GI)->getInternalName() )
    {
      INTERNAL_ERROR("Adding port " << p->getName() << ", but duplicate generic exists!\n");
      assert(0 and "Cannot add a port with duplicate name of already added generic!");
    }
  }
  for(std::vector<Port*>::iterator PI = ports.begin(); PI != ports.end(); ++PI)
  {
    if( (*PI)->getInternalName() == p->getInternalName() )
    {
      if( (*PI)->getType() != p->getType() )
      {
        //output a warning: using the same name, but different directions
        INTERNAL_WARNING("Adding " << p->getName() << ", but port name already exists with different direction.\n");
      }
      else if( (*PI)->getLLVMValue() == p->getLLVMValue() and (*PI)->getSize() == p->getSize() )
      {
        //output a warning: adding the same port multiple times
        INTERNAL_WARNING("Adding " << p->getName() << ", but duplicate port exists.\n");
        //we found a matching port, so return it
        return (*PI);
      }
      else
      {
        INTERNAL_ERROR("Error mapping " << p->getName() << " to a duplicate port!\n");
        if( (*PI)->getLLVMValue() != p->getLLVMValue() )
        {
          INTERNAL_ERROR("Mismatched LLVMValue: \n");
          if( (*PI)->getLLVMValue() )
          {
#ifdef LLVM_VALUE_H
            INTERNAL_MESSAGE("  Existing = " << (*PI)->getLLVMValue()->getName() << "\n");
#endif
          }
          if( p->getLLVMValue() )
          {
#ifdef LLVM_VALUE_H
            INTERNAL_MESSAGE("  Added = " << p->getLLVMValue()->getName() << "\n");
#endif
          }
        }
        if( (*PI)->getSize() != p->getSize() )
        {
          INTERNAL_ERROR("Mismatched sizes: \n" << (*PI)->getSize() << "\n" << p->getSize() << "\n");
        }
        assert( 0 and "Duplicate port added with different value or size!" );
      }
    }
  }
  ports.push_back(p);
  return p;
}
void ComponentDeclaration::addStandardPorts()
{
  addPort( new Port(this, "clk", 1, Port::INPUT, NULL, false) );
  addPort( new Port(this, "rst", 1, Port::INPUT, NULL, false) );
  addPort( new Port(this, "inputReady", 1, Port::INPUT, NULL, false) );
  addPort( new Port(this, "outputReady", 1, Port::OUTPUT, NULL, false) );
  addPort( new Port(this, "done", 1, Port::OUTPUT, NULL, false) );
  addPort( new Port(this, "stall", 1, Port::INPUT, NULL, false) );
}
std::string ComponentDeclaration::generateCode()
{
  std::stringstream ss;
  ss << name << " is\n";
  std::vector<Generic*>::iterator GI = generics.begin();
  if ( GI != generics.end() )
  {
    ss << "generic (\n";
    ss << "\t" << (*GI)->generateDeclarationCode();
    ++GI;
  }
  while( GI != generics.end() )
  {
    ss << ";\n";
    ss << "\t" << (*GI)->generateDeclarationCode();
    ++GI;
  }
  if( GI != generics.begin() )
  {
    ss << "\n\t);\n";
  }
  std::vector<Port*>::iterator PI = ports.begin();
  if ( PI != ports.end() )
  {
    ss << "port (\n";
    ss << "\t" << (*PI)->generateDeclarationCode();
    ++PI;
  }
  while( PI != ports.end() )
  {
    ss << ";";
#if ROCCC_DEBUG >= SOURCE_WARNINGS
    if( (*(PI-1))->getLLVMValue() )
    {
      ss << " --" << (*(PI-1))->getLLVMValue()->getName();
    }
#endif
    ss << "\n";
    ss << "\t" << (*PI)->generateDeclarationCode();
    ++PI;
  }
  if( PI != ports.begin() )
  {
#if ROCCC_DEBUG >= SOURCE_WARNINGS
    if( (*(PI-1))->getLLVMValue() )
    {
      ss << " --" << (*(PI-1))->getLLVMValue()->getName();
    }
#endif
    ss << "\n\t);";
  }
  ss << "\nend ";
  return ss.str();
}
ComponentDeclaration::StandardPorts ComponentDeclaration::getStandardPorts()
{
  std::vector<Port*> c = getPort("clk");
  assert( c.size() == 1 );
  std::vector<Port*> r = getPort("rst");
  assert( r.size() == 1 );
  std::vector<Port*> i = getPort("inputReady");
  assert( i.size() == 1 );
  std::vector<Port*> o = getPort("outputReady");
  assert( o.size() == 1 );
  std::vector<Port*> d = getPort("done");
  assert( d.size() == 1 );
  std::vector<Port*> s = getPort("stall");
  assert( s.size() == 1 );
  return StandardPorts(c[0],r[0],i[0],o[0],d[0],s[0]);
}
ComponentDeclaration::~ComponentDeclaration()
{
}
ComponentDeclaration::ComponentDeclaration(std::string n) : name(n), is_declared(false)
{
  if( !isValidVHDLIdentifierName(name) )
  {
    assert(0 and "Invalid VHDL identifier in component declaration.");
  }
  addStandardPorts();
}
//ComponentDeclaration( llvm::LibraryEntry* le, llvm::CallInst* CI=NULL, bool top=false );
std::vector<Port*>::iterator ComponentDeclaration::firstNonStandardPort()
{
  std::vector<Port*>::iterator ret = ports.begin();
  assert( ports.size() >= 6 );
  ret += 6;
  return ret;
}
Port* ComponentDeclaration::addPortCopy(Variable* v, Port::TYPE t)
{
  assert( v );
  if( dynamic_cast<Port*>(v) )
    return addPort( new Port(this, v->getName(), v->getSize(), t, v->getLLVMValue(), false) );
  return addPort( new Port(this, v->getName(), v->getSize(), t, v->getLLVMValue()) );
}
Port* ComponentDeclaration::addPort(std::string n, int s, Port::TYPE t, Variable::T* v, bool show_extension)
{
  return addPort( new Port(this, n, s, t, v, show_extension) );
}
Generic* ComponentDeclaration::addGeneric(Generic* g)
{
  assert( g and "addGeneric() passed NULL pointer!" );
  for(std::vector<Port*>::iterator PI = ports.begin(); PI != ports.end(); ++PI)
  {
    if( g->getInternalName() == (*PI)->getInternalName() )
    {
      INTERNAL_ERROR("Adding generic " << g->getName() << ", but duplicate port exists!\n");
      assert(0 and "Cannot add a generic with duplicate name of already added port!");
    }
  }
  for(std::vector<Generic*>::iterator GI = generics.begin(); GI != generics.end(); ++GI)
  {
    if( g->getInternalName() == (*GI)->getInternalName() )
    {
      INTERNAL_WARNING("Adding " << g->getName() << ", but duplicate generic exists.\n");
    }
  }
  generics.push_back(g);
  return g;
}
std::string ComponentDeclaration::getName()
{
  return name;
}
std::vector<Port*>& ComponentDeclaration::getPorts()
{
  return ports;
}
std::vector<Generic*>& ComponentDeclaration::getGenerics()
{
  return generics;
}
std::string ComponentDeclaration::generateEntityDeclaration()
{
  std::stringstream ss;
  ss << "entity " << generateCode() << "entity;\n";
  return ss.str();
}
std::string ComponentDeclaration::generateComponentDeclaration()
{
  std::stringstream ss;
  ss << "component " << generateCode() << "component;\n";
  is_declared = true;
  return ss.str();
}
std::vector<Port*> ComponentDeclaration::getPort(Variable::T* v)
{
  std::vector<Port*> ret;
  for(std::vector<Port*>::iterator PI = ports.begin(); PI != ports.end(); ++PI)
  {
    if( (*PI)->getLLVMValue() == v )
      ret.push_back(*PI);
  }
  return ret;
}
std::vector<Port*> ComponentDeclaration::getPort(std::string n)
{
  std::vector<Port*> ret;
  for(std::vector<Port*>::iterator PI = ports.begin(); PI != ports.end(); ++PI)
  {
    if( (*PI)->getInternalName() == n )
      ret.push_back(*PI);
  }
  return ret;
}
Generic* ComponentDeclaration::getGeneric(std::string n)
{
  for(std::vector<Generic*>::iterator GI = generics.begin(); GI != generics.end(); ++GI)
  {
    if( (*GI)->getInternalName() == n )
      return *GI;
  }
  return NULL;
}
/*bool ComponentDeclaration::isDeclared()
{
  return is_declared;
}*/

Signal::Signal(std::string n, int s, Variable::T* v) : Variable(s, v), name(n), read_from(false), written_to(false)
{
  if( !isValidVHDLIdentifierName(name) )
  {
    assert(0 and "Invalid VHDL identifier in signal declaration.");
  }
}
bool Signal::isReadFrom()
{
  return read_from;
}
bool Signal::isWrittenTo()
{
  return written_to;
}
std::string Signal::getInternalName() //from Value
{
  return name;
}
std::string Signal::generateDeclarationCode() //from Signal
{
  if( !isReadFrom() )
  {
    if( !isWrittenTo() )
    {
      return "";
    }
    else
    {
      INTERNAL_WARNING("Generating declaration for value " << getName() << ", which is never read!\n");
    }
  }
  Variable::declare();
  std::stringstream ss;
  ss << "signal " << generateDefaultCode();
  if(getSize() == 1)
    ss << " : STD_LOGIC ;";
  else
    ss << " : STD_LOGIC_VECTOR(" << getSize()-1 << " downto 0) ;";
#if ROCCC_DEBUG >= SOURCE_WARNINGS
  if( getLLVMValue() != NULL )
  {
    ss << " --" << getLLVMValue()->getName();
  }
#endif
  ss << "\n"; 
  return ss.str();
}
void Signal::setReadFrom() //from Value
{
  read_from = true;
}
void Signal::setWrittenTo() //from Value
{
  written_to = true;
}

NamedConstant::NamedConstant(std::string n, int v, Variable::T* t) : Signal(n, 1, t), is_unsigned( v >= 0 )
{
  value = ConstantInt::get(v);
  resize( value->getSize() );
}
NamedConstant::NamedConstant(std::string n, float v, Variable::T* t) : Signal(n, 1, t), is_unsigned( v >= 0 )
{
  value = ConstantFloat::get(v);
  resize( value->getSize() );
}
bool NamedConstant::isUnsigned()
{
  return is_unsigned;
}
class ConstantOwner : public ValueOwner {
public:
  virtual std::string getName(); //from ValueOwner
};
std::string ConstantOwner::getName()
{
  return "CONSTANT_OWNER";
}
std::string NamedConstant::generateDeclarationCode() //from Signal
{
  if( !isReadFrom() )
  {
    return "";
  }
  Variable::declare();  
  Signal::setWrittenTo();
  static ConstantOwner CONSTANT_OWNER;
  CONSTANT_OWNER.add(this);
  std::stringstream ss;
  ss << "constant " << generateDefaultCode();
  if(getSize() == 1)
    ss << " : STD_LOGIC";
  else
    ss << " : STD_LOGIC_VECTOR(" << getSize()-1 << " downto 0)";
  ss << " := " << value->generateCode(getSize()) << " ;";
#if ROCCC_DEBUG >= SOURCE_WARNINGS
  if( getLLVMValue() != NULL )
  {
    ss << " --" << *getLLVMValue();
  }
#endif
  ss << "\n"; 
  return ss.str();
}
void NamedConstant::setWrittenTo() //from Value
{
  assert(0 and "Cannot write to a Constant!");
}
Value* NamedConstant::generateResetValue() //from Variable
{
  assert(0 and "Constants do not have reset value!");
}
void NamedConstant::resize(int s)
{
  if( s > getSize() )
    setSize( s );
}

class ConstantIntImpl : public Value {
  int val;
public:
  ConstantIntImpl(int v);
  virtual  int getSize(); //from Value
  virtual std::string generateCode(int size); //from Value
  virtual void setReadFrom(); //from Value
  virtual void setWrittenTo(); //from Value
  virtual std::string getInternalName(); //from Value
};
ConstantIntImpl::ConstantIntImpl(int v) : val(v)
{
}
Wrap ConstantInt::get(int v)
{
  return Wrap(new ConstantIntImpl(v));
}
 int ConstantIntImpl::getSize() //from Value
{
  return itoa(val,2).length();
}
std::string ConstantIntImpl::generateCode(int size) //from Value
{
  std::string convertedVal = itoa(val, 2);
  while( unsigned(size) > convertedVal.length() )
  {
    if( val < 0 )
      convertedVal = "1" + convertedVal;
    else
      convertedVal = "0" + convertedVal;
  }
  if( convertedVal.length() > unsigned(size) )
  {
    INTERNAL_WARNING("Size of constant overflows storage!\n");
    convertedVal = convertedVal.substr(0, size);
  }
  assert( convertedVal.length() == unsigned(size) and "Error converting a constant! Is the target size too small?" );
  if(size == 1)
    return "\'" + convertedVal + "\'";
  else
    return "\"" + convertedVal + "\"";
}
void ConstantIntImpl::setReadFrom() //from Value
{
}
void ConstantIntImpl::setWrittenTo() //from Value
{
  assert(0 and "Cant write to a constant!");
}
std::string ConstantIntImpl::getInternalName()
{
  std::stringstream ss;
  ss << val;
  return ss.str();
}

class ConstantFloatImpl : public Value {
  float val;
public:
  ConstantFloatImpl(float v);
  virtual  int getSize(); //from Value
  virtual std::string generateCode(int size); //from Value
  virtual void setReadFrom(); //from Value
  virtual void setWrittenTo(); //from Value
};
ConstantFloatImpl::ConstantFloatImpl(float v) : val(v)
{
}
Wrap ConstantFloat::get(float v)
{
  return Wrap(new ConstantFloatImpl(v));
}
 int ConstantFloatImpl::getSize() //from Value
{
  return 32;
}
int* cast_float_pointer_to_int_pointer(float* f)
{
  return reinterpret_cast<int*>(f);
}
long long* cast_double_pointer_to_long_long_pointer(double* f)
{
  return reinterpret_cast<long long*>(f);
}
std::string ConstantFloatImpl::generateCode(int size) //from Value
{
  if( size != 32 and size != 64 )
  {
    INTERNAL_ERROR("Floating point value " << val << " attempted to be converted to size " << size << ", but only 32 bit or 64 bit floats are supported!\n");
  }
  assert( (size == 32 or size == 64) and "Only 32 or 64 bit floats supported!" );
  std::stringstream ss;
  ss << "x\"" ;
  if( size == 32 )
  {
    ss << std::hex << std::setw(8) << std::setfill('0') << *cast_float_pointer_to_int_pointer(&val) << std::dec << std::setfill(' ') ;
  }
  else if ( size == 64 )
  {
    double double_val = val;
    ss << std::hex << std::setw(16) << std::setfill('0') << *cast_double_pointer_to_long_long_pointer(&double_val) << std::dec << std::setfill(' ') ;
  }
  //ss << std::hex << *((int*)(&val)) << std::dec ;
  ss << "\"" ;
  return ss.str();
}
void ConstantFloatImpl::setReadFrom() //from Value
{
}
void ConstantFloatImpl::setWrittenTo() //from Value
{
  assert(0 and "Cant write to a constant!");
}

State::State(std::string n) : name(n)
{
  if( !isValidVHDLIdentifierName(name) )
  {
    assert(0 and "Invalid VHDL identifier in state declaration.");
  }
}
 int State::getSize() //from Value
{
  return 0;
}
std::string State::generateCode(int size) //from Value
{
  assert( size == 0 );
  return getName();
}
void State::setReadFrom() //from Value
{
}
void State::setWrittenTo() //from Value
{
  assert( 0 and "Cannot write to a state type!" );
}
std::string State::getInternalName() //from Value
{
  return name;
}

StateVar::StateVar(std::string n, int, Variable::T*) : Signal(n, 1)
{
}
int StateVar::getSize() //from Value
{
  return 0;
}
std::string StateVar::generateCode(int size) //from Value
{
  assert( size == 0 );
  return getName();
}
std::string StateVar::generateDeclarationCode() //from Signal
{
  Variable::declare();
  std::stringstream ss;
  ss << "type " << getName() << "_STATE_TYPE is (";
  for( std::vector<State*>::iterator SI = states.begin(); SI != states.end(); ++SI )
  {
    if( SI != states.begin() )
      ss << ", ";
    ss << (*SI)->getName();
  }
  ss << ") ;\n";
  ss << "signal " << generateDefaultCode() << " : " << getName() << "_STATE_TYPE ;\n";
  return ss.str();
}
Value* StateVar::generateResetValue()
{
  assert(states.size() != 0);
  return states.at(0);
}
State* StateVar::insertState(std::string n, std::vector<State*>::iterator i)
{
  for(std::vector<State*>::iterator SI = states.begin(); SI != states.end(); ++SI)
  {
    assert( (*SI)->getName() != n );
  }
  State* ret = new State(n);
  states.insert( i, ret );
  return ret;
}
State* StateVar::insertState(State* s, std::vector<State*>::iterator i)
{
  for(std::vector<State*>::iterator SI = states.begin(); SI != states.end(); ++SI)
  {
    assert( (*SI)->getName() != s->getName() );
  }
  states.insert( i, s );
  return s;
}
State* StateVar::addState(std::string n)
{
  return insertState(n, states.end() );
}
State* StateVar::addState(State* s)
{
  return insertState(s, states.end() );
}
std::vector<State*>& StateVar::getStates()
{
  return states;
}

class OperatedValue : public Value, public HasReplaceAllUsesOfWith {
public:
  enum TYPE {ADD,SUB,MUL,DIV,BW_AND,BW_OR,BW_XOR};
private:
  Value* lhs;
  TYPE op;
  Value* rhs;
protected:
  virtual HasReplaceAllUsesOfWith* cloneImpl() //from HasReplaceAllUsesOfWith
  {
    return new OperatedValue(getClonedCopyIfPossible(lhs), op, getClonedCopyIfPossible(rhs));
  }
public:
  OperatedValue(Value* l, TYPE t, Value* r);
  virtual  int getSize(); //from Value
  virtual std::string generateCode(int size); //from Value
  virtual void setReadFrom(); //from Value
  virtual void setWrittenTo(); //from Value
  virtual void replaceAllUsesOfWith(Value* of, Value* with) //from HasReplaceAllUsesOfWith
  {
    lhs = ReplaceAllUsesOfWithIn(of, with, lhs);
    rhs = ReplaceAllUsesOfWithIn(of, with, rhs);
  }
};
OperatedValue::OperatedValue(Value* l, TYPE t, Value* r) : lhs(l), op(t), rhs(r)
{
  assert(lhs);
  assert(rhs);
}
int OperatedValue::getSize() //from Value
{
  return (lhs->getSize() > rhs->getSize())?lhs->getSize():rhs->getSize();
}
std::string OperatedValue::generateCode(int size) //from Value
{
  std::stringstream ss;
  int max_size = (size > getSize()) ? size : getSize();
  switch( op )
  {
    case ADD:
      { 
	ss << "ROCCCADD(" << lhs->generateCode(max_size) + ", " 
	   << rhs->generateCode(max_size) << ", " << size << ")"; 
      }
      break ;
    case SUB:
      {
	ss << "ROCCCSUB(" << lhs->generateCode(max_size) + ", " 
	   << rhs->generateCode(max_size) << ", " << size << ")"; 
      }
      break ;
    case MUL: 
      {
	ss << "ROCCCMUL(" << lhs->generateCode(max_size) + ", " 
	   << rhs->generateCode(max_size) << ", " << size << ")"; 
      }
      break ;
    case DIV: 
      {
	ss << "ROCCCDIV(" << lhs->generateCode(max_size) + ", " 
	   << rhs->generateCode(max_size) << ", " << size << ")";
	assert(0 and "Division was not converted to a core call!");
      }
      break;
    case BW_AND:
      {
	ss << "(" << lhs->generateCode(size) + " and " 
	   << rhs->generateCode(size) << ")"; 
      }
      break ;
    case BW_OR:
      {
	ss << "(" << lhs->generateCode(size) + " or " 
	   << rhs->generateCode(size) << ")"; 
      }
      break ;
    case BW_XOR: 
      {
	ss << "(" << lhs->generateCode(size) + " xor " 
	   << rhs->generateCode(size) << ")"; 
      }
      break ;
    default: 
      assert(0 and "Unknown operator!");
  }
  return ss.str();
}
void OperatedValue::setReadFrom() //from Value
{
  lhs->setReadFrom();
  rhs->setReadFrom();
}
void OperatedValue::setWrittenTo() //from Value
{
  assert(0 and "Cant write to a binary expression!");
}
Wrap operator + (Wrap lhs, Wrap rhs)
{
  return Wrap(new OperatedValue(lhs, OperatedValue::ADD, rhs));
}
Wrap operator - (Wrap lhs, Wrap rhs)
{
  return Wrap(new OperatedValue(lhs, OperatedValue::SUB, rhs));
}
Wrap operator * (Wrap lhs, Wrap rhs)
{
  return Wrap(new OperatedValue(lhs, OperatedValue::MUL, rhs));
}
Wrap operator / (Wrap lhs, Wrap rhs)
{
  return Wrap(new OperatedValue(lhs, OperatedValue::DIV, rhs));
}
Wrap operator & (Wrap lhs, Wrap rhs) //bitwise and
{
  return Wrap(new OperatedValue(lhs, OperatedValue::BW_AND, rhs));
}
Wrap operator | (Wrap lhs, Wrap rhs) //bitwise or
{
  return Wrap(new OperatedValue(lhs, OperatedValue::BW_OR, rhs));
}
Wrap operator ^ (Wrap lhs, Wrap rhs) //bitwise xor
{
  return Wrap(new OperatedValue(lhs, OperatedValue::BW_XOR, rhs));
}
Wrap shift_left_logical(Variable* lhs, int rhs)
{
  assert(rhs > 0);
  assert(lhs);
  assert( lhs->getSize() > rhs and "Shifting further than the bitsize will allow!");
  Wrap ret = BitRange::get(lhs, lhs->getSize()-1-rhs, 0);
  for(int i = 0; i < rhs; ++i)
    ret = bitwise_concat(ret, ConstantInt::get(0));
  return ret;
}
Wrap shift_right_logical(Variable* lhs, int rhs)
{
  assert(rhs > 0);
  assert(lhs);
  assert( lhs->getSize() > rhs and "Shifting further than the bitsize will allow!");
  Wrap ret = BitRange::get(lhs, lhs->getSize()-1, rhs);
  for(int i = 0; i < rhs; ++i)
    ret = bitwise_concat(ConstantInt::get(0), ret);
  return ret;
}
Wrap shift_right_arithmetic(Variable* lhs, int rhs)
{
  assert(rhs > 0);
  assert(lhs);
  assert( lhs->getSize() > rhs and "Shifting further than the bitsize will allow!");
  Wrap ret = BitRange::get(lhs, lhs->getSize()-1, rhs);
  for(int i = 0; i < rhs; ++i)
    ret = bitwise_concat(BitRange::get(lhs,lhs->getSize()-1,lhs->getSize()-1), ret);
  return ret;
}
class BitWiseConcatImpl : public Value, public HasReplaceAllUsesOfWith {
private:
  Value* lhs;
  Value* rhs;
protected:
  virtual HasReplaceAllUsesOfWith* cloneImpl() //from HasReplaceAllUsesOfWith
  {
    //TODO: implement
    return this;
  }
public:
  BitWiseConcatImpl(Value* l, Value* r);
  virtual int getSize(); //from Value
  virtual std::string generateCode(int size); //from Value
  virtual void setReadFrom(); //from Value
  virtual void setWrittenTo(); //from Value
  virtual void replaceAllUsesOfWith(Value* of, Value* with) //from HasReplaceAllUsesOfWith
  {
    //TODO: implement, be size-aware
  }
};
BitWiseConcatImpl::BitWiseConcatImpl(Value* l, Value* r) : lhs(l), rhs(r)
{
  assert(lhs);
  assert(rhs);
}
int BitWiseConcatImpl::getSize() //from Value
{
  return lhs->getSize() + rhs->getSize();
}
std::string BitWiseConcatImpl::generateCode(int size) //from Value
{
  if( size <= rhs->getSize() )
    return rhs->generateCode(size);
  int lhs_size = size - rhs->getSize();
  return lhs->generateCode(lhs_size) + " & " + rhs->generateDefaultCode();
}
void BitWiseConcatImpl::setReadFrom() //from Value
{
  rhs->setReadFrom();
  lhs->setReadFrom();
}
void BitWiseConcatImpl::setWrittenTo() //from Value
{
  assert(0 and "Cannot write to a concatenation!");
}
Wrap bitwise_concat(Value* lhs, Value* rhs)
{
  return Wrap(new BitWiseConcatImpl(lhs,rhs));
}

class ConditionImpl : public Condition {
  virtual HasReplaceAllUsesOfWith* cloneImpl(); //from HasReplaceAllUsesOfWith
public:
  enum TYPE {EQ=0x1,
             LT=0x2,
             GT=0x4,
             NEQ=~EQ,
             LTE=~GT,
             GTE=~LT};
  Value* lhs;
  TYPE op;
  Value* rhs;
  bool is_signed;
public:
  ConditionImpl(Value* l, TYPE t, Value* r, bool isSigned=false);
  virtual std::string generateCode(); //from Condition
  virtual void setReadFrom(); //from Condition
  virtual void replaceAllUsesOfWith(Value* of, Value* with); //from HasReplaceAllUsesOfWith
};
class MultiCondition : public Condition {
  virtual HasReplaceAllUsesOfWith* cloneImpl(); //from HasReplaceAllUsesOfWith
public:
  enum TYPE {AND, OR};
private:
  Condition* lhs;
  TYPE op;
  Condition* rhs;
public:
  MultiCondition(Condition* l, TYPE t, Condition* r);
  virtual std::string generateCode(); //from Condition
  virtual void setReadFrom(); //from Condition
  virtual void replaceAllUsesOfWith(Value* of, Value* with); //from HasReplaceAllUsesOfWith
};
Condition::Condition()
{
}
ConditionImpl::ConditionImpl(Value* l, TYPE t, Value* r, bool isSigned) : lhs(l), op(t), rhs(r)
{
  assert( lhs );
  assert( rhs );
  is_signed = isSigned or ((dynamic_cast<Variable*>(lhs)) ? !dynamic_cast<Variable*>(lhs)->isUnsigned() : (dynamic_cast<Variable*>(rhs)) ? !dynamic_cast<Variable*>(rhs)->isUnsigned() : false);
}
std::string ConditionImpl::generateCode() //from Condition
{
  std::stringstream ss;
  int size = (lhs->getSize() > rhs->getSize())?lhs->getSize():rhs->getSize();
  switch( op )
  {
    case EQ: ss << "(" << lhs->generateCode(size) << " = " << rhs->generateCode(size) << ")"; break;
    case LT: ss << "ROCCC_" << ((is_signed)?"S":"U") << "LT(" << lhs->generateCode(size) << ", " << rhs->generateCode(size) << ", " << size << ")"; break;
    case GT: ss << "ROCCC_" << ((is_signed)?"S":"U") << "GT(" << lhs->generateCode(size) << ", " << rhs->generateCode(size) << ", " << size << ")"; break;
    case NEQ: ss << "(" << lhs->generateCode(size) << " /= " << rhs->generateCode(size) << ")"; break;
    case LTE: ss << "ROCCC_" << ((is_signed)?"S":"U") << "LTE(" << lhs->generateCode(size) << ", " << rhs->generateCode(size) << ", " << size << ")"; break;
    case GTE: ss << "ROCCC_" << ((is_signed)?"S":"U") << "GTE(" << lhs->generateCode(size) << ", " << rhs->generateCode(size) << ", " << size << ")"; break;
    default: assert(0 and "Unknown operator!");
  }
  return ss.str();
}
void ConditionImpl::setReadFrom() //from Condition
{
  lhs->setReadFrom();
  rhs->setReadFrom();
}
HasReplaceAllUsesOfWith* ConditionImpl::cloneImpl() //from HasReplaceAllUsesOfWith
{
  return new ConditionImpl(getClonedCopyIfPossible(lhs), op, getClonedCopyIfPossible(rhs), is_signed);
}
void ConditionImpl::replaceAllUsesOfWith(Value* of, Value* with)
{
  lhs = ReplaceAllUsesOfWithIn(of, with, lhs);
  rhs = ReplaceAllUsesOfWithIn(of, with, rhs);
}
class SpecialTypeNONE : public Condition {
  virtual std::string generateCode() //from Condition
  {
    return "";
  }
  virtual void setReadFrom() //from Condition
  {
    assert(0 and "Cannot read from a null Condition!");
  }
  virtual HasReplaceAllUsesOfWith* cloneImpl() //from HasReplaceAllUsesOfWith
  {
    return this;
  }
  virtual void replaceAllUsesOfWith(Value* of, Value* with) //from HasReplaceAllUsesOfWith
  {
  }
};
Condition* Condition::NONE = new SpecialTypeNONE;

class ConstConditionImpl : public Condition {
  bool val;
  virtual HasReplaceAllUsesOfWith* cloneImpl() //from HasReplaceAllUsesOfWith
  {
    return this;
  }
public:
  ConstConditionImpl( bool v ) : val(v){}
  virtual std::string generateCode() //from Condition
  {
    return (val)?"true":"false";
  }
  virtual void setReadFrom() //from Condition
  {
  }
  virtual void replaceAllUsesOfWith(Value* of, Value* with) //from HasReplaceAllUsesOfWith
  {
  }
};
CWrap ConstCondition::get(bool v)
{
  return CWrap( new ConstConditionImpl(v) );
}

class EventConditionImpl : public Condition {
  Variable* val;
  virtual HasReplaceAllUsesOfWith* cloneImpl() //from HasReplaceAllUsesOfWith
  {
    return new EventConditionImpl(getClonedCopyIfPossible(val));
  }
public:
  EventConditionImpl( Variable* v ) : val(v)
  {
    assert(val);
  }
  virtual std::string generateCode() //from Condition
  {
    std::stringstream ss;
    ss << val << "'event";
    return ss.str();
  }
  virtual void setReadFrom() //from Condition
  {
    val->setReadFrom();
  }
  virtual void replaceAllUsesOfWith(Value* of, Value* with) //from HasReplaceAllUsesOfWith
  {
    val = ReplaceAllUsesOfWithIn(of, with, val);
  }
};
CWrap EventCondition::rising_edge(Variable* v)
{
  assert(v);
  assert( v->getSize() == 1 and "Cannot have rising_edge on a bit_vector!" );
  return event(v) and (Wrap(v) == ConstantInt::get(1));
}
CWrap EventCondition::falling_edge(Variable* v)
{
  assert(v);
  assert( v->getSize() == 1 and "Cannot have falling_edge on a bit_vector!" );
  return event(v) and (Wrap(v) == ConstantInt::get(0));
}
CWrap EventCondition::event(Variable* v)
{
  return CWrap(new EventConditionImpl(v));
}

CWrap operator == (Wrap lhs, Wrap rhs)
{
  return CWrap(new ConditionImpl(lhs.val, ConditionImpl::EQ, rhs.val));
}
CWrap operator < (Wrap lhs, Wrap rhs)
{
  return CWrap(new ConditionImpl(lhs.val, ConditionImpl::LT, rhs.val));
}
CWrap operator > (Wrap lhs, Wrap rhs)
{
  return CWrap(new ConditionImpl(lhs.val, ConditionImpl::GT, rhs.val));
}
CWrap operator != (Wrap lhs, Wrap rhs)
{
  return CWrap(new ConditionImpl(lhs.val, ConditionImpl::NEQ, rhs.val));
}
CWrap operator >= (Wrap lhs, Wrap rhs)
{
  return CWrap(new ConditionImpl(lhs.val, ConditionImpl::GTE, rhs.val));
}
CWrap operator <= (Wrap lhs, Wrap rhs)
{
  return CWrap(new ConditionImpl(lhs.val, ConditionImpl::LTE, rhs.val));
}
CWrap SLT(Wrap lhs, Wrap rhs)
{
  return CWrap(new ConditionImpl(lhs.val, ConditionImpl::LT, rhs.val, true));
}
CWrap SGT(Wrap lhs, Wrap rhs)
{
  return CWrap(new ConditionImpl(lhs.val, ConditionImpl::GT, rhs.val, true));
}
CWrap SLTE(Wrap lhs, Wrap rhs)
{
  return CWrap(new ConditionImpl(lhs.val, ConditionImpl::LTE, rhs.val, true));
}
CWrap SGTE(Wrap lhs, Wrap rhs)
{
  return CWrap(new ConditionImpl(lhs.val, ConditionImpl::GTE, rhs.val, true));
}

MultiCondition::MultiCondition(Condition* l, TYPE t, Condition* r) : lhs(l), op(t), rhs(r)
{
  assert( lhs );
  assert( rhs );
}
std::string MultiCondition::generateCode() //from Condition
{
  std::stringstream ss;
  ss << "(" << lhs->generateCode();
  switch( op )
  {
    case AND: ss << " and "; break;
    case OR : ss << " or "; break;
    default : assert(0 and "Unknown operator!");
  }
  ss << rhs->generateCode() << ")";
  return ss.str();
}
void MultiCondition::setReadFrom() //from Condition
{
  lhs->setReadFrom();
  rhs->setReadFrom();
}
HasReplaceAllUsesOfWith* MultiCondition::cloneImpl() //from HasReplaceAllUsesOfWith
{
  Condition* n_lhs = lhs->clone<Condition>();
  Condition* n_rhs = rhs->clone<Condition>();
  return new MultiCondition(n_lhs, op, n_rhs);
}
void MultiCondition::replaceAllUsesOfWith(Value* of, Value* with) //from HasReplaceAllUsesOfWith
{
  lhs->replaceAllUsesOfWith(of, with);
  rhs->replaceAllUsesOfWith(of, with);
}

CWrap operator and (CWrap lhs, CWrap rhs)
{
  return CWrap(new MultiCondition(lhs.val, MultiCondition::AND, rhs.val));
}
CWrap operator or (CWrap lhs, CWrap rhs)
{
  return CWrap(new MultiCondition(lhs.val, MultiCondition::OR, rhs.val));
}
class NotCondition : public Condition {
  Condition* cond;
  virtual HasReplaceAllUsesOfWith* cloneImpl() //from HasReplaceAllUsesOfWith
  {
    Condition* n_cond = cond->clone<Condition>();
    return new NotCondition(n_cond);
  }
public:
  NotCondition( Condition* c ) : cond(c)
  {
    assert(cond);
  }
  virtual std::string generateCode() //from Condition
  {
    std::stringstream ss;
    ss << "not(" << cond->generateCode() << ")";
    return ss.str();
  }
  virtual void setReadFrom() //from Condition
  {
    cond->setReadFrom();
  }
  virtual void replaceAllUsesOfWith(Value* of, Value* with) //from HasReplaceAllUsesOfWith
  {
    cond->replaceAllUsesOfWith(of, with);
  }
};
CWrap operator not(CWrap rhs)
{
  return CWrap(new NotCondition(rhs.val));
}

ValueOwner::ValueOwner()
{
}
void ValueOwner::add(Value* v) //from ValueOwner, DO NOT CALL EXCEPT FROM DERIVED CLASSES
{
  assert( v );
  if( this->owns(v) )
    return;
  vals.push_back( v );
  v->setOwner(this);
  //some values can never be owned - if we just tried to add one of those,
  //  then we should back up and remove it so we dont think we actually
  //  own it.
  if( v->getOwner() != this )
    vals.erase(vals.end()-1);
}
bool ValueOwner::owns(Value* v)
{
  return (find(vals.begin(), vals.end(), v) != vals.end() );
}
bool ValueOwner::canTransferOwnershipOfTo(Value* v, ValueOwner* vo)
{
  return false;
}

ProcessVariable::ProcessVariable(std::string n, int s, Variable::T* v) : Variable(s, v), name(n), read_from(false), written_to(false)
{
  if( !isValidVHDLIdentifierName(name) )
  {
    assert(0 and "Invalid VHDL identifier in variable declaration.");
  }
}
std::string ProcessVariable::generateAssignmentOperator() //from Value
{
  return ":=";
}
bool ProcessVariable::isReadFrom()
{
  return read_from;
}
bool ProcessVariable::isWrittenTo()
{
  return written_to;
}
std::string ProcessVariable::getInternalName() //from Value
{
  return name;
}
std::string ProcessVariable::generateDeclarationCode() //from Signal
{
  if( !isReadFrom() )
  {
    if( !isWrittenTo() )
    {
      return "";
    }
    else
    {
      INTERNAL_WARNING("Generating declaration for value " << getName() << ", which is never read!\n");
    }
  }
  Variable::declare();
  std::stringstream ss;
  ss << "variable " << generateDefaultCode();
  if(getSize() == 1)
    ss << " : STD_LOGIC ;";
  else
    ss << " : STD_LOGIC_VECTOR(" << getSize()-1 << " downto 0) ;";
#if ROCCC_DEBUG >= SOURCE_WARNINGS
  if( getLLVMValue() != NULL )
  {
    ss << " --" << getLLVMValue()->getName();
  }
#endif
  ss << "\n"; 
  return ss.str();
}
void ProcessVariable::setReadFrom() //from Value
{
  read_from = true;
}
void ProcessVariable::setWrittenTo() //from Value
{
  written_to = true;
}

HasReplaceAllUsesOfWith* Process::cloneImpl() //from HasReplaceAllUsesOfWith
{
  assert(0 and "Subclasses of Process must implement cloneImpl() before calling clone()!");
}
std::vector<VHDLInterface::Variable*> Process::getSensitivityList() //from Process
{
  std::vector<Variable*> ret;
  ret.push_back(getClockDomain());
  if( hasReset )
    ret.push_back(parent->getDeclaration()->getStandardPorts().rst);
  return ret;
}
std::string Process::generateResetCode(int level) //from Process
{
  std::stringstream ss;
  MultiStatement ms(this);
  for(std::vector<Value*>::iterator VI = vals.begin(); VI != vals.end(); ++VI)
  {
    Value* v = *VI;
    assert( v );
    AssignmentStatement* r = new AssignmentStatement(v, this);
    if( Variable* var = dynamic_cast<Variable*>(v) )
    {
      r->addCase(var->generateResetValue());
    }
    else
    {
      r->addCase(ConstantInt::get(0));
    }
    ms.addStatement(r);
  }
  ss << ms.generateCode(level);
  return ss.str();
}
Entity* Process::getParent()
{
  return parent;
}
Process::Process(Entity* p, Variable* clk) : ValueOwner(), parent(p), _clk(clk), hasReset(true)
{
  assert(parent);
}
void Process::setClockDomain(Variable* clk)
{
  _clk = clk;
}
Variable* Process::getClockDomain()
{
  return _clk;
}
void Process::setHasReset(bool hr)
{
  hasReset = hr;
}
std::string Process::getName() //from ValueOwner
{
  return "Process";
}

std::string Process::generateCode()
{
  std::stringstream ss;
  assert( getClockDomain() and "Clock domain has not been set!" );
  ss << "process(";
  std::vector<Variable*> sensitivityList = getSensitivityList();
  for(std::vector<Variable*>::iterator SLI = sensitivityList.begin(); SLI != sensitivityList.end(); ++SLI)
  {
    if( SLI != sensitivityList.begin() )
      ss << ", ";
    assert(*SLI);
    ss << (*SLI)->generateDefaultCode();
  }
  ss << ")\n";
  for(std::vector<Value*>::iterator VI = vals.begin(); VI != vals.end(); ++VI)
  {
    ProcessVariable* v = dynamic_cast<ProcessVariable*>(*VI);
    if( v )
    {
      ss << v->generateDeclarationCode();
    }
  }
  ss << "begin\n";
  if( hasReset )
  {
    ss << "  if (" << parent->getDeclaration()->getStandardPorts().rst << " = \'1\') then\n";
    ss << generateResetCode(2);
    ss << "  els";
  }
  else
    ss << "  ";
  ss << "if( " << getClockDomain()->generateDefaultCode() << "\'event and " << getClockDomain()->generateDefaultCode() << " = \'1\' ) then\n";
  ss << generateSteadyState(2);
  ss << "  end if;\n";
  ss << "end process;\n";
  return ss.str();
}
void Process::replaceAllUsesOfWith(Value* of, Value* with) //from HasReplaceAllUsesOfWith
{
  assert(0 and "Subclasses of Process must implement replaceAllUsesOfWith() before replacing values!");
}

Statement::Statement(Process* p) : parent(p)
{
}
Process* Statement::getParent()
{
  return parent;
}

IfStatement::IfStatement(Process* p, Condition* c, Statement* t, Statement* f) : Statement(p), cond(c), true_statement(t), false_statement(f)
{
  assert( p and "Cannot have an IfStatement outside of a process!" );
  assert( c );
  c->setReadFrom();
  assert( t );
}
std::string IfStatement::generateCode(int level) //from Statement
{
  std::stringstream ss;
  assert( level >= 0 );
  for(int i = 0; i < level; ++i)
    ss << "  ";
  ss << "if( " << cond->generateCode() << " ) then\n";
  ss << true_statement->generateCode(level+1);
  if( false_statement )
  {
    for(int i = 0; i < level; ++i)
      ss << "  ";
    ss << "else\n";
    ss << false_statement->generateCode(level+1);
  }
  for(int i = 0; i < level; ++i)
    ss << "  ";
  ss << "end if;\n";
  return ss.str();
}
HasReplaceAllUsesOfWith* IfStatement::cloneImpl() //from HasReplaceAllUsesOfWith
{
  Condition* n_cond = cond->clone<Condition>();
  Statement* n_true = true_statement->clone<Statement>();
  Statement* n_false = NULL;
  if( false_statement )
    n_false = false_statement->clone<Statement>();
  return new IfStatement(this->getParent(), n_cond, n_true, n_false);
}
void IfStatement::replaceAllUsesOfWith(Value* of, Value* with) //from HasReplaceAllUsesOfWith
{
  cond->replaceAllUsesOfWith(of, with);
  true_statement->replaceAllUsesOfWith(of, with);
  if( false_statement )
    false_statement->replaceAllUsesOfWith(of, with);
}

IfElsifStatement::IfElsifStatement(Process* p) : Statement(p)
{
  assert(p != NULL and "If statement must appear in a process!") ;  
}

IfElsifStatement::IfElsifStatement(Process* p, 
				   std::list<Condition*> c,
				   std::list<Statement*> s) : Statement(p),
							      conditions(c),
							      statements(s)
{
  assert(p != NULL and "If statement must appear in a process!") ;
}

std::string IfElsifStatement::generateCode(int level) //from Statement
{
  std::stringstream ss;
  bool first = true ;
  assert( level >= 0 );
  std::list<Condition*>::iterator condIter = conditions.begin() ;
  assert(condIter != conditions.end() and "If statement has no conditions") ;
  std::list<Statement*>::iterator stateIter = statements.begin() ;
  assert(stateIter != statements.end() and "If statement has no statements!") ;

  while (condIter != conditions.end())
  {
    assert(stateIter != statements.end() and 
	   "Unmatched conditions and statements in if statement!") ;
    if (first == true)
    {
      for(int i = 0; i < level; ++i)
	ss << "  ";
      ss << "if (" ;
      first = false ;
    }
    else
    {
      for(int i = 0; i < level; ++i)
	ss << "  ";
      ss << "elsif (" ;
    }
    assert(*condIter != NULL and "Empty condition in if statement!") ;
    ss << (*condIter)->generateCode() << " ) then\n" ;
    ss << (*stateIter)->generateCode(level+1) ;
    ++condIter ;
    ++stateIter ;
  }
  for(int i = 0; i < level; ++i)
    ss << "  ";
  ss << "end if;\n";
  return ss.str();
}

void IfElsifStatement::addCase(Condition* c, Statement* s)
{
  assert(c != NULL) ;
  assert(s != NULL) ;
  conditions.push_back(c) ;
  statements.push_back(s) ;
}

HasReplaceAllUsesOfWith* IfElsifStatement::cloneImpl() 
{
  std::list<Condition*> cloneConditions ;
  std::list<Condition*>::iterator condIter = conditions.begin() ;
  while (condIter != conditions.end())
  {
    cloneConditions.push_back((*condIter)->clone<Condition>()) ;
    ++condIter ;
  }
  std::list<Statement*> cloneStatements ;
  std::list<Statement*>::iterator stateIter = statements.begin() ;
  while (stateIter != statements.end())
  {
    cloneStatements.push_back((*stateIter)->clone<Statement>()) ;
    ++stateIter ;
  }
  return new IfElsifStatement(this->getParent(), cloneConditions,
			      cloneStatements) ;
}
void IfElsifStatement::replaceAllUsesOfWith(Value* of, Value* with) 
{
  std::list<Condition*>::iterator condIter = conditions.begin() ;
  while (condIter != conditions.end())
  {
    (*condIter)->replaceAllUsesOfWith(of, with) ;
    ++condIter ;
  }
  std::list<Statement*>::iterator stateIter = statements.begin() ;
  while (stateIter != statements.end())
  {
    (*stateIter)->replaceAllUsesOfWith(of, with) ;
    ++stateIter ;
  }
}

MultiStatement::MultiStatement(Process* p) : Statement(p)
{
  assert( p and "Cannot have a MultiStatement outside of a process!" );
}
MultiStatement* MultiStatement::addStatement(Statement* s)
{
  assert( s );
  assert( getParent() == s->getParent() );
  statements.push_back( s );
  return this;
}
bool MultiStatement::isEmpty()
{
  for(std::vector<Statement*>::iterator SI = statements.begin(); SI != statements.end(); ++SI)
  {
    if( MultiStatement* cms = dynamic_cast<MultiStatement*>(*SI) )
    {
      if( !cms->isEmpty() )
        return false;
    }
    else
      return false;
  }
  return true;
}
std::string MultiStatement::generateCode(int level) //from Statement
{
  std::stringstream ss;
  for(std::vector<Statement*>::iterator SI = statements.begin(); SI != statements.end(); ++SI)
  {
    ss << (*SI)->generateCode(level);
  }
  return ss.str();
}
HasReplaceAllUsesOfWith* MultiStatement::cloneImpl() //from HasReplaceAllUsesOfWith
{
  MultiStatement* n_ms = new MultiStatement(getParent());
  for(std::vector<Statement*>::iterator SI = statements.begin(); SI != statements.end(); ++SI)
  {
    Statement* n_s = (*SI)->clone<Statement>();
    n_ms->addStatement(n_s);
  }
  return n_ms;
}
void MultiStatement::replaceAllUsesOfWith(Value* of, Value* with) //from HasReplaceAllUsesOfWith
{
  for(std::vector<Statement*>::iterator SI = statements.begin(); SI != statements.end(); ++SI)
  {
    (*SI)->replaceAllUsesOfWith(of, with);
  }
}

AssignmentStatement::AssignmentStatement(Value* l, Process* p) : Statement(p), ValueOwner(), lhs(l)
{
  assert( lhs );
  lhs->setWrittenTo();
  add(lhs);
}
AssignmentStatement::AssignmentStatement(Value* l, Value* r, Process* p) : Statement(p), ValueOwner(), lhs(l)
{
  assert( lhs );
  lhs->setWrittenTo();
  add(lhs);
  assert(r and "Cannot have NULL right value in assignment statement!");
  if( r )
  {
    addCase(r);
  }
}
void AssignmentStatement::add(Value* v) //from ValueOwner, DO NOT CALL
{
  assert( v );
  if( getParent() )
    getParent()->add( v );
  else
    ValueOwner::add( v );
}
AssignmentStatement* AssignmentStatement::addCase(Value* v, Condition* c)
{
  assert( v );
  assert( c );
  v->setReadFrom();
  if( c != Condition::NONE )
    c->setReadFrom();
  rhs.push_back(std::pair<Value*,Condition*>(v,c));
  return this;
}
std::string AssignmentStatement::getName() //from ValueOwner
{
  return "AssignmentStatement";
}

void AssignmentStatement::removeLastCase()
{
  assert(rhs.size() != 0 and "Cannot remove last case!") ;
  rhs.pop_back() ;
}

std::string AssignmentStatement::generateCode(int level)
{
  std::stringstream ss;
  assert( lhs );
  if( rhs.size() == 0 )
  {
    INTERNAL_WARNING(lhs->getName() << " has no assigned value!\n");
  }
  assert( rhs.size() != 0 and "Cannot generate an assignment statement with no rhs value!");
  if( getParent() == NULL )
  {
    assert( (rhs.end()-1)->second == Condition::NONE and "Synchronous statement must have default case!" );
    ss << lhs->generateDefaultCode() << " " << lhs->generateAssignmentOperator() << " ";
    for(std::vector<std::pair<Value*,Condition*> >::iterator RHS = rhs.begin(); RHS != rhs.end(); ++RHS)
    {
      if( RHS != rhs.begin() )
        ss << "              ";
      assert( RHS->first );
      ss << RHS->first->generateCode(lhs->getSize());
      if( RHS->second->generateCode() != "")
        ss << " when " << RHS->second->generateCode();
      else
        assert( RHS+1 == rhs.end() and "Default case is not last case!");
      if( RHS+1 != rhs.end() )
        ss << " else\n";
    }
    ss << ";\n";
  }
  else
  {
    assert( level >= 0 );
    for(std::vector<std::pair<Value*,Condition*> >::iterator RHS = rhs.begin(); RHS != rhs.end(); ++RHS)
    {
      assert( RHS->first );
      if( RHS->second->generateCode() != "")
      {
        for(int i = 0; i < level; ++i)
          ss << "  ";
        if( RHS == rhs.begin() )
          ss << "if( ";
        else
          ss << "elsif( ";
        ss << RHS->second->generateCode() << " ) then\n  ";
      }
      else
      {
        assert( RHS+1 == rhs.end() and "Default case is not last case!");
        if( RHS != rhs.begin() )
        {
          for(int i = 0; i < level; ++i)
            ss << "  ";
          ss << "else\n  ";
        }
      }
      for(int i = 0; i < level; ++i)
        ss << "  ";
      ss << lhs->generateDefaultCode() << " " << lhs->generateAssignmentOperator() << " " << RHS->first->generateCode(lhs->getSize()) << ";\n";
      if( RHS+1 == rhs.end() and (RHS != rhs.begin() or  RHS->second->generateCode() != ""))
      {
        for(int i = 0; i < level; ++i)
          ss << "  ";
        ss << "end if;\n";
      }
    }
  }
  return ss.str();
}
Value* AssignmentStatement::getSingleValue()
{
  if( rhs.size() == 1 )
  {
    return rhs.begin()->first;
  }
  return NULL;
}
HasReplaceAllUsesOfWith* AssignmentStatement::cloneImpl() //from HasReplaceAllUsesOfWith
{
  AssignmentStatement* n_as = new AssignmentStatement(lhs, getParent());
  for(std::vector<std::pair<Value*,Condition*> >::iterator RHS = rhs.begin(); RHS != rhs.end(); ++RHS)
  {
    Condition* n_cond = RHS->second->clone<Condition>();
    n_as->addCase(getClonedCopyIfPossible(RHS->first), n_cond);
  }
  return n_as;
}
void AssignmentStatement::replaceAllUsesOfWith(Value* of, Value* with) //from HasReplaceAllUsesOfWith
{
  lhs = ReplaceAllUsesOfWithIn(of, with, lhs);
  for(std::vector<std::pair<Value*,Condition*> >::iterator RHS = rhs.begin(); RHS != rhs.end(); ++RHS)
  {
    RHS->first = ReplaceAllUsesOfWithIn(of, with, RHS->first);
    RHS->second->replaceAllUsesOfWith(of, with);
  }
}

CaseStatement::CaseStatement(Process* p, StateVar* sv) : Statement(p), state_var(sv)
{
}
CaseStatement* CaseStatement::addCase(State* state, Statement* statement)
{
  assert(state);
  assert(statement);
  for(std::vector<std::pair<State*,Statement*> >::iterator CSI = cases.begin(); CSI != cases.end(); ++CSI)
  {
    if( CSI->first == state )
    {
      assert( 0 and "Cannot add state multiple times to a case statement!" );
    }
  }
  cases.push_back(std::pair<State*, Statement*>(state, statement));
  return this;
}
CaseStatement* CaseStatement::addStatement(State* state, Statement* statement)
{
  assert(state);
  for(std::vector<std::pair<State*,Statement*> >::iterator CSI = cases.begin(); CSI != cases.end(); ++CSI)
  {
    if( CSI->first == state )
    {
      MultiStatement* ms = dynamic_cast<MultiStatement*>(CSI->second);
      assert( ms and "Cannot add a statement to a statement other than a multistatement!" );
      ms->addStatement(statement);
      return this;
    }
  }
  MultiStatement* ms = new MultiStatement(getParent());
  ms->addStatement(statement);
  return addCase(state, ms);
}
Statement* CaseStatement::getCase(State* state)
{
  assert(state);
  for(std::vector< std::pair<State*, Statement*> >::iterator CI = cases.begin(); CI != cases.end(); ++CI)
  {
    if( CI->first == state )
      return CI->second;
  }
  return NULL;
}
std::string CaseStatement::generateCode(int level)
{
  std::stringstream ss;
  for(int i = 0; i < level; ++i)
    ss << "  ";
  ss << "case " << state_var->generateDefaultCode() << " is\n";
  for(std::vector< std::pair<State*,Statement*> >::iterator CI = cases.begin(); CI != cases.end(); ++CI)
  {
    for(int i = 0; i < level+1; ++i)
      ss << "  ";
    ss << "when " << CI->first->generateDefaultCode() << " =>\n";
    ss << CI->second->generateCode(level+2);
  }
  for(int i = 0; i < level+1; ++i)
    ss << "  ";
  ss << "when others =>\n";
  for(int i = 0; i < level+2; ++i)
    ss << "  ";
  ss << "--error if we get here\n";
  for(int i = 0; i < level+2; ++i)
    ss << "  ";
  ss << state_var->generateDefaultCode() << " " << state_var->generateAssignmentOperator() << " " << state_var->generateResetValue()->generateDefaultCode() << ";\n";
  for(int i = 0; i < level; ++i)
    ss << "  ";
  ss << "end case;\n";
  return ss.str();
}
HasReplaceAllUsesOfWith* CaseStatement::cloneImpl() //from HasReplaceAllUsesOfWith
{
  assert(0 and "Havent implemented cloneImpl on case statements\n");
}
void CaseStatement::replaceAllUsesOfWith(Value* of, Value* with) //from HasReplaceAllUsesOfWith
{
  assert(0 and "Havent implemented replaceAllUsesOfWith() on case statements\n");
}

HasReplaceAllUsesOfWith* MultiStatementProcess::cloneImpl() //from HasReplaceAllUsesOfWith
{
  MultiStatementProcess* ret = new MultiStatementProcess(getParent(), getClockDomain());
  ret->addStatement(getClonedCopyIfPossible(ms));
  return ret;
}
MultiStatement* MultiStatementProcess::getSteadyState()
{
  return ms;
}
std::string MultiStatementProcess::generateSteadyState(int level) //from Process
{
  assert(ms);
  return ms->generateCode(level);
}
MultiStatementProcess::MultiStatementProcess(Entity* p, Variable* clk) : Process(p,clk), ms(new MultiStatement(this))
{
}
MultiStatementProcess* MultiStatementProcess::addStatement(Statement* s)
{
  assert(ms);
  ms->addStatement(s);
  return this;
}
void MultiStatementProcess::replaceAllUsesOfWith(Value* of, Value* with) //from HasReplaceAllUsesOfWith
{
  ReplaceAllUsesOfWithIn(of, with, getSteadyState());
}

HasReplaceAllUsesOfWith* CommentStatement::cloneImpl() //from HasReplaceAllUsesOfWith
{
  return this;
}
CommentStatement::CommentStatement(Process* p, std::string c) : Statement(p), comment(c)
{
}
std::string CommentStatement::generateCode(int level) //from Statement
{
  std::stringstream ss;
#if ROCCC_DEBUG >= SOURCE_WARNINGS
  for(int i = 0; i < level; ++i)
    ss << "  ";
  ss << "--" << comment << "\n";
#endif
  return ss.str();
}
void CommentStatement::replaceAllUsesOfWith(Value* of, Value* with) //from HasReplaceAllUsesOfWith
{
}

ComponentDefinition::ComponentDefinition(std::string n, ComponentDeclaration* d) : ValueOwner(), name(n), declaration(d)
{
  if( !isValidVHDLIdentifierName(name) )
  {
    assert(0 and "Invalid VHDL identifier in component definition.");
  }
  assert(declaration);
}
void ComponentDefinition::map(Variable* s, Port* p)
{
  assert(s);
  assert(p);
  assert( p->getParent() == declaration && "Port not recognized!" );
  if( s->getSize() != p->getSize() )
  {
    INTERNAL_WARNING("Mapping elements of different sizes in component " << getName() << " of type " << declaration->getName() << "!\n" <<
                      "  " << s->getName() << " is size " << s->getSize() << "\n"
                      "  " << p->getName() << " is size " << p->getSize() << "\n");
    assert( p->getSize() < s->getSize() and "Cannot map a subcomponent to a value of smaller size!" );
  }
  if ( p->getType() == Port::OUTPUT )
  {
    //llvm::cout << s->getName() << " is being written to by " << getName() << "::" << p->getName() << ".\n";
    s->setWrittenTo();
    //p->setReadFrom(); //removed because outputs expect to be written to
    ValueOwner::add( s );
  }
  else if ( p->getType() == Port::INPUT )
  {
    //llvm::cout << s->getName() << " is being read by " << getName() << "::" << p->getName() << ".\n";
    s->setReadFrom();
    //p->setWrittenTo(); //removed because inputs expect to be read from
  }
  else
    assert( "Port is not input or output!" );
  if( s->getLLVMValue() != p->getLLVMValue() )
  {
#ifdef LLVM_VALUE_H
    INTERNAL_WARNING("Mapping signal " << s->getName()
    << "(llvm::value " << (s->getLLVMValue()?s->getLLVMValue()->getName():"NULL") << ")"
    << " to port " << p->getParent()->getName() << "->" << p->getName()
    << "(llvm::value " << (p->getLLVMValue()?p->getLLVMValue()->getName():"NULL") << ")\n"
    );
#endif
  }
  std::map<Port*, Variable*>::iterator f = mappings.find(p);
  if( f != mappings.end() and 
      (
        f->second->getName() != s->getName() or f->second->getSize() != s->getSize() or
        f->second->getLLVMValue() != s->getLLVMValue()
      )
    )
  {
    INTERNAL_ERROR("Remapping port \'" << p->getName() << "\' from value \'"
                << f->second->getName() << "\' to value \'" << s->getName() << "\' in component " << getName() << "!\n");
    assert( 0 and "Remapping an already mapped port to a different value!" );
  }
  if( !owns(s) )
  {
    //this really isnt even a warning . . .
    //INTERNAL_WARNING("Component " << getName() << " doesnt own signal " << s->getName() << "!\n");
  }
  mappings[p] = s;
}
void ComponentDefinition::map(Variable* s, std::string name)
{
  std::vector<Port*> p = declaration->getPort(name);
  assert( p.size() > 0 and "Could not find port with that name!" );
  assert( p.size() == 1 and "Multiple ports with same name!" );
  this->map(s, p[0]);
}
void ComponentDefinition::mapGeneric(Value* s, Generic* g)
{
  assert(s);
  assert(g);
  assert( g->getParent() == declaration && "Generic not recognized!" );
  if( s->getSize() != g->getSize() )
  {
    INTERNAL_WARNING("Mapping elements of different sizes in component " << getName() << " of type " << declaration->getName() << "!\n" <<
                      "  " << s->getName() << " is size " << s->getSize() << "\n"
                      "  " << g->getName() << " is size " << g->getSize() << "\n");
    assert( g->getSize() < s->getSize() and "Cannot map a subcomponent to a value of smaller size!" );
  }
  std::map<Generic*, Value*>::iterator f = genericMap.find(g);
  if( f != genericMap.end() and (f->second->getName() != s->getName() or f->second->getSize() != s->getSize()) )
  {
    INTERNAL_ERROR("Remapping generic \'" << g->getName() << "\' from value \'"
                << f->second->getName() << "\' to value \'" << s->getName() << "\' in component " << getName() << "!\n");
    assert( 0 and "Remapping an already mapped generic to a different value!" );
  }
  genericMap[g] = s;
}
void ComponentDefinition::mapGeneric(Value* s, std::string name)
{
  this->mapGeneric(s, declaration->getGeneric(name));
}
std::string ComponentDefinition::getName()
{
  return name;
}
Signal* ComponentDefinition::map(ComponentDefinition* cde, Port* external, Port* internal, std::string n)
{
  assert( cde );
  assert( external );
  assert( internal );
  assert( cde != this );
  assert( external->getParent() == cde->getDeclaration() && "Port and external component do not match!" );
  assert( internal->getParent() == declaration && "Port is not internal!" );
  assert( external->getSize() == internal->getSize() && "Mapping elements of different sizes!" );
  assert( !(external->getType() == Port::INPUT && internal->getType() == Port::INPUT) && "Cannot connect input to input!" );
  assert( !(external->getType() == Port::OUTPUT && internal->getType() == Port::OUTPUT) && "Cannot connect output to output!" );
  if( external->getLLVMValue() != internal->getLLVMValue() )
  {
#ifdef LLVM_VALUE_H
    INTERNAL_WARNING("Mapping port " << external->getParent()->getName() << "->" << external->getName()
          << "(llvm::value " << (external->getLLVMValue()?external->getLLVMValue()->getName():"NULL") << ")"
          << " to port " << internal->getParent()->getName() << "->" << internal->getName()
          << "(llvm::value " << (internal->getLLVMValue()?internal->getLLVMValue()->getName():"NULL") << ")"
          );
#endif
  }
  if( n == "" )
    n = "connection_" + cde->getName() + "_" + external->getName() + "__" + this->getName() + "_" + internal->getName();
  Signal* connection = new Signal( n, external->getSize(), external->getLLVMValue() );
  this->map( connection, internal );
  cde->map( connection, external );
  return connection;
}
Signal* ComponentDefinition::map( ComponentDefinition* cde, std::string external, std::string internal, std::string n )
{
  assert( cde );
  std::vector<Port*> external_ports = cde->getDeclaration()->getPort(external);
  std::vector<Port*> internal_ports = this->getDeclaration()->getPort(internal);
  assert( (external_ports.size() == 1 or internal_ports.size() == 1) and "Cannot resolve multiple port names!" );
  Port* external_port = NULL;
  Port* internal_port = NULL;
  if( external_ports.size() == 1 )
  {
    external_port = external_ports[0];
    for(std::vector<Port*>::iterator PI = internal_ports.begin(); PI != internal_ports.end(); ++PI)
    {
      if( (*PI)->getType() != external_port->getType() )
      {
        assert( !internal_port and "Multiple possible matching ports!" );
        internal_port = *PI;
      }
    }
  }
  else if( internal_ports.size() == 1 )
  {
    internal_port = internal_ports[0];
    for(std::vector<Port*>::iterator PI = external_ports.begin(); PI != external_ports.end(); ++PI)
    {
      if( (*PI)->getType() != internal_port->getType() )
      {
        assert( !external_port and "Multiple possible matching ports!" );
        external_port = *PI;
      }
    }
  }
  assert( external_port );
  assert( internal_port );
  return this->map(cde, external_port, internal_port, n );
}
bool ComponentDefinition::isMapped(VHDLInterface::Port* p)
{
  assert(p);
  assert(p->getParent() == this->getDeclaration());
  return( mappings.find(p) != mappings.end() );
}
std::map<Port*, Variable*>& ComponentDefinition::getMap()
{
  return mappings;
}
std::map<Generic*, Value*>& ComponentDefinition::getGenericMap()
{
  return genericMap;
}
ComponentDeclaration* ComponentDefinition::getDeclaration()
{
  return declaration;
}
std::string ComponentDefinition::generateCode()
{
  std::stringstream ret;
  //assert( declaration->isDeclared() );
  ret << name << ": " << declaration->getName();
  if( genericMap.begin() != genericMap.end() )
    ret << " generic map (";
  bool on_first_generic_mapping = true;
  for(std::vector<Generic*>::iterator GI = declaration->getGenerics().begin(); GI != declaration->getGenerics().end(); ++GI)
  {
    if( genericMap.find(*GI) != genericMap.end() )
    {
      if( !on_first_generic_mapping )
        ret << ", ";
      on_first_generic_mapping = false;
      ret << (*GI)->getName() << " => " << genericMap[*GI]->generateCode((*GI)->getSize());
    }
    else
    {
      if( !(*GI)->getDefaultValue() )
      {
        INTERNAL_ERROR(getName() << "::" << (*GI)->getName() << " is left unmapped, with no default!\n");
        assert(0 and "Cannot leave generic unmapped with no default!");
      }
    }
  }
  if( genericMap.begin() != genericMap.end() )
    ret << ")";
  ret << " port map (";
  for(std::vector<Port*>::iterator PI = declaration->getPorts().begin(); PI != declaration->getPorts().end(); ++PI)
  {
    if( PI != declaration->getPorts().begin() )
      ret << ", ";
    ret << (*PI)->generateDefaultCode() << " => ";
    if( mappings.find(*PI) == mappings.end() )
    {
      INTERNAL_WARNING(getName() << "::" << (*PI)->getName() << " is left open!\n");
      assert( (*PI)->getType() != Port::INPUT and "Cannot leave input ports open!" );
      ret << "open";
    }
    else
      ret << mappings[*PI]->generateCode((*PI)->getSize());
  }
  ret << ") ;\n";
  return ret.str();    
}
void ComponentDefinition::setParent(Entity* e) //from ComponentDefinition
{
}

Entity::Entity(std::string name) : declaration(new ComponentDeclaration(name))
{
  if( !isValidVHDLIdentifierName(name) )
  {
    assert(0 and "Invalid VHDL identifier in entity declaration.");
  }
}
Port* Entity::addPortCopy(Variable* v, Port::TYPE t)
{
  return getDeclaration()->addPortCopy(v, t);
}
Port* Entity::addPort(std::string name, int size, Port::TYPE direction, Variable::T* v)
{
  assert( name != "" );
  return declaration->addPort(name, size, direction, v);      
}
void Entity::addSignal(Signal* s)
{
  assert(s);
  if( findSignal(s->getName()).size() != 0 )
  {
    INTERNAL_ERROR(s->getName() << " already exists, as a signal!\n");
  }
  assert( findSignal(s->getName()).size() == 0 and "Duplicate signal added!" );
  if( getPort(s->getName()).size() != 0 )
  {
    INTERNAL_ERROR(s->getName() << " already exists, as a port!\n");
  }
  assert( getPort(s->getName()).size() == 0 and "Signal added with same name as port!" );
  signals.push_back(s);
  assert( s->getName() != "" );
  signalNameMap[s->getName()].push_back(s);
  signalValueMap[s->getLLVMValue()].push_back(s);
}
AssignmentStatement* Entity::createSynchronousStatement(Value* v, Value* rhs)
{
  assert( v );
  assert( findSignal(v->getInternalName()).size() or getPort(v->getInternalName()).size() );
  AssignmentStatement* ret = new AssignmentStatement( v );
  if( rhs )
    ret->addCase(rhs);
  synchronous_statements.push_back( ret );
  return ret;
}
Attribute* Entity::getAttribute(std::string name)
{
  for(std::vector<Attribute*>::iterator AI = attributes.begin(); AI != attributes.end(); ++AI)
  {
    if( name == (*AI)->getName() )
      return *AI;
  }
  return NULL;
}
Attribute* Entity::addAttribute(Attribute* att)
{
  assert(att);
  assert(!getAttribute(att->getName()) and "Cannot have Attributes with same name!");
  attributes.push_back(att);
  return att;
}
ComponentDefinition* Entity::addComponent(ComponentDefinition* cd)
{
  assert(cd);
  for(std::vector<ComponentDefinition*>::iterator CDI = components.begin(); CDI != components.end(); ++CDI)
  {
    //make sure they arent named the same as a previous component
    assert( (*CDI)->getName() != cd->getName() );
    //if the declarations are named the same thing, but are different declarations, check that the ports match exactly
    if( (*CDI)->getDeclaration()->getName() == cd->getDeclaration()->getName() and (*CDI)->getDeclaration() != cd->getDeclaration() )
    {
      std::vector<Port*>::iterator PI1 = cd->getDeclaration()->getPorts().begin();
      for(std::vector<Port*>::iterator PI2 = (*CDI)->getDeclaration()->getPorts().begin(); PI1 != cd->getDeclaration()->getPorts().end() and PI2 != (*CDI)->getDeclaration()->getPorts().end(); ++PI2, ++PI1)
      {
        assert( (*PI1)->getName() == (*PI2)->getName() );
        assert( (*PI1)->getSize() == (*PI2)->getSize() );
        assert( (*PI1)->getType() == (*PI2)->getType() );
      }
    }
  }
  components.push_back( cd );
  cd->setParent(this);
  return cd;
}
ComponentDefinition* Entity::createComponent(std::string name, ComponentDeclaration* cd)
{
  ComponentDefinition* ret = new ComponentDefinition(name, cd);
  return this->addComponent(ret);
}
StateVar* Entity::createStateVar(std::string name, int num_elements)
{
  StateVar* ret = new StateVar(name);
  ret->addState( "S_START" );
  ret->addState( "S_OUTPUT" );
  for( int i = 0; i < num_elements; ++i )
  {
    insertBufferState(ret);
  }
  addSignal(ret);
  return ret;
}
State* Entity::insertBufferState(StateVar* state)
{
  std::stringstream state_name;
  state_name << "S" << getNumBufferStates(state);
  assert( state );
  assert( state->getStates().size() >= 2 );
  state->insertState( state_name.str(), state->getStates().end()-1 );
  return getBufferState( state, getNumBufferStates(state)-1 );
}
State* Entity::getFirstState(StateVar* state)
{
  assert( state );
  assert( state->getStates().size() != 0 );
  return state->getStates()[0];
}
State* Entity::getLastState(StateVar* state)
{
  assert( state );
  assert( state->getStates().size() != 0 );
  return state->getStates()[state->getStates().size() - 1];
}
int Entity::getNumBufferStates(StateVar* state)
{
  assert(state);
  assert( state->getStates().size() >= 2 );
  return state->getStates().size() - 2;
}
State* Entity::getBufferState(StateVar* state, int i)
{
  assert( 0 <= i and i < getNumBufferStates(state) );
  return state->getStates()[i+1];
}
std::string Entity::generateCode()
{
  std::stringstream ss;
  ss << "-- This file was automatically generated by "
     << ROCCC_VERSION << ", DO NOT EDIT\n"; 
  ss << "library IEEE ;\n" ;
  ss << "use IEEE.STD_LOGIC_1164.all ;\n" ;
  ss << "use IEEE.STD_LOGIC_ARITH.ALL;\n";
  ss << "use IEEE.STD_LOGIC_UNSIGNED.all ;\n" ;
  ss << "use work.HelperFunctions.all;\n";
  ss << "use work.HelperFunctions_Unsigned.all;\n";
  ss << "use work.HelperFunctions_Signed.all;\n";
  ss << "\n";
  assert( declaration );
  ss << declaration->generateEntityDeclaration() << "\n";
  ss << "architecture Synthesized of " << declaration->getName() << " is \n";
  for(std::vector<Port*>::iterator PI = declaration->getPorts().begin(); PI != declaration->getPorts().end(); ++PI)
    ss << (*PI)->generateAttributeCode();
  for(std::vector<Signal*>::iterator SI = signals.begin(); SI != signals.end(); ++SI)
    ss << (*SI)->generateDeclarationCode();
  for(std::vector<Signal*>::iterator SI = signals.begin(); SI != signals.end(); ++SI)
    ss << (*SI)->generateAttributeCode();
  std::map<std::string, bool> defsDeclared;
  for(std::vector<ComponentDefinition*>::iterator CI = components.begin(); CI != components.end(); ++CI)
  {
    if( defsDeclared.find((*CI)->getDeclaration()->getName()) == defsDeclared.end() )
    {
      ss << (*CI)->getDeclaration()->generateComponentDeclaration();
      defsDeclared[(*CI)->getDeclaration()->getName()] = true;
    }
    else
      (*CI)->getDeclaration()->generateComponentDeclaration();
  }
  ss << "begin\n";
  for(std::vector<ComponentDefinition*>::iterator CI = components.begin(); CI != components.end(); ++CI)
    ss << (*CI)->generateCode();
  for(std::vector<AssignmentStatement*>::iterator SI = synchronous_statements.begin(); SI != synchronous_statements.end(); ++SI)
    ss << (*SI)->generateCode(0);
  for(std::vector<Process*>::iterator PI = processes.begin(); PI != processes.end(); ++PI)
    ss << (*PI)->generateCode();
  ss << "end Synthesized;\n";
  return ss.str();
}
ComponentDeclaration* Entity::getDeclaration()
{
  return declaration;
}
Signal* Entity::mapSubComponentPorts(ComponentDefinition* c1, Port* p1, ComponentDefinition* c2, Port* p2)
{
  assert( std::find(components.begin(), components.end(), c1) != components.end() );
  assert( std::find(components.begin(), components.end(), c2) != components.end() );
  assert(c1);
  Signal* ret = c1->map(c2, p2, p1);
  addSignal(ret);
  return ret;
}
void Entity::mapPortToSubComponentPort(Port* p1, ComponentDefinition* c2, Port* p2)
{
  assert(p1);
  assert(c2);
  assert(p2);
  assert( std::find(components.begin(), components.end(), c2) != components.end() );
  assert( p1->getParent() == declaration );
  if( p1->getType() == Port::OUTPUT )
  {
    INTERNAL_WARNING("Strongly suggest using \n\t" << c2->getName() << "->map(this->getVariableMappedTo(" << p1->getName() << "), " << p2->getName() << ") \n\tinstead of \n\tthis->mapPortToSubComponentPort(" << p1->getName() << ", " << c2->getName() << ", " << p2->getName() << ") for output ports!\n");
  }
  c2->map( p1, p2 );
}
std::vector<Signal*> Entity::findSignal(Variable::T* t)
{
  return signalValueMap[t];
}
std::vector<Signal*> Entity::findSignal(std::string n)
{
  return signalNameMap[n];
}
std::vector<Port*> Entity::getPort(Variable::T* v)
{
  return declaration->getPort(v);
}
std::vector<Port*> Entity::getPort(std::string n)
{
  return declaration->getPort(n);
}
ComponentDeclaration::StandardPorts Entity::getStandardPorts()
{
  return declaration->getStandardPorts();
}
Variable* Entity::getVariableMappedTo(ComponentDefinition* cd, Port* p)
{
  if( !cd )
    INTERNAL_ERROR("Component is null!\n");
  assert( cd );
  if( !p )
    INTERNAL_ERROR("Port is null!\n");
  assert( p );
  if( p->getParent() != cd->getDeclaration() )
    INTERNAL_ERROR(p->getParent()->getName() << " != " << cd->getDeclaration()->getName() << "!\n");
  assert( p->getParent() == cd->getDeclaration() && "Port is not part of component!" );
  if( cd->getMap().find(p) == cd->getMap().end() )
  {
    Signal* ns = createSignal<Signal>( cd->getName() + "_" + p->getInternalName(), p->getSize(), p->getLLVMValue() );
    cd->map(ns, p);
  }
  return cd->getMap().find(p)->second;
}
Signal* Entity::getSignalMappedToPort(Port* p)
{
  assert( p );
  assert( p->getParent() == declaration && "Port is not part of entity!" );
  assert( p->getType() == Port::OUTPUT and "Cannot getSignalMappedToPort() on input port!");
  for(std::vector<AssignmentStatement*>::iterator SSI = synchronous_statements.begin(); SSI != synchronous_statements.end(); ++SSI)
  {
    if( (*SSI)->owns(p) )
    {
      assert( (*SSI)->getSingleValue() and "Multiple possible values in assignment statement!" );
      Signal* ret = dynamic_cast<Signal*>((*SSI)->getSingleValue());
      assert( ret and "Value mapped to port is not a signal!" );
      return ret;
    }
  }
  for(std::vector<Process*>::iterator PI = processes.begin(); PI != processes.end(); ++PI)
  {
    assert( !(*PI)->owns(p) and "Process owns port, cannot create signal mapped to port!" );
  }
  AssignmentStatement* assign = createSynchronousStatement(p);
  Signal* sig = createSignal<Signal>(getDeclaration()->getName() + "_" + p->getInternalName(), p->getSize(), p->getLLVMValue());
  assign->addCase(sig);
  return sig;
}
/*
std::vector<AssignmentStatement*> Entity::getSynchronousStatements()
{
  return synchronous_statements;
}
std::vector<Process*> Entity::getProcesses()
{
  return processes;
}
std::vector<Signal*> Entity::getSignals()
{
  return signals;
}
*/

} //end namespace VHDLInterface

