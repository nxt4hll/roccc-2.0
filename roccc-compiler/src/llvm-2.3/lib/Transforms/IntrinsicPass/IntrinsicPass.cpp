/*
  File:
  
  Purpose: 
*/

#include "IntrinsicPass.h"
#include "llvm/Constants.h"
#include "llvm/Module.h"

#include <vector>
#include <assert.h>
#include <sstream>
#include <algorithm>

#include "rocccLibrary/SizeInBits.h"
#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/MessageLogger.h"
#include "rocccLibrary/ROCCCNames.h"
#include "rocccLibrary/DatabaseInterface.h"
#include "rocccLibrary/CopyValue.h"
#include "rocccLibrary/GetValueName.h"

using namespace llvm ;
using namespace Database;

/*
create a function header for an invoke hardware that will call
a module named name, with 3 arguments of type argument_type.
this is used when converting operators to calls to ipcores.
*/
Function* create_RocccInvokeHardware_for(GlobalValue* name, std::vector<const Type*> argument_type, Module *M)
{
  std::vector<const Type*> paramTypes;
  paramTypes.push_back( name->getType() );
  //we are assuming all of the original operands have the same type
  for(std::vector<const Type*>::iterator ATI = argument_type.begin(); ATI != argument_type.end(); ++ATI)
  {
    paramTypes.push_back( *ATI );
  }

  FunctionType* ft = FunctionType::get(Type::VoidTy, paramTypes, false);

  Function* rocccInvokeHw;
  //create a call to an invoke hardware with the specified operands
  rocccInvokeHw = Function::Create(ft,
				   (GlobalValue::LinkageTypes)0,
				   ROCCCNames::InvokeHardware, 
				   M );
  return rocccInvokeHw;
}

/*
given an instruction inst that is an operator with 2 operands,
create a call instruction that calls the ipcore of the specified
operator and pass it the correct operands
*/
Value* convert_to_HW_call(Instruction* inst, Instruction* next, GlobalValue* name, int size)
{   
   std::vector<Value*> valArgs;
   std::vector<const Type*> valTypes;
   valArgs.push_back( name );
   //add all the arguments from the instruction to the new call instruction
   for(unsigned OpNum = 0; OpNum < inst->getNumOperands(); ++OpNum)
   {
     //if its was already a call instruction, we need to ignore the first argument, which is the function name
     if( dynamic_cast<CallInst*>(inst) and OpNum == 0 )
     {
       continue;
     }
     valArgs.push_back( inst->getOperand(OpNum) );
     valTypes.push_back( inst->getOperand(OpNum)->getType() );
   }
   //if we arent a void type, then we need to add ourselves as the last argument
   if( inst->getType()->getTypeID() != llvm::Type::VoidTyID )
   {
     //we need to create a new value that will be used as destination for the operation
     Value* ptr = new AllocaInst(inst->getType(), 0, "", next);
     Value* result = new LoadInst(ptr, inst->getName(), next);
     //create a size for the newly created result
     setSizeInBits(result, size);
     //the new value is the last argument
     valArgs.push_back( result );
     valTypes.push_back( result->getType() );
     
     Function* rocccInHw = create_RocccInvokeHardware_for(name, valTypes, inst->getParent()->getParent()->getParent());
     //create and return the call instruction
     CallInst::Create( rocccInHw,
  				    valArgs.begin(),
  				    valArgs.end(),
  				    "" ,
  				    next);
     return result;
   }
   else
   {
     Function* rocccInHw = create_RocccInvokeHardware_for(name, valTypes, inst->getParent()->getParent()->getParent());
     //create and return the call instruction
     return CallInst::Create( rocccInHw,
      				    valArgs.begin(),
      				    valArgs.end(),
      				    "" ,
      				    next);
   }
}

// The call to the constructor that registers this pass
char ROCCCIntrinsicPass::ID = 0 ;

static RegisterPass<ROCCCIntrinsicPass> X("ROCCCfloat", "Float Pass") ;

ROCCCIntrinsicPass::ROCCCIntrinsicPass() : ModulePass((intptr_t)&ID)
{
}

ROCCCIntrinsicPass::~ROCCCIntrinsicPass() 
{
 // Nothing to clean up yet
}

/*
Create or find a unique global string that is the same as name.
Used to find the names of operators, such as fp_mul, as a global string.
*/
GlobalValue* getOrCreateNamedGlobalString(Module& M, std::string name)
{
  //see if the value already exists
  GlobalValue* ret = M.getNamedGlobal(name);
  //create the global if it doesnt already exist
  if(ret == NULL )
  {
    Constant* val = ConstantArray::get(name);
    ret = new GlobalVariable(val->getType(), true, (GlobalValue::LinkageTypes)0, val, name, &M);
  }
  assert(ret);
  return ret;
}

template< class T >
std::list<LibraryEntry*> filterIntrinsics(std::list<LibraryEntry*> cores, T* filterFunction)
{
  //INTERNAL_MESSAGE("Running filter " << filterFunction->getDescription() << "\n");
  std::list<LibraryEntry*> ret;
  for(std::list<LibraryEntry*>::iterator CI = cores.begin(); CI != cores.end(); ++CI)
  {
    if( (*filterFunction)(*CI) )
    {
      ret.push_back(*CI);
    }
  }
  return ret;
}

template< class T >
std::list<LibraryEntry*> sortIntrinsics(std::list<LibraryEntry*> cores, T* sortFunction)
{
  //INTERNAL_MESSAGE("Running sort " << sortFunction->getDescription() << "\n");
  std::vector<LibraryEntry*> vec;
  vec.resize(cores.size());
  std::copy(cores.begin(), cores.end(), vec.begin());
  assert(vec.size() == cores.size());
  std::sort(vec.begin(), vec.end(), *sortFunction);
  std::list<LibraryEntry*> ret;
  ret.resize(vec.size());
  std::copy(vec.begin(), vec.end(), ret.begin());
  assert(ret.size() == vec.size());
  return ret;
}

class Filter {
public:
  Filter(){}
  virtual bool operator()(LibraryEntry*)=0;
  virtual std::string getDescription()=0;
};

class Sort {
public:
  Sort(){}
  virtual bool operator()(LibraryEntry* c, LibraryEntry* d)=0;
  virtual std::string getDescription()=0;
};

class MultiFilterAnd : public Filter {
  std::list<Filter*> filters;
public:
  MultiFilterAnd(){}
  MultiFilterAnd& add(Filter* f){filters.push_back(f);return *this;}
  virtual bool operator()(LibraryEntry* LE)
  {
    for(std::list<Filter*>::iterator FLI = filters.begin(); FLI != filters.end(); ++FLI)
    {
      assert(*FLI);
      if( !(**FLI)(LE) )
        return false;
    }
    return true;
  }
  virtual std::string getDescription()
  {
    std::stringstream ret;
    ret << "(";
    for(std::list<Filter*>::iterator FLI = filters.begin(); FLI != filters.end(); ++FLI)
    {
      if( FLI != filters.begin() )
        ret << " AND ";
      ret << (*FLI)->getDescription();
    }
    ret << ")";
    return ret.str();
  }
};

class MultiFilterOr : public Filter {
  std::list<Filter*> filters;
public:
  MultiFilterOr(){}
  MultiFilterOr& add(Filter* f){filters.push_back(f);return *this;}
  virtual bool operator()(LibraryEntry* LE)
  {
    for(std::list<Filter*>::iterator FI = filters.begin(); FI != filters.end(); ++FI)
    {
      if( (**FI)(LE) )
        return true;
    }
    return false;
  }
  virtual std::string getDescription()
  {
    std::stringstream ret;
    ret << "(";
    for(std::list<Filter*>::iterator FLI = filters.begin(); FLI != filters.end(); ++FLI)
    {
      if( FLI != filters.begin() )
        ret << " OR ";
      ret << (*FLI)->getDescription();
    }
    ret << ")";
    return ret.str();
  }
};

class MultiSortTiered : public Sort {
protected:
  std::list<Sort*> sorts;
public:
  MultiSortTiered(){}
  virtual MultiSortTiered& add(Sort* s){sorts.push_back(s);return *this;}
  virtual bool operator()(LibraryEntry* c, LibraryEntry* d)
  {
    for(std::list<Sort*>::iterator SI = sorts.begin(); SI != sorts.end(); ++SI)
    {
      if( (**SI)(c, d) ) //c is greater than d
        return true;
      else if( (**SI)(d, c) ) //d is greater than c
        return false;
      //they are equal, so check next sorting criteria
    }
    //they are equal on every single comparison criteria!
    return false;
  }
  virtual std::string getDescription()
  {
    std::stringstream ret;
    ret << "(";
    for(std::list<Sort*>::iterator SI = sorts.begin(); SI != sorts.end(); ++SI)
    {
      if( SI != sorts.begin() )
        ret << ", THEN ";
      ret << (*SI)->getDescription();
    }
    ret << ")";
    return ret.str();
  }
};

class FilterCorePortSizeIsEqual : public Filter {
  int port_num;
  int size;
public:
  FilterCorePortSizeIsEqual(int p, int s):port_num(p),size(s){}
  virtual bool operator()(LibraryEntry* c)
  {
    std::list<Database::Port*> ports = c->getNonStreamPorts();
    std::list<Database::Port*>::const_iterator PI = ports.begin();
    for(int i = 0; i < port_num and PI != ports.end(); ++i)
    {
      ++PI;
    }
    assert( PI != ports.end() );
    return ((*PI)->getBitSize() == size);
  }
  virtual std::string getDescription()
  {
    std::stringstream ret;
    ret << "(portSize(" << port_num << ") == " << size << ")";
    return ret.str();
  }
};

class FilterCoreStreamSizeIsEqual : public Filter {
  int stream_num;
  int size;
public:
  FilterCoreStreamSizeIsEqual(int n, int s) : stream_num(n), size(s) {}
  virtual bool operator()(LibraryEntry* c)
  {
    std::list<Database::Stream> streams = c->getStreams();
    std::list<Database::Stream>::const_iterator SI = streams.begin();
    for(int i = 0; i < stream_num and SI != streams.end(); ++i)
    {
      ++SI;
    }
    assert( SI != streams.end() );
    assert( SI->getDataChannels().size() > 0 );
    return (SI->getDataChannels().front()->getBitSize() == size);
  }
  virtual std::string getDescription()
  {
    std::stringstream ret;
    ret << "(streamSize(" << stream_num << ") == " << size << ")";
    return ret.str();
  }
};

class FilterCorePortSizeIsGreater : public Filter {
  int port_num;
  int size;
public:
  FilterCorePortSizeIsGreater(int p, int s):port_num(p),size(s){}
  bool operator()(LibraryEntry* c)
  {
    std::list<Database::Port*> ports = c->getNonStreamPorts();
    std::list<Database::Port*>::const_iterator PI = ports.begin();
    for(int i = 0; i < port_num and PI != ports.end(); ++i)
    {
      ++PI;
    }
    if( PI == ports.end() )
    {
      INTERNAL_MESSAGE(c->getName() << " " << ports.size() << " " << port_num << "\n");
    }
    assert( PI != ports.end() );
    return ((*PI)->getBitSize() > size);
  }
  virtual std::string getDescription()
  {
    std::stringstream ret;
    ret << "(portSize(" << port_num << ") > " << size << ")";
    return ret.str();
  }
};

class FilterCoreStreamSizeIsGreater : public Filter {
  int stream_num;
  int size;
public:
  FilterCoreStreamSizeIsGreater(int p, int s):stream_num(p),size(s){}
  bool operator()(LibraryEntry* c)
  {
    std::list<Database::Stream> streams = c->getStreams();
    std::list<Database::Stream>::const_iterator SI = streams.begin();
    for(int i = 0; i < stream_num and SI != streams.end(); ++i)
    {
      ++SI;
    }
    assert( SI != streams.end() );
    assert( SI->getDataChannels().size() > 0 );
    return (SI->getDataChannels().front()->getBitSize() > size);
  }
  virtual std::string getDescription()
  {
    std::stringstream ret;
    ret << "(streamSize(" << stream_num << ") > " << size << ")";
    return ret.str();
  }
};

class WeightedSort : public Sort {
public:
  WeightedSort(){}
  virtual int getWeight()=0;
};

class MultiSortTieredByWeight : public MultiSortTiered {
public:
  MultiSortTieredByWeight(){}
  virtual MultiSortTiered& add(Sort* s)
  {
    WeightedSort* ws = dynamic_cast<WeightedSort*>(s);
    assert(ws and "Can only add a weighted sort to a TieredByWeight sort!");
    std::list<Sort*>::iterator SI = sorts.begin();
    for(; SI != sorts.end(); ++SI)
    {
      WeightedSort* si = dynamic_cast<WeightedSort*>(*SI);
      assert(si);
      if( si->getWeight() < ws->getWeight() )
        break;
    }
    sorts.insert(SI, ws);
    return *this;
  }
};

class SortCorePortIsLargerSize : public WeightedSort {
  int port_num;
  int weight;
public:
  SortCorePortIsLargerSize(int p, int w):port_num(p), weight(w){}
  virtual bool operator()(LibraryEntry* c, LibraryEntry* d)
  {
    std::list<Database::Port*> cports = c->getNonStreamPorts();
    std::list<Database::Port*> dports = d->getNonStreamPorts();
    std::list<Database::Port*>::const_iterator PI1 = cports.begin();
    std::list<Database::Port*>::const_iterator PI2 = dports.begin();
    for(int i = 0; i < port_num and PI1 != cports.end() and PI2 != dports.end(); ++i)
    {
      ++PI1;
      ++PI2;
    }
    assert( PI1 != cports.end() );
    assert( PI2 != dports.end() );
    return ((*PI1)->getBitSize() > (*PI2)->getBitSize());
  }
  virtual std::string getDescription()
  {
    std::stringstream ret;
    ret << "(portSize(" << port_num << ") > [w=" << weight << "])";
    return ret.str();
  }
  virtual int getWeight(){return weight;}
};

class SortCoreStreamIsLargerSize : public WeightedSort {
  int stream_num;
  int weight;
public:
  SortCoreStreamIsLargerSize(int p, int w):stream_num(p), weight(w){}
  virtual bool operator()(LibraryEntry* c, LibraryEntry* d)
  {
    std::list<Database::Stream> cstreams = c->getStreams();
    std::list<Database::Stream> dstreams = d->getStreams();
    std::list<Database::Stream>::const_iterator SI1 = cstreams.begin();
    std::list<Database::Stream>::const_iterator SI2 = dstreams.begin();
    for(int i = 0; i < stream_num and SI1 != cstreams.end() and SI2 != dstreams.end(); ++i)
    {
      ++SI1;
      ++SI2;
    }
    assert( SI1 != cstreams.end() );
    assert( SI2 != dstreams.end() );
    assert( SI1->getDataChannels().size() > 0 );
    assert( SI2->getDataChannels().size() > 0 );
    return (SI1->getDataChannels().front()->getBitSize() > SI2->getDataChannels().front()->getBitSize());
  }
  virtual std::string getDescription()
  {
    std::stringstream ret;
    ret << "(streamSize(" << stream_num << ") > [w=" << weight << "])";
    return ret.str();
  }
  virtual int getWeight(){return weight;}
};

class SortCorePortIsSmallerSize : public WeightedSort {
  int port_num;
  int weight;
public:
  SortCorePortIsSmallerSize(int p, int w):port_num(p), weight(w){}
  virtual bool operator()(LibraryEntry* c, LibraryEntry* d)
  {
    std::list<Database::Port*> cports = c->getNonStreamPorts();
    std::list<Database::Port*> dports = d->getNonStreamPorts();
    std::list<Database::Port*>::const_iterator PI1 = cports.begin();
    std::list<Database::Port*>::const_iterator PI2 = dports.begin();
    for(int i = 0; i < port_num and PI1 != cports.end() and PI2 != dports.end(); ++i)
    {
      ++PI1;
      ++PI2;
    }
    assert( PI1 != cports.end() );
    assert( PI2 != dports.end() );
    return ((*PI1)->getBitSize() < (*PI2)->getBitSize());
  }
  virtual std::string getDescription()
  {
    std::stringstream ret;
    ret << "(portSize(" << port_num << ") < [w=" << weight << "])";
    return ret.str();
  }
  virtual int getWeight(){return weight;}
};

class SortCoreStreamIsSmallerSize : public WeightedSort {
  int stream_num;
  int weight;
public:
  SortCoreStreamIsSmallerSize(int p, int w):stream_num(p), weight(w){}
  virtual bool operator()(LibraryEntry* c, LibraryEntry* d)
  {
    std::list<Database::Stream> cstreams = c->getStreams();
    std::list<Database::Stream> dstreams = d->getStreams();
    std::list<Database::Stream>::const_iterator SI1 = cstreams.begin();
    std::list<Database::Stream>::const_iterator SI2 = dstreams.begin();
    for(int i = 0; i < stream_num and SI1 != cstreams.end() and SI2 != dstreams.end(); ++i)
    {
      ++SI1;
      ++SI2;
    }
    assert( SI1 != cstreams.end() );
    assert( SI2 != dstreams.end() );
    assert( SI1->getDataChannels().size() > 0 );
    assert( SI2->getDataChannels().size() > 0 );
    return (SI1->getDataChannels().front()->getBitSize() < SI2->getDataChannels().front()->getBitSize());
  }
  virtual std::string getDescription()
  {
    std::stringstream ret;
    ret << "(streamSize(" << stream_num << ") < [w=" << weight << "])";
    return ret.str();
  }
  virtual int getWeight(){return weight;}
};

class FilterCoreIsType : public Filter {
  LibraryEntry::TYPE type;
public:
  FilterCoreIsType(LibraryEntry::TYPE t):type(t){}
  virtual bool operator()(LibraryEntry* c)
  {
    return (c->getType() == type);
  }
  virtual std::string getDescription()
  {
    std::stringstream ret;
    ret << "(type() == " << LibraryEntry::convTypeToString(type) << ")";
    return ret.str();
  }
};

class FilterCoreStreamIsNumAddressChannels : public Filter {
  int stream_num;
  int num_channels;
public:
  FilterCoreStreamIsNumAddressChannels(int p, int s):stream_num(p),num_channels(s){}
  bool operator()(LibraryEntry* c)
  {
    std::list<Database::Stream> streams = c->getStreams();
    std::list<Database::Stream>::const_iterator SI = streams.begin();
    for(int i = 0; i < stream_num and SI != streams.end(); ++i)
    {
      ++SI;
    }
    assert( SI != streams.end() );
    return (SI->getAddressChannelsBase().size() == num_channels);
  }
  virtual std::string getDescription()
  {
    std::stringstream ret;
    ret << "(streamSize(" << stream_num << ") > " << num_channels << ")";
    return ret.str();
  }
};

class FilterCoreStreamIsNumDataChannels : public Filter {
  int stream_num;
  int num_channels;
public:
  FilterCoreStreamIsNumDataChannels(int p, int s):stream_num(p),num_channels(s){}
  bool operator()(LibraryEntry* c)
  {
    std::list<Database::Stream> streams = c->getStreams();
    std::list<Database::Stream>::const_iterator SI = streams.begin();
    for(int i = 0; i < stream_num and SI != streams.end(); ++i)
    {
      ++SI;
    }
    assert( SI != streams.end() );
    return (SI->getDataChannels().size() == num_channels);
  }
  virtual std::string getDescription()
  {
    std::stringstream ret;
    ret << "(streamSize(" << stream_num << ") > " << num_channels << ")";
    return ret.str();
  }
};

const llvm::Type* getBaseType(const llvm::Type* t)
{
  if( t->getTypeID() == Type::PointerTyID or t->getTypeID() == Type::ArrayTyID )
  {
    const SequentialType* ST = dynamic_cast<const SequentialType*>( t );
    assert( ST and "Problem trying to figure out a type of a value." ); 
    return getBaseType( ST->getElementType() );
  }
  return t;
}

void processArgument(llvm::Value* v, int scalarNum, int streamNum, MultiFilterAnd& mf, MultiSortTiered& ms)
{
  bool isScalar = (getBaseType(v->getType()) == v->getType());
  if( dynamic_cast<Constant*>(v) )
  {
    INTERNAL_WARNING("Not using size of constant as search criteria!\n");
  }
  else if(getBaseType(v->getType())->isFloatingPoint())
  {
    if( isScalar )
      mf.add( new FilterCorePortSizeIsEqual(scalarNum, getSizeInBits(v)) );
    else
      mf.add( new FilterCoreStreamSizeIsEqual(streamNum, getSizeInBits(v)) );
  }
  else if(getBaseType(v->getType())->isInteger())
  {
    if( isScalar )
    {
      mf.add( new FilterCorePortSizeIsGreater(scalarNum, getSizeInBits(v)-1)  );
      ms.add( new SortCorePortIsSmallerSize(streamNum, getSizeInBits(v)) );
    }
    else
    {
      mf.add( new FilterCoreStreamSizeIsGreater(scalarNum, getSizeInBits(v)-1)  );
      ms.add( new SortCoreStreamIsSmallerSize(streamNum, getSizeInBits(v)) );
    }
  }
  else if(getBaseType(v->getType())->getTypeID() == llvm::Type::VoidTyID)
  {
  
  }
  else
  {
    INTERNAL_SERIOUS_WARNING("Not using size of non-integer, non-float (" << v->getType()->getDescription() << ") as search criteria!\n");
  }
}

void processArgumentRelaxed(llvm::Value* v, int scalarNum, int streamNum, MultiFilterAnd& mf, MultiSortTiered& ms)
{
  bool isScalar = (getBaseType(v->getType()) == v->getType());
  if( dynamic_cast<Constant*>(v) )
  {
    INTERNAL_WARNING("Not using size of constant as search criteria!\n");
  }
  else if(getBaseType(v->getType())->isFloatingPoint())
  {
    if( isScalar )
      mf.add( new FilterCorePortSizeIsEqual(scalarNum, getSizeInBits(v)) );
    else
      mf.add( new FilterCoreStreamSizeIsEqual(streamNum, getSizeInBits(v)) );
  }
  else if(getBaseType(v->getType())->isInteger())
  {
    if( isScalar )
      ms.add( new SortCorePortIsLargerSize(scalarNum, getSizeInBits(v)) );
    else
      ms.add( new SortCoreStreamIsLargerSize(streamNum, getSizeInBits(v)) );
  }
  else if(getBaseType(v->getType())->getTypeID() == llvm::Type::VoidTyID)
  {
  
  }
  else
  {
    INTERNAL_SERIOUS_WARNING("Not using size of non-integer, non-float (" << v->getType()->getDescription() << ") as search criteria!\n");
  }
}

std::string printInstruction(llvm::Instruction* II)
{
  std::stringstream ss;
  if( II->getType()->getTypeID() == llvm::Type::VoidTyID )
  {
    ss << *II;
    return ss.str();
  }
  if( II->getType()->isFloatingPoint() )
    ss << "f" << getSizeInBits(II) << " ";
  else if( II->getType()->isInteger() )
    ss << "i" << getSizeInBits(II) << " ";
  ss << getValueName(II) << " = " << II->getOpcodeName() << " ";
  for(Instruction::op_iterator OP = II->op_begin(); OP != II->op_end(); ++OP)
  {
    if( OP != II->op_begin() )
      ss << ", ";
    if( getValueName(*OP) == "" )
    {
      ss << **OP;
    }
    else
    {
      if( (*OP)->getType()->getTypeID() == llvm::Type::FloatTyID )
        ss << "f";
      else
        ss << "i";
      ss << getSizeInBits(*OP) << " ";
      ss << getValueName(*OP);
    }
  }
  return ss.str();
}

Value* getIntrinsicOperandNumbered(Instruction* II, int current_op_num)
{
  //v is the value that we are currently processing; first, all the arguments, then the instruction's return value, as the last argument
  Value* v = NULL;
  if( current_op_num > II->getNumOperands() )
  {
    INTERNAL_ERROR("Current operand number, " << current_op_num << ", is greater than actual number of operands, " << II->getNumOperands() << ", when processing " << *II << "!\n");
    assert(0 and "Invalid number of operands!");
  }
  else if( current_op_num == II->getNumOperands() )
  {
    v = II;
  }
  else
  {
    v = II->getOperand(current_op_num);
  }
  return v;
}

std::string getCoreThatMatchesRecursivelyRelaxedRequirements(std::list<LibraryEntry*> cores, MultiFilterAnd mf, MultiSortTieredByWeight ms, Instruction* II, unsigned current_op_num=0, unsigned current_scalar_num=0, unsigned current_stream_num=0)
{
  bool is_callInst = dynamic_cast<CallInst*>(II);
  int actual_op_num = current_op_num, actual_scalar_num = current_scalar_num, actual_stream_num = current_stream_num;
  if( is_callInst )
  {
    actual_op_num += 1;
  }
  //we are creating the filters recursively, and only checking for valid intrinsics when we have processed all of the arguments.
  //are we done adding search parameters on a operand-by-operand basis? Then return!
  if( actual_op_num > II->getNumOperands() )
  {
    std::list<LibraryEntry*> temp = filterIntrinsics(cores, &mf);
    std::list<LibraryEntry*> ret = sortIntrinsics(temp, &ms);
    if( ret.begin() != ret.end() )
    {
      return (*ret.begin())->getName();
    }
    return "";
  }
  //if its a call instruction, we ignore the first argument, and also subtract 1 from the current_op_num when processingArgument()
  Value* v = getIntrinsicOperandNumbered(II, actual_op_num);
  bool isScalar = (getBaseType(v->getType()) == v->getType());
  if( is_callInst )
  {
    if( isScalar )
      actual_scalar_num += 1;
    else
      actual_stream_num += 1;
  }
  assert(actual_op_num == actual_scalar_num + actual_stream_num);
  MultiFilterAnd mf_copy = mf;
  MultiSortTieredByWeight ms_copy = ms;
  processArgument(v, current_scalar_num, current_stream_num, mf_copy, ms_copy);
  std::string ret;
  if( isScalar )
    ret = getCoreThatMatchesRecursivelyRelaxedRequirements(cores, mf_copy, ms_copy, II, current_op_num+1, current_scalar_num+1, current_stream_num);
  else
    ret = getCoreThatMatchesRecursivelyRelaxedRequirements(cores, mf_copy, ms_copy, II, current_op_num+1, current_scalar_num, current_stream_num+1);
  if( ret != "" )
    return ret;
  LOG_MESSAGE1("Intrinsic Replacement", "Could not find exact match for \'" << printInstruction(II) << "\'; relaxing requirements on " << getValueName(v) << ".\n" 
                                        << "Note: this can often happen if you are using an intrinsic operation (floating point operations, integer divide) of a "
                                        << "bit size that is different than the bitsize of the currently activated intrinsic.\n" );
  mf_copy = mf;
  ms_copy = ms;
  processArgumentRelaxed(v, current_scalar_num, current_stream_num, mf_copy, ms_copy);
  if( isScalar )
    return getCoreThatMatchesRecursivelyRelaxedRequirements(cores, mf_copy, ms_copy, II, current_op_num+1, current_scalar_num+1, current_stream_num);
  else
    return getCoreThatMatchesRecursivelyRelaxedRequirements(cores, mf_copy, ms_copy, II, current_op_num+1, current_scalar_num, current_stream_num+1);
}

std::string printIntrinsic(LibraryEntry le)
{
  std::stringstream ss;
  std::list<Database::Port*> ports = le.getNonDebugScalarPorts();
  std::vector<Database::Port*> input_ports, output_ports;
  for(std::list<Database::Port*>::iterator PI = ports.begin(); PI != ports.end(); ++PI)
  {
    if( (*PI)->isOutput() )
      output_ports.push_back(*PI);
    else
      input_ports.push_back(*PI);
  }
  if( output_ports.size() > 1 )
    ss << "(";
  for(std::vector<Database::Port*>::iterator PI = output_ports.begin(); PI != output_ports.end(); ++PI)
  {
    if( PI != output_ports.begin() )
      ss << ", ";
    ss << "b" << (*PI)->getBitSize();
  }
  if( output_ports.size() > 1 )
    ss << ") ";
  if( output_ports.size() == 0 )
    ss << "void";
  ss << " " << le.getName();
  ss << "(";
  for(std::vector<Database::Port*>::iterator PI = input_ports.begin(); PI != input_ports.end(); ++PI)
  {
    if( PI != input_ports.begin() )
      ss << ", ";
    ss << "b" << (*PI)->getBitSize();
  }
  ss << ")";
  return ss.str();
}

std::string getCoreThatMatches(LibraryEntry::TYPE type, Instruction* II)
{
  DatabaseInterface* dbInterface = DatabaseInterface::getInstance();

  std::list<LibraryEntry*> cores = filterIntrinsics(dbInterface->getCores(), new FilterCoreIsType(type));
  if( cores.size() == 0 )
  {
    INTERNAL_ERROR("The database has no matching cores for " << LibraryEntry::convTypeToString(type) << ".\n");
  }
  assert( cores.size() != 0 and "No intrinsics of that type in database!" );
  MultiFilterAnd mf;
  MultiSortTieredByWeight ms;
  std::string ret = getCoreThatMatchesRecursivelyRelaxedRequirements(cores, mf, ms, II);
  if( ret != "" )
  {    
    LOG_MESSAGE1("Intrinsic Replacement", "Using intrinsic \'" << printIntrinsic(dbInterface->LookupEntry(ret)) << "\' as closest corresponding intrinsic for \'" << printInstruction(II) << "\'.\n");
    return ret;
  }
  INTERNAL_ERROR("Cannot find intrinsic for " << printInstruction(II) << "\n");
  assert(0);
}

Instruction* splitConvertIntoSize(CallInst* CI, const Type* newType)
{
  assert(CI->getNumOperands() == 3);
  Value* val = CI->getOperand(1);
  //newCI is the copy we are going to replace the convert with
  Instruction* newCI = NULL;
  //if the types are the same, handle them with the llvm builtins
  if( newType->getTypeID() == val->getType()->getTypeID() )
  {
    if( newType->getTypeID() == llvm::Type::FloatTyID )
      newCI = CastInst::createFPCast(val, newType, CI->getName(), CI);
    else
      newCI = CastInst::createIntegerCast(val, newType, true, CI->getName(), CI);
  }
  //otherwise, we need to use a bitcast instruction
  else
  {
    newCI = new BitCastInst(val, newType, CI->getName(), CI);
  }
  std::string oldName = getValueName(CI);
  CI->replaceAllUsesWith(newCI);
  ConstantInt* constInt = dynamic_cast<ConstantInt*>( CI->getOperand(2));
  assert( constInt and "Conversion variable size must be a constant int!" );
  setSizeInBits(newCI, constInt->getValue().getSExtValue());
  setValueName(newCI, oldName);
  CI->eraseFromParent();
  return newCI;
}

void addDebugCall(Value* val, Instruction* insertBefore)
{
  assert(insertBefore);
  assert(insertBefore->getParent());
  assert(insertBefore->getParent()->getParent());
  Module* M = insertBefore->getParent()->getParent()->getParent();
  assert(M);
  Function* debugFunc = M->getFunction(ROCCCNames::DebugScalarOutput);
  if( !debugFunc )
  {
    std::vector<const Type*> paramTypes;
    paramTypes.push_back( val->getType() );

    FunctionType* ft = FunctionType::get(Type::VoidTy, paramTypes, false);

    debugFunc = Function::Create(ft,
  				   (GlobalValue::LinkageTypes)0,
  				   ROCCCNames::DebugScalarOutput, 
  				   M );
  }
	//now create the actual call instruction
  std::vector<Value*> valArgs;
  valArgs.push_back(val);
	CallInst::Create( debugFunc,
          			    valArgs.begin(),
          			    valArgs.end(),
          			    "" ,
          			    insertBefore);
}

// This is the entry point to our pass and where all of our work gets done
bool ROCCCIntrinsicPass::runOnModule(Module& M)
{
  CurrentFile::set(__FILE__);
  //resetSizeMap();
  bool changed = false;
  
  for( Module::iterator f = M.begin(); f != M.end(); ++f )
  {
    for( Function::iterator BB = f->begin(); BB != f->end(); ++BB )
    {
      //for each instruction, check if its an operand that needs
      //to be replaced with an ipcore, and if so, replace it     
      for( BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II )
      {
  	    std::string instrName = II->getOpcodeName();
    	  GlobalValue* name = NULL;
    	  Instruction* replace_val = &*II;
    	  int size = 0;
    	  //integer division is the only integer operation needing to
    	  //use an ipcore
    	  if( II->getType()->isInteger() )
    	  {
    	    //if it is a signed divide, replace it
    	    if( instrName=="sdiv" )
    	    {
    	      size = getSizeInBits(&*II);
    	 	    name = getOrCreateNamedGlobalString(M, getCoreThatMatches(LibraryEntry::INT_DIV, &*II));
    	 	  }
    	 	  //if it is a signed remainder (mod), replace it
    	 	  else if( instrName=="srem" )
    	 	  {
    	      size = getSizeInBits(&*II);
    	 	    name = getOrCreateNamedGlobalString(M, getCoreThatMatches(LibraryEntry::INT_MOD, &*II));
    	 	  }
    	 	  CallInst* CI = dynamic_cast<CallInst*>(&*II);
    	 	  if( isROCCCFunctionCall(CI, ROCCCNames::ConvertFloatToInt) )
    	 	  {
    	 	    replace_val = splitConvertIntoSize(CI, CI->getType());
    	 	    size = getSizeInBits(replace_val);
    	 	    name = getOrCreateNamedGlobalString(M, getCoreThatMatches(LibraryEntry::FP_TO_INT, replace_val));
    	 	  }
    	 	  else if( isROCCCFunctionCall(CI, ROCCCNames::ConvertIntToInt) )
    	 	  {
    	 	    replace_val = splitConvertIntoSize(CI, CI->getType());
    	 	    II = BB->begin();
    	 	    continue;
    	 	  }
    	 	  if( instrName=="fcmp" )
    	 	  {
    	      size = getSizeInBits(&*II);
    	 	    FCmpInst* CMPI = dynamic_cast<FCmpInst*>(&*II);
    	 	    assert(CMPI);
    	 	    switch( CMPI->getPredicate() )
    	 	    {
    	 	      case FCmpInst::FCMP_UEQ : //fall through
    	 	      case FCmpInst::FCMP_OEQ : name = getOrCreateNamedGlobalString(M, getCoreThatMatches(LibraryEntry::FP_EQ, &*II)); break;
    	 	      case FCmpInst::FCMP_UNE : //fall through
    	 	      case FCmpInst::FCMP_ONE : name = getOrCreateNamedGlobalString(M, getCoreThatMatches(LibraryEntry::FP_NEQ, &*II)); break;
    	 	      case FCmpInst::FCMP_ULT : //fall through
    	 	      case FCmpInst::FCMP_OLT : name = getOrCreateNamedGlobalString(M, getCoreThatMatches(LibraryEntry::FP_LT, &*II)); break;
    	 	      case FCmpInst::FCMP_UGT : //fall through
    	 	      case FCmpInst::FCMP_OGT : name = getOrCreateNamedGlobalString(M, getCoreThatMatches(LibraryEntry::FP_GT, &*II)); break;
    	 	      case FCmpInst::FCMP_ULE : //fall through
    	 	      case FCmpInst::FCMP_OLE : name = getOrCreateNamedGlobalString(M, getCoreThatMatches(LibraryEntry::FP_LTE, &*II)); break;
    	 	      case FCmpInst::FCMP_UGE : //fall through
    	 	      case FCmpInst::FCMP_OGE : name = getOrCreateNamedGlobalString(M, getCoreThatMatches(LibraryEntry::FP_GTE, &*II)); break;
    	 	      default : INTERNAL_ERROR("Cannot handle floating point compare with predicate " << CMPI->getPredicate() << "!\n"); assert(0 and "Cannot handle fpcmp predicate!");
    	 	    }
    	 	  }
    	  }
    	  //all floating point operations need to be done with ipcores
    	  else if ( II->getType()->isFloatingPoint() )
    	  {
    	    size = getSizeInBits(&*II);
    	 	  CallInst* CI = dynamic_cast<CallInst*>(&*II);
    	 	  if( isROCCCFunctionCall(CI, ROCCCNames::ConvertIntToFloat) )
    	 	  {
    	 	    replace_val = splitConvertIntoSize(CI, Type::FloatTy);
    	 	    size = getSizeInBits(replace_val);
    	 	    name = getOrCreateNamedGlobalString(M, getCoreThatMatches(LibraryEntry::INT_TO_FP, replace_val));
    	 	  }
    	    else if( instrName=="mul" )
    	 	    name = getOrCreateNamedGlobalString(M, getCoreThatMatches(LibraryEntry::FP_MUL, &*II));
    	    else if( instrName=="sub" )
    	 	    name = getOrCreateNamedGlobalString(M, getCoreThatMatches(LibraryEntry::FP_SUB, &*II));
    	    else if( instrName=="fdiv" )
    	 	    name = getOrCreateNamedGlobalString(M, getCoreThatMatches(LibraryEntry::FP_DIV, &*II));
    	    else if( instrName=="add" )
    	 	    name = getOrCreateNamedGlobalString(M, getCoreThatMatches(LibraryEntry::FP_ADD, &*II));
    	 	  //loads dont need an ipcore, but also shouldnt output
    	 	  //a warning, so they get their own empty block here
    	 	  else if( instrName=="load" )
    	 	    ;
    	 	  else if( isROCCCFunctionCall(CI, ROCCCNames::ConvertFloatToFloat) )
    	 	  {
    	 	    assert(CI->getNumOperands() == 3);
    	 	    int value_size = getSizeInBits(CI->getOperand(1));
    	 	    //this will turn the call into a bitcast, which we want to do regardless, to preserve naming
    	 	    replace_val = splitConvertIntoSize(CI, Type::FloatTy);
    	 	    size = getSizeInBits(replace_val);
    	 	    //only do this if the size is different, otherwise leave it as a bitcast!
    	 	    if( value_size != size )
    	 	    {
    	 	      name = getOrCreateNamedGlobalString(M, getCoreThatMatches(LibraryEntry::FP_TO_FP, replace_val));
    	 	    }
    	 	    else
    	 	    {
    	 	      //if we are leaving it alone, reset II, as we invalidated the pointer
    	 	      II = BB->begin();
    	 	      continue;
    	 	    }
    	 	  }
    		  else
    		  {
    		    INTERNAL_WARNING("Unknown instruction name " << instrName << "!\n");
    		  }
     	  }
     	  else
     	  {
     	    CallInst* CI = dynamic_cast<CallInst*>(&*II);
     	    if( isROCCCFunctionCall(CI, ROCCCNames::TripleVote) )
     	    {
     	      name = getOrCreateNamedGlobalString(M, getCoreThatMatches(LibraryEntry::TRIPLE_VOTE, &*II));
     	    }
     	    if( isROCCCFunctionCall(CI, ROCCCNames::DoubleVote) )
     	    {
     	      name = getOrCreateNamedGlobalString(M, getCoreThatMatches(LibraryEntry::DOUBLE_VOTE, &*II));
     	    }
     	    if( isROCCCFunctionCall(CI, ROCCCNames::StreamSplitter) )
     	    {
     	      name = getOrCreateNamedGlobalString(M, getCoreThatMatches(LibraryEntry::STREAM_SPLITTER, &*II));
          }
     	    if( isROCCCFunctionCall(CI, ROCCCNames::StreamDoubleVote) )
     	    {
     	      name = getOrCreateNamedGlobalString(M, getCoreThatMatches(LibraryEntry::STREAM_DOUBLE_VOTE, &*II));
          }
     	    if( isROCCCFunctionCall(CI, ROCCCNames::StreamTripleVote) )
     	    {
     	      name = getOrCreateNamedGlobalString(M, getCoreThatMatches(LibraryEntry::STREAM_TRIPLE_VOTE, &*II));
          }
     	  }
     	  if( name != NULL )
     	  {
     	    //replace the current instruction with a call to hardware
    	    Value* ci = convert_to_HW_call(replace_val, replace_val, name, size);
    	    std::string oldName = getValueName(replace_val);
    	 	  replace_val->replaceAllUsesWith(ci);
    	 	  setSizeInBits(ci, size);
          setValueName(ci, oldName);
    	 	  replace_val->eraseFromParent();
    	 	  //we have erased the current iterator; its just easier to start over
    	 	  II = BB->begin();
    	 	  changed = true;
     	  }
     	}
    }
  }
  return changed ; 
}

