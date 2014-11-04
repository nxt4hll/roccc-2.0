#include "rocccLibrary/GetValueName.h"

#include "llvm/Constants.h"
#include "llvm/Function.h"
#include "llvm/Module.h"

#include <map>

#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/ROCCCNames.h"
#include "rocccLibrary/CopyValue.h"
#include "rocccLibrary/DFBasicBlock.h"

using namespace llvm;

static std::map<Value*, std::string> nameMap;

std::string getValueName(Value* v)
{
  assert( v );
  //if this is the first time through the map, fill the map with the values we
  //  find from the ROCCCName calls
  if( nameMap.empty() )
  {
    if(Instruction* II = dynamic_cast<Instruction*>(v))
    {
      Function* f = II->getParent()->getParent();
      for(Function::iterator BB = f->begin(); BB != f->end(); ++BB)
      {
        for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
        {
          if(CallInst* CI = dynamic_cast<CallInst*>(&*II))
          {
            if( isROCCCFunctionCall(CI, ROCCCNames::VariableName) )
            {
              assert( CI->getNumOperands() == 3 );
              Constant* tmpCast = dynamic_cast<Constant*>(CI->getOperand(1));
              assert( tmpCast and "ROCCC variable name must be a constant!");
              assert( tmpCast->getStringValue() != "" and "ROCCC variable name must be an inline constant, and not a global!" );
              nameMap[CI->getOperand(2)] = tmpCast->getStringValue();
            }
          }
        }
      }
    }
  }
  if( nameMap.find(v) == nameMap.end() )
  {
  	//maybe we are a copy . . . check!
  	if( isCopyValue(v) )
  	{
  	  nameMap[v] = getValueName(dynamic_cast<Instruction*>(v)->getOperand(0));
  	}
  	//maybe its a call instruction returning void?
  	else if( v->getType()->getTypeID() == llvm::Type::VoidTyID )
  	{
  	  if( CallInst* CI = dynamic_cast<CallInst*>(v) )
  	  {
  	    nameMap[v] = CI->getCalledFunction()->getName();
  	  }
  	}
  }
  if( nameMap.find(v) == nameMap.end() )
  {
    INTERNAL_WARNING("No ROCCCName found for variable " << *v << "! Using default (" << v->getName() << ") name!\n");
    nameMap[v] = v->getName();
  }
  return nameMap.find(v)->second;
}

void setValueName(Value* v, std::string n)
{
  Instruction* II = dynamic_cast<Instruction*>(v);
  assert(II and "Cannot set name on a non-instruction!");
  assert(II->getParent());
  assert(II->getParent()->getParent());
  assert(II->getParent()->getParent()->getParent());
  Module* M = II->getParent()->getParent()->getParent();
  Function* nameFunc = M->getFunction(ROCCCNames::VariableName);
  assert(nameFunc);
  //create the insertion point
  DFBasicBlock* dfbb = new DFBasicBlock("", II->getParent()->getParent());
	//now create the actual call instruction
  std::vector<Value*> valArgs;
  //see if the value already exists
  llvm::Constant* name = M->getNamedGlobal(n);
  //create the global if it doesnt already exist
  if( name == NULL )
  {
    Constant* val = ConstantArray::get(n);
    name = new GlobalVariable(val->getType(), true, (GlobalValue::LinkageTypes)0, val, n, M);
  }
  name = ConstantExpr::getPointerCast(name, nameFunc->getFunctionType()->getParamType(0));
  assert(name);
  assert(nameFunc->getFunctionType()->getParamType(0) == name->getType());
  valArgs.push_back(name);
  valArgs.push_back(v);
	CallInst::Create( nameFunc,
          			    valArgs.begin(),
          			    valArgs.end(),
          			    "" ,
          			    dfbb->getTerminator());
  nameMap[v] = n;
}
