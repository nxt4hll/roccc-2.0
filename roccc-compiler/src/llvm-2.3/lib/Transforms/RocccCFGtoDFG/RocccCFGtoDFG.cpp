// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

This pass converts the LLVM-CFG to a ROCCC DFG, using the LLVM-CFG
as a framework. This is not a direct translation of the LLVM-DFG, as
certain definitions in LLVM are not definitions in ROCCC, and vise-versa,
such as ROCCCInputScalars being the definition for any values they take as
arguments.

*/

#include "llvm/Module.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/LoopPass.h"

#include <vector>
#include <map>
#include <sstream>

#include "rocccLibrary/DFBasicBlock.h"
#include "rocccLibrary/DFFunction.h"
#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/ROCCCNames.h"
#include "rocccLibrary/InductionVariableInfo.h"
#include "rocccLibrary/DefinitionInst.h"
#include "rocccLibrary/FunctionType.h"

using namespace std;
using namespace llvm;

namespace ROCCC {

bool actuallyUses(DFBasicBlock* use, DFBasicBlock* def)
{
  bool hasUse = false;
  bool wasUsed = false;
  for(BasicBlock::iterator defInst = def->begin(); defInst != def->end(); ++defInst)
  {
    for(BasicBlock::iterator II = use->begin(); II != use->end(); ++II)
    {
      for(unsigned int i = 0; i < II->getNumOperands(); ++i)
      {
        if( II->getOperand(i) == defInst )
        {
          wasUsed = true;
          //there is an instruction defInst in def, that is used in an
          //instruction II in use, at operand number i
          //this is where we actually decide if thats considered
          //a real use or not
          if(CallInst* CI = dynamic_cast<CallInst*>(&*II))
          {
            if( isROCCCInputStream(CI) )
            {
              if( 2 <= i and i <= static_cast<unsigned>(getROCCCStreamDimension(CI))+1 )
              {
                hasUse = hasUse or false;
              }
              else
                hasUse = true;
            }
            else
              hasUse = true;
          }
          else
            hasUse = true;
        }
        else if( isROCCCFunctionCall(dynamic_cast<CallInst*>(&*II), ROCCCNames::LoadPrevious) and isROCCCFunctionCall(dynamic_cast<CallInst*>(&*defInst), ROCCCNames::StoreNext) )
        {
          wasUsed = true;
          hasUse = hasUse or false;
        }
      }
    }
  }
  return !wasUsed or hasUse;
}

void AddUse(Value* _def, Value* _use)
{
  if( !dynamic_cast<BasicBlock*>(_def) or !dynamic_cast<BasicBlock*>(_use) )
    return;
  DFBasicBlock* def = dynamic_cast<BasicBlock*>(_def)->getDFBasicBlock();
  DFBasicBlock* use = dynamic_cast<BasicBlock*>(_use)->getDFBasicBlock();
  if ( !def || !use ) //Check for NULL pointers.
    return;
  if( actuallyUses(use, def) )
    def->AddUse( use );
}

//creates a blank DFBasicBlock
DFBasicBlock* DFBasicBlock_Create(string name, DFFunction* parent)
{
  assert( parent && "DFBB_Create() passed NULL parent!" );
  DFBasicBlock* new_bb = DFBasicBlock::Create( name, parent );
  //new UnreachableInst( new_bb );
  return new_bb;
}

//creates a blank DFFunction
DFFunction* DFFunction_Create( Function* F  )
{
  assert( F && "DFFunction_Create() passed NULL Function pointer!" );
  DFFunction* func = DFFunction::Create( F->getFunctionType(), 
					 GlobalValue::ExternalLinkage, 
					 "DF_" + F->getName(), 
					 F->getParent() ) ;
  
  //@TODO: we dont use arguments. . . YET
  //name the arguments
  for (Function::arg_iterator arg_f=F->arg_begin(), arg_df=func->arg_begin(); 
       arg_f != F->arg_end() and arg_df != func->arg_end(); 
       ++arg_f, ++arg_df)
  {
    assert(0 and "Cannot have arguments in processed function!");
    arg_df->setName( arg_f->getName() );
  }
  // Create an entry block with no successors, so llvm doesn't complain
  DFBasicBlock_Create("", func);
  return func;
}

//Add a source to the sourceHead, creating it if it doesnt already exist
void AddSource( DFBasicBlock* source, DFFunction* func )
{
  assert( func && "AddSource() passed a NULL function pointer!" );
  if ( !func->getSource() )
  {
    func->setSource( DFBasicBlock_Create("sourceHead", func ) );
  }
  if ( DFBasicBlock* sourceHead = dynamic_cast<DFBasicBlock*>(func->getSource()) )
  {
    AddUse( sourceHead, source );
  }
}

//add a sink to the sink head, creating it if it doesnt already exist
void AddSink( DFBasicBlock* sink, DFFunction* func )
{
  assert( func && "AddSink() passed a NULL function pointer!" );
  if ( !func->getSink() )
  {
    func->setSink( DFBasicBlock_Create("sinkHead", func) );
  }
  if ( DFBasicBlock* sinkHead = dynamic_cast<DFBasicBlock*>(func->getSink()) )
  {
    AddUse( sink, sinkHead );
  }
}

//ROCCCInputs are pulled to the top of the DFG. These are all values that
//will be controlled by the inputController.
bool isROCCCInput( Value* hold  )
{
  CallInst* CI = dynamic_cast<CallInst*>(hold);
  if (!CI)
    return false;
  return (isROCCCFunctionCall(CI, ROCCCNames::InputScalar) or
          isROCCCFunctionCall(CI, ROCCCNames::LoadPrevious) or
          isROCCCFunctionCall(CI, ROCCCNames::SystolicPrevious) or
          isROCCCFunctionCall(CI, ROCCCNames::InternalLUTDeclaration) or
          isROCCCInputStream(CI));
}

//ROCCCOutputs are pulled to the bottom of the DFG.
bool isROCCCOutput( Value* hold )
{
  CallInst* CI = dynamic_cast<CallInst*>(hold);
  if (!CI)
    return false;
  return (isROCCCFunctionCall(CI, ROCCCNames::OutputScalar) or 
          isROCCCOutputStream(CI) or
          isROCCCFunctionCall(CI, ROCCCNames::StoreNext) or
          isROCCCFunctionCall(CI, ROCCCNames::OutputInductionVariableEndValue) or
          isROCCCFunctionCall(CI, ROCCCNames::LUTOrder) or
          isROCCCFunctionCall(CI, ROCCCNames::FeedbackScalar) );
}

/*
Given a BasicBlock BB and a newly created DFFunction new_df_func,
take every instruction in BB, create a DFBasicBlock for it in new_df_func,
and insert the instruction into the new DFBasicBlock. If it is an input
or output, add it as an input or output.
*/ 
void pullBBIntoDFFunction( BasicBlock* BB, DFFunction* new_df_func )
{
  assert( BB && "pullBBIntoDFFunction() passed NULL BasicBlock pointer!" );
  assert( new_df_func && "pullBBIntoDFFunction() passed NULL function pointer!" );
  //add a basicblock whose name is the same as BB, for referencing purposes
  DFBasicBlock_Create( BB->getName(), new_df_func );

  for (BasicBlock::iterator II = BB->begin(); II != BB->end();)
  {
    //if its a terminator, replace it with an UnreachableInst
    if ( dyn_cast<TerminatorInst>(II) )
    {
      ReplaceInstWithInst(II->getParent()->getInstList(), II, new UnreachableInst() );
      ++II;
    }
    else //its not a terminator, put it in its own basicblock
    {
      // Create the DFBasicBlock with no name, 
      //  and insert it into the new DFFunction
      DFBasicBlock* head =  DFBasicBlock_Create("h", new_df_func);
      //move II into the newly created DFBB
      Instruction* hold = &*II;
      ++II;
      hold->removeFromParent();
      head->getInstList().push_front( hold );
      //check to see if its a roccc input or output, and set it accordingly
      if ( isROCCCInput(hold) )
      {
        AddSource( head, new_df_func );
      }
      if ( isROCCCOutput(hold) )
      {
        AddSink( head, new_df_func );
      }
      //check to see if its a use of the old function arguments, and replace if so
      //@TODO: is this necessary? Neither systems nor modules at hi_cirrf level
      //use arguments, can this be deleted?
      for (User::op_iterator op = hold->op_begin(); op != hold->op_end(); ++op)
      {
        if ( Argument* arg_f = dynamic_cast<Argument*>(op->get()) ) //if the operand is a reference to an argument
        {
          assert(0 and "Cannot have arguments in hi_cirrf!");
          Function::arg_iterator arg_df = new_df_func->arg_begin();
          for (unsigned int c = 0; c < arg_f->getArgNo(); c++)
          {
            ++arg_df;
            assert( arg_df != new_df_func->arg_end() && "Argument mismatch in function pullBBIntoDFFunction()." );
          }
          hold->replaceUsesOfWith( arg_f, &*arg_df );
        }
      }
    }
  }
}

//the canonical induction variable is the variable that is incremented
//by one each time through the loop.
//We are only looking for indexes of this type.
Value* getForLoopIndex( Loop* LB )
{
  assert( LB && "getForLoopIndex() passed NULL Loop pointer!" );
  return LB->getCanonicalInductionVariable();
}

//STUFF
Value* getRealCondition( TerminatorInst* TI )
{
  BranchInst *BI = dynamic_cast<BranchInst*>(TI);
  assert(BI and "Poorly formed loop!");
  assert(BI->isConditional() and "Poorly formed loop!");
  ICmpInst* cmp = dynamic_cast<ICmpInst*>( BI->getCondition() );
  assert( cmp and "Poorly formed loop!" );
  assert( cmp->getPredicate() == ICmpInst::ICMP_NE and "Poorly formed loop!" );
  assert( cmp->getOperand(1) == ConstantInt::get(APInt(8,0)) and "Poorly formed loop!" );
  return cmp->getOperand(0);
}

//Given a loop LB, find the condition of the headerblock of that loop,
//and return the value that the induction variable in that loop is being
//compared to. If this is neither a not-equal or a signed-less-than, then
//increment the value before returning it, as it is off by one.
Value* getForLoopEndValue( Loop* LB )
{  
  assert( LB && "getForLoopEndValue() passed NULL loop pointer!" );

  Instruction* IX = dynamic_cast<Instruction*>( getForLoopIndex(LB) );
  if ( !IX ) return NULL;
  BasicBlock* HeaderBlock = LB->getHeader();
  if ( !HeaderBlock ) return NULL;
  if ( Value* CV = getRealCondition(HeaderBlock->getTerminator()) )
  {
    //the loop's induction variable must be an integer, so the comparison
    //is going to be an integer comparison
    if ( ICmpInst* ICI = dynamic_cast<ICmpInst*>(CV) )
    {
      //the comparison is of the form ICMP_type Op1, Op2
      //the hi_cirrf guarantees that the induction variable is Op1, and the
      //value being compared to is Op2
      if ( ICI->getOperand(0) == IX )
      {
        //if its a not-equal or a signed less than, return the value
        //being compared, as it is the endValue
        if (ICI->getPredicate() == ICmpInst::ICMP_NE or
            ICI->getPredicate() == ICmpInst::ICMP_SLT)
        {
          return ICI->getOperand(1); //return the value that the index is being compared to
        }
        else
        {
          //@TODO: better hope, if we are getting here, that the comparison is
          //a less-than-or-equals . . . this is probably dead code, anyways
          if(ConstantInt* CI = dynamic_cast<ConstantInt*>(ICI->getOperand(1)))
          {
            //increment, as a workaround
            return ConstantInt::get(CI->getValue()+1);
          }
          else
          {
            //we arent comparing it correctly, nor are we even comparing it to a
            //constant . . . we're going to be generating off-by-one code, at best
            INTERNAL_SERIOUS_WARNING("Index variable " << IX->getName() << " not compared with NOT_EQUAL or SIGNED_LESS_THAN in loop header!\n");
            return ICI->getOperand(1);
          }
        }
      }
      else if ( ICI->getOperand(1) == IX )
      {
        //if its a not-equal or a signed less than, return the value
        //being compared, as it is the endValue
        if (ICI->getPredicate() == ICmpInst::ICMP_NE or
            ICI->getPredicate() == ICmpInst::ICMP_SGT)
        {
          return ICI->getOperand(0); //return the value that the index is being compared to
        }
        else
        {
          //@TODO: better hope, if we are getting here, that the comparison is
          //a less-than-or-equals . . . this is probably dead code, anyways
          if(ConstantInt* CI = dynamic_cast<ConstantInt*>(ICI->getOperand(0)))
          {
            //increment, as a workaround
            return ConstantInt::get(CI->getValue()+1);
          }
          else
          {
            //we arent comparing it correctly, nor are we even comparing it to a
            //constant . . . we're going to be generating off-by-one code, at best
            INTERNAL_SERIOUS_WARNING("Index variable " << IX->getName() << " not compared with NOT_EQUAL or SIGNED_GREATER_THAN in loop header!\n");
            return ICI->getOperand(0);
          }
        }
      }
      else
      {
        INTERNAL_ERROR( *ICI << *IX );
        assert(0 and "First operand to loop comparison must be loop induction variable!");
      }
    }
  }
  return NULL;
} 

//is it a store instruction? than make some special connections.
//LoadPrevious are no longer stores, and have to be 
//searched for explicitly when dealing with them, but now
//StoreNexts are stores.
bool isStore( Value* v )
{
  if ( dynamic_cast<StoreInst*>(v) )
    return true;
  CallInst* CI = dynamic_cast<CallInst*>(v);
  if ( isROCCCFunctionCall(CI, ROCCCNames::LoadPrevious) )
    return false; //changed to remove loops with inputScalar in dfg
  if ( isROCCCFunctionCall(CI, ROCCCNames::StoreNext) )
    return true; //changed to remove loops with loadPrevious in dfg
  if ( isROCCCFunctionCall(CI, ROCCCNames::SystolicNext) )
    return false;
  return false;
}

//if its a store, return the value that this store is storing into
Instruction* getStoreDestination( Value* v )
{
  Value* ret = NULL;
  if ( StoreInst* si = dynamic_cast<StoreInst*>(v) )
    ret = si->getOperand(1);
  CallInst* CI = dynamic_cast<CallInst*>(v);
  if ( isROCCCFunctionCall(CI, ROCCCNames::LoadPrevious) )
    ret = CI->getOperand(1);
  if ( isROCCCFunctionCall(CI, ROCCCNames::StoreNext) )
    ret = CI->getOperand(1);
  return dynamic_cast<Instruction*>(ret);
}

//given a DFBasicBlock _def, and a DFBasicBlock _use, add each use of
//_def as a use of _use.
//This means, each successor block of _def becomes a successor of _use.
void AddAllUsesOfAsUse( Value* _def, Value* _use )
{
  DFBasicBlock* def = dynamic_cast<DFBasicBlock*> (_def);
  DFBasicBlock* use = dynamic_cast<DFBasicBlock*> (_use);
  if ( !def || !use )
    return;
  Instruction* ai = dynamic_cast<Instruction*>( &*(use->begin()) );
  assert( ai && "Use added in AddAllUses that is not instruction!" );
  for (Value::use_iterator i = ai->use_begin(), e = ai->use_end(); i != e; ++i)
  {
    Instruction* ai2 = dynamic_cast<Instruction*>(*i);
    assert(ai2 and "Serious error.");
    AddUse( def, ai2->getParent() );
  }
}

/*
llvm-gcc represents booleans as 32 bits (or maybe 8 bits, but thats 7 bits
too many). This takes a BoolSelect call and a condition that is 1-bit, and
converts the BoolSelect to use the new condition instead of the extended
version of the condition. After this is done, the whole extending can be
deleted, and we save doing an uneccessary conversion.
*/
CallInst* getBoolSelectCall(Instruction* old_bs, Value* cond)
{
  Value* true_val = old_bs->getOperand(1);
  Value* false_val = old_bs->getOperand(2);
  std::string name = old_bs->getName();
  Module* M = old_bs->getParent()->getParent()->getParent();
  
  std::vector<const Type*> paramTypes;
  paramTypes.push_back(true_val->getType());
  paramTypes.push_back(false_val->getType());
  paramTypes.push_back(cond->getType());

  llvm::FunctionType* ft = llvm::FunctionType::get(old_bs->getType(), paramTypes, false);
  Function* rocccInHw = Function::Create(ft,
				                      (GlobalValue::LinkageTypes)0,
				                       ROCCCNames::BoolSelect, 
				                       M );

  std::vector<Value*> valArgs;
  valArgs.push_back( true_val );
  valArgs.push_back( false_val );
  valArgs.push_back( cond );

  return CallInst::Create( rocccInHw,
			                     valArgs.begin(),
			                     valArgs.end(),
			                     name,
			                     old_bs);
}

/*
llvm-gcc makes really inefficient comparisons, probably because software cant 
use 1-bit variables (suckers!). This removes all the overhead of the comparisons,
such as zero extension and recomparison.
*/
bool CompactCompares( Function& F )
{
  bool changed = false;
  for ( Function::iterator BB = F.begin(); BB != F.end(); ++BB )
  {
    for ( BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II )
    {
      if ( ICmpInst* CI = dynamic_cast<ICmpInst*>(&*II) )
      {
        //check for loop induction variables / ifs, and compact them
        ZExtInst* ZI = dynamic_cast<ZExtInst*>(CI->getOperand(0));
        ConstantInt* conInt = dynamic_cast<ConstantInt*>(CI->getOperand(1));
        if ( CI->getPredicate() == ICmpInst::ICMP_NE && ZI && conInt->getValue() == APInt(conInt->getBitWidth(), 0) && ZI->getNumUses() == 1 )
        {
          ICmpInst* CR = dynamic_cast<ICmpInst*>(ZI->getOperand(0));
          if ( CR && CR->getNumUses() == 1 )
          {
            CI->replaceAllUsesWith( CR );
            CI->eraseFromParent();
            ZI->eraseFromParent();
            II = BasicBlock::iterator( CR );
            changed = true;
          }
        }
        //check for BoolSelects, and compact them
        else
        {
          ZI = dynamic_cast<ZExtInst*>(*CI->use_begin());
          if( CI->hasOneUse() and ZI )
          {
            CallInst* old_bs = dynamic_cast<CallInst*>(*ZI->use_begin());
            if( ZI->hasOneUse() and isROCCCFunctionCall(old_bs, ROCCCNames::BoolSelect) )
            {
              Instruction* new_bs = getBoolSelectCall( old_bs, CI );
              old_bs->replaceAllUsesWith( new_bs );
              old_bs->eraseFromParent();
              ZI->eraseFromParent();
              II = BasicBlock::iterator( new_bs );
              changed = true;
            }
          }
        }
      }
    }
  }
  return changed;
}

struct RocccIfCompactPass : public FunctionPass
{
  static char ID;
  
  RocccIfCompactPass() : FunctionPass((intptr_t)&ID) {}
  virtual void getAnalysisUsage(AnalysisUsage &AU) const
  {
    AU.addRequired<DominatorTree>();
  }
  virtual bool runOnFunction(Function &F)
  {
    CurrentFile::set(__FILE__);
    return unsafeRunOnFunction( F );
  }
  bool unsafeRunOnFunction( Function& F )
  {
    if ( dynamic_cast<DFFunction*>(&F) )
      return false;

    bool changed = CompactCompares( F );
            
    return changed;
  }
};

char RocccIfCompactPass::ID = 0;
RegisterPass<RocccIfCompactPass> X_ROCCC_IF_COMPACT_PASS("rocccIfCompact", "Compacts compares and reduces large phis to phis with max 2 sources.");
  
struct RocccFunctionInfoPass : public FunctionPass
{
  static char ID;
  typedef map<PHINode*, Instruction*> IfMap;
  static ROCCCLoopInformation RInfo;
  IfMap phiNodeConditionMap;

  RocccFunctionInfoPass() : FunctionPass((intptr_t)&ID) {}
  virtual void getAnalysisUsage(AnalysisUsage &AU) const
  {
    AU.setPreservesAll();
    AU.addRequired<LoopInfo>();
  }
  ROCCCLoopInformation getInfo()
  {
    return RInfo;
  }
  IfMap getIfMap()
  {
    return phiNodeConditionMap;
  }
  virtual bool runOnFunction(Function &F)
  {
    CurrentFile::set(__FILE__);
    return unsafeRunOnFunction( F );
  }
  bool unsafeRunOnFunction(Function& F)
  {
    if( RInfo.indexes.begin() != RInfo.indexes.end() )
      return false;
    //setup the anaylsis, necessary to do loop analysis
    LoopInfo* LI = &getAnalysis<LoopInfo>();

    //start doing control flow analysis and getting LoopInformation
    //first, check to see if we have a loop, and if it has a canonical induction variable 
    //( an integer recurrence that starts at 0 and increments by one each time through the loop )
    for( Function::iterator BB = F.begin(); BB != F.end(); ++BB )
    {
      if( LI->getLoopFor(BB) )
      {
        Value* index = getForLoopIndex(LI->getLoopFor(BB));
        assert(index and "index variable unable to be detected!");
        if( find( RInfo.indexes.begin(), RInfo.indexes.end(), index ) == RInfo.indexes.end() ) //its not already in the list of indexes
        {
          //this index has not been added yet, so add it to
          //the list of indexes, add the endValue, and add the children info
          RInfo.indexes.push_back( index );
          Value* end = getForLoopEndValue(LI->getLoopFor(BB));
          RInfo.endValues[index] = end;
          //get the parent loop, if any
          Instruction* ins = dynamic_cast<Instruction*>(index);
          assert(ins and "index must be an instruction!");
          BasicBlock* pred = *(++pred_begin( ins->getParent() ));
          if( LI->getLoopFor(pred) )
          {
            Value* parentIndex = getForLoopIndex(LI->getLoopFor(pred) );
            assert(parentIndex);
            RInfo.loopChildren[parentIndex].push_back(index);
          }
        }
      }
    }
    return false;
  }
};

char RocccFunctionInfoPass::ID = 0;
ROCCCLoopInformation RocccFunctionInfoPass::RInfo;
RegisterPass<RocccFunctionInfoPass> X_ROCCC_FUNCTION_INFO_PASS("rocccFunctionInfo", "Sets up information relating to roccc.");  

/*
This function finds all of the input streams, and pulls information about
that stream into a more readable format.
The ROCCCInputFifos and smart buffers will be of the form -
ROCCCInputFifo(stream, induction_variable0, ..., induction_variableN,
               element0, element0_index0, ..., element0_indexN,
               ...,
               elementM, elementM_index0, ..., elementM_indexN);
For each stream, we are interested in the following information:
  1) dimensions of the stream, N
  2) stream value itself
  3) N loop induction variables used to index into the stream
  4) M read elements from the stream, with each element containing
     A) a value read
     B) N integer offsets added to the induction variables and used as indexes into the stream
  5) The number of invalidated buffer elements, which is the step size along the innermost loop's induction_variable dimension

This takes a ROCCCLoopInformation struct because it is adding to the
info we already collected.   
*/
bool setLoopControllerBufferInfo(DFFunction* F, ROCCCLoopInformation& inf)
{
    bool MadeChange = false;
    //find the input/output buffers, and save their info
    for ( Function::iterator BB = F->begin(); BB != F->end(); ++BB )
    {
      for ( BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II )
      {
        if ( CallInst* CI = dynamic_cast<CallInst*>(&*II) )
        {
          if ( isROCCCInputStream(CI) )
          {
            unsigned int dim = getROCCCStreamDimension(CI);
            Value* buffer = CI->getOperand(1);
            inf.inputBuffers.push_back( buffer );
            for (unsigned int i = 2; i < dim+2; ++i)
            {
              Value* v = CI->getOperand(i);
              inf.inputBufferLoopIndexes[buffer].push_back( v );
              if( inf.loopChildren[v].size() == 0 )
              {
                INTERNAL_MESSAGE("Setting invalidated of input stream " << buffer->getName() << " to step size of " << v->getName() << ", " << getIVStepSize(v) << "\n");
                inf.inputBufferNumInvalidated[buffer] = getIVStepSize(v);
              }
            }
            for (unsigned int i = dim+2; i + dim < CI->getNumOperands(); i+=dim+1)
            {
              Value* v = CI->getOperand(i);
              vector<int> hold;
              for(unsigned int d = 1; d <= dim; ++d)
              {
                ConstantInt* conInt = dynamic_cast<ConstantInt*>((CI->getOperand(i+d)));
                assert(conInt);
                int index = conInt->getSExtValue();
                hold.push_back( index );
              }
              inf.inputBufferIndexes[buffer].push_back( pair<Value*,vector<int> >(v, hold) );
            }
          }
          if ( isROCCCOutputStream(CI) )
          {
            unsigned int dim = getROCCCStreamDimension(CI);
            Value* buffer = CI->getOperand(1);
            inf.outputBuffers.push_back( buffer );
            for (unsigned int i = 2; i < dim+2; ++i)
            {
              Value* v = CI->getOperand(i);
              inf.outputBufferLoopIndexes[buffer].push_back( v );
              if( inf.loopChildren[v].size() == 0 )
              {
                INTERNAL_MESSAGE("Setting invalidated of output stream " << buffer->getName() << " to step size of " << v->getName() << ", " << getIVStepSize(v) << "\n");
                inf.outputBufferNumInvalidated[buffer] = getIVStepSize(v);
              }
            }
            for (unsigned int i = dim+2; i + dim < CI->getNumOperands(); i+=dim+1)
            {
              Value* v = CI->getOperand(i);
              vector<int> hold;
              for(unsigned int d = 1; d <= dim; ++d)
              {
                ConstantInt* conInt = dynamic_cast<ConstantInt*>((CI->getOperand(i+d)));
                assert(conInt);
                int index = conInt->getSExtValue();
                hold.push_back( index );
              }
              inf.outputBufferIndexes[buffer].push_back( std::make_pair(v, hold) );
            }
            //inf.outputBufferNumInvalidated[buffer] = getStepSize(inf./*inputBufferLoopIndexes[buffer]*/indexes[0]);
          }
        }
      }
    }
  return MadeChange;
}

/*
The llvm-generated CFG has a backedge to the loop header, we need
to remove it so there are no loops in the DFG. All of the information
that the backedge represents should already be saved, such as the 
induction variables, the endValues, and the step size.
*/  
bool removeLoopControllerBackEdge(DFFunction* F, ROCCCLoopInformation& inf)
{
  bool changed = false;
  //if there are no induction variables, then we dont have any loops
  for( std::vector<Value*>::iterator II = inf.indexes.begin(); II != inf.indexes.end(); ++II )
  {
    PHINode* v = dynamic_cast<PHINode*>( *II );
    assert( v );
    DFBasicBlock* p = v->getParent()->getDFBasicBlock();
    assert( p );
    //If the loop header is both the predecessor and successor of the same block,
    //then we need to remove that backedge
    for ( pred_iterator BB = pred_begin(p); BB != pred_end(p); ++BB )
    {
      for( pred_iterator BB2 = pred_begin(*BB); BB2 != pred_end(*BB); ++BB2 )
      {
        if ( p == *BB2 )
        {
          //remove the connection
          DFBasicBlock* new_bb = DFBasicBlock_Create( "LoopRemoveGen", F );
          (*BB)->getTerminator()->replaceUsesOfWith( p, new_bb );
          if( v->getBasicBlockIndex(*BB) != -1 )
          {
            v->removeIncomingValue(*BB, false);
          }
          changed = true;
        }
      }
    }
  }
  return changed;
}
	
struct RocccCFGtoDFGPass : public FunctionPass 
{
  static char ID;
  RocccCFGtoDFGPass() : FunctionPass((intptr_t)&ID) {}
  virtual void getAnalysisUsage(AnalysisUsage &AU) const
  {
    AU.addRequired<RocccIfCompactPass>();
    AU.addRequired<RocccFunctionInfoPass>();
    AU.addPreserved<RocccIfCompactPass>();
    AU.addPreserved<RocccFunctionInfoPass>();
  }
  virtual bool runOnFunction(Function& F)
  {
    CurrentFile::set(__FILE__);
    return unsafeRunOnFunction( F );
  }
  bool unsafeRunOnFunction(Function& F)
  {
    CurrentFile::set(__FILE__);
    
    // Before doing anything, check to see if this is already a DFFunction. 
    //  If so, we have already processed it.
    if ( dynamic_cast<DFFunction*>( &F ) )
    {
       //check to see if we need to remove the old function
       if ( F.getName().substr(0,3) == "DF_" ) //we need to remove
       {
         std::string name = F.getName().substr(3);
         if ( F.getParent()->getFunction(name) != NULL )
         {
           F.getParent()->getFunction(name)->replaceAllUsesWith(&F);
           //remove everything in the old function F
           Function::iterator BB = F.getParent()->getFunction(name)->begin(); 
           while(BB != F.getParent()->getFunction(name)->end())
           {
             assert(BB->begin() == BB->end() or &*BB->begin() == BB->getTerminator());
             if( BB->use_begin() != BB->use_end() )
             {
               INTERNAL_ERROR("Attempting to delete temporary function, but BasicBlock \'" << BB->getName() << "\' is still being used!\n");
               for(Value::use_iterator UI = BB->use_begin(); UI != BB->use_end(); ++UI)
               {
                 INTERNAL_MESSAGE("use - " << **UI);
               }
               assert(0 and "BB in original function still being used!");
             }
             BB->eraseFromParent();
             BB = F.getParent()->getFunction(name)->begin(); 
           }
           F.getParent()->getFunction(name)->eraseFromParent();
           F.setName( name );
           return true;
         }
      }      
      return false; //we made no changes.
    }

    //dont turn it into a DFFunction if its a system, only if its a
    //block or module
    if( ROCCCFunctionType(&F) == SYSTEM )
    {
      return false;
    }

    ROCCCLoopInformation inf = getAnalysis<RocccFunctionInfoPass>().getInfo();
    RocccFunctionInfoPass::IfMap imap = getAnalysis<RocccFunctionInfoPass>().getIfMap();

    //MadeChange is returned from runOnFunction to state whether the function was changed at all
    bool MadeChange = false; 

    //create a DFFunction that will be a copy of the currently processed Function, F
    DFFunction* new_df_func = DFFunction_Create( &F ) ;
    //set the function type of the new DFFunc

    new_df_func->setFunctionType( ROCCCFunctionType(&F) );
    

    //for each instruction in the old function, except terminator instructions:
    //  -create a new DFBasicBlock to hold that instruction
    //  -move that instruction into the new DFBasicBlock
    //  -move the DFBasicBlock into the newly created DFFunction
    for (Function::iterator BB = F.begin(); BB != F.end(); ++BB)
    {
      pullBBIntoDFFunction( &*BB, new_df_func );
      MadeChange = true;
    }
    //merge all of the indexes, because they will be output by one component
    if(inf.indexes.begin() != inf.indexes.end())
    {
      llvm::Instruction* indexBegin = dynamic_cast<llvm::Instruction*>(*inf.indexes.begin());
      for(std::vector<llvm::Value*>::iterator IXI = inf.indexes.begin(); IXI != inf.indexes.end(); ++IXI)
      {
        llvm::Instruction* inst = dynamic_cast<llvm::Instruction*>(*IXI);
        assert(inst and "Loop index is not an instruction!");
        llvm::PHINode* phi = dynamic_cast<llvm::PHINode*>(inst);
        assert(phi and "Loop index is not a phi?!");
        
        if( IXI != inf.indexes.begin() )
          inst->moveBefore(indexBegin);
        while(phi->getNumIncomingValues())
        {
          phi->removeIncomingValue(0u, false);
        }
      }
      AddSource( indexBegin->getParent()->getDFBasicBlock(), new_df_func );
	  }		
    // Make the connections of each switch statement mirror the use-def chains
    //  Iterate over the newly created DFFunction, connecting each switch
    //  statement to the DFFunction that it uses
    for( Function::iterator BB = new_df_func->begin(); BB != new_df_func->end(); ++BB )
    {
      assert( BB->begin() != BB->end() && "Empty BB in new_df_func!" );
      //If a value has a definition, and isnt its own definition
      //(ie input_fifos, smart buffers, input_scalars), then we dont want to connect
      //it except to the definition. The definition itself needs to handle
      //connecting to the children
      //scan to see if we have another definition, or if we are the definition
      bool has_other_definition = false;
      for (Value::use_iterator i = BB->begin()->use_begin(), e = BB->begin()->use_end(); i != e; ++i)
      {
        Instruction* ins = dynamic_cast<Instruction*>(*i);
        CallInst* ci = dynamic_cast<CallInst*>(ins);
        if ( isStore(ins) && getStoreDestination(ins) && getStoreDestination(ins)->getParent() == &*BB )
        {
          if( has_other_definition )
            INTERNAL_ERROR( ins->getName() << " already has other definition, and " << getStoreDestination(ins) << " is definition!\n");
          assert( !has_other_definition and "Can only have one definition per value!" );
          has_other_definition = true;
        }
        else if ( ci )
        {
          for ( unsigned int op = 0; op < ci->getNumOperands(); ++op )
          {
            if ( Instruction* ai = dynamic_cast<Instruction*> (ci->getOperand(op)) )
            {
              if ( ai == BB->begin() && isDefinition(ci, ci->getOperand(op)) )
              {
                if( has_other_definition )
                  INTERNAL_ERROR( ai->getName() << " already has other definition, and " << *ci << " is definition!\n");
                assert( !has_other_definition and "Can only have one definition per value!" );
                has_other_definition = true;
              }
            }
          }
        }
      }        
      //now that we have whether or not we are the definition, go ahead
      //and scan for the definition (if we arent already the definition),
      //and attach us. If we are actually the definition, then connect us as
      //normal 
      for (Value::use_iterator i = BB->begin()->use_begin(), e = BB->begin()->use_end(); i != e; ++i)
      {
        Instruction* ins = dynamic_cast<Instruction*>(*i);
        CallInst* ci = dynamic_cast<CallInst*>(ins);
        if ( isStore(ins) && getStoreDestination(ins) && getStoreDestination(ins)->getParent() == &*BB )
        {
          AddAllUsesOfAsUse( ins->getParent(), &*BB );
        }
        else if ( ci )
        {
          //for each operand, we need to make this function the def of the use-def chain that includes that operand
          for ( unsigned int op = 0; op < ci->getNumOperands(); ++op )
          {
            if ( Instruction* ai = dynamic_cast<Instruction*> (ci->getOperand(op)) )
            {
              if ( ai == BB->begin() && isDefinition(ci, ci->getOperand(op)) )
              {
                AddAllUsesOfAsUse( ci->getParent(), ai->getParent() );
                if( isROCCCInput(ci) and isROCCCInputStream(ci) )
                    AddUse( ai->getParent(), ci->getParent() );
              }
              else if ( ai == BB->begin() )
                if( !has_other_definition )
                  AddUse( ai->getParent(), ci->getParent() );
            }
          }
        }
        else if (ins) //its nothing special, so just connect it normally
        {
          //dont connect it if there is a use that is also a definition; we should
          // let that definition automatically connect us to the correct place
          if( !has_other_definition )
            AddUse( &*BB, ins->getParent() );
        }
      }
    }
    
    /*
    //we need to make sure that the phi node entries reflect the new connections of the DFBBs
    for( Function::iterator BB = new_df_func->begin(); BB != new_df_func->end(); ++BB )
    {
      assert( BB->begin() != BB->end() && "Empty BB in new_df_func!" );      
      if ( PHINode* pn = dynamic_cast<PHINode*> (&*BB->begin()) )
      {
        //create a new phinode that we will replace the current phinode with
        PHINode* new_pn = PHINode::Create( pn->getType(), pn->getName() );
        //go through the list of values the phinode could be, and for each value that is an instruction,
        //add that value to the phinode 
        for ( unsigned int op = 0; op < pn->getNumIncomingValues(); ++op )
        {
          Value* v = pn->getIncomingValue(op);
          //if its not already an instruction, we need to make it an instruction
          if ( !dynamic_cast<Instruction*>(v) )
          {
            DFBasicBlock* constBB = DFBasicBlock_Create( "constBB", new_df_func );
            pn->setIncomingBlock( op, constBB );
            AddUse( constBB, pn->getParent() );
          }
          if ( Instruction* op_in = dynamic_cast<Instruction*>(pn->getIncomingValue(op)) )
            new_pn->addIncoming( op_in, op_in->getParent() );
        }
        //now look to see if there are any predecessors of the phi node's basic block that werent in the original phi's incoming list
        for (pred_iterator PI = pred_begin(BB); PI != pred_end(BB); ++PI)
        {
          DFBasicBlock* pred = dynamic_cast<DFBasicBlock*>(*PI);
          bool exists_in_incoming_list = false;
          for ( unsigned int op = 0; op < pn->getNumIncomingValues(); ++op )
          {
            if ( Instruction* op_in = dynamic_cast<Instruction*> (pn->getIncomingValue(op)) )
            {
              if ( pred == op_in->getParent() )
                exists_in_incoming_list = true;
            }
          }
          if ( !exists_in_incoming_list )
          {
             new_pn->addIncoming( llvm::ConstantInt::get(new_pn->getType(),0), pred ); //just add a useless temporary
          }
        }
        //replace the old phi with the newly created phi
        ReplaceInstWithInst(&*(BB->begin()), new_pn);
        for(std::map<Value*,Value*>::iterator MI = inf.endValues.begin(); MI != inf.endValues.end(); ++MI)
        {
          if(MI->second == pn )
            inf.endValues[MI->first] = new_pn;
        }
        for(std::map<Value*,Value*>::iterator MI = inf.endValues.begin(); MI != inf.endValues.end(); ++MI)
        {
          if( MI->first == pn )
          {
            inf.endValues[new_pn] = MI->second;
            inf.endValues.erase( MI );
            break;
          }
        }
        //since we are replacing the phi nodes, we also need to update the loopChildren
        //map, since the loop induction variables are phi nodes
        for(std::map<Value*,std::vector<Value*> >::iterator MI = inf.loopChildren.begin(); MI != inf.loopChildren.end(); ++MI)
        {
           replace(MI->second.begin(), MI->second.end(), pn, new_pn);
        }
        for(std::map<Value*,std::vector<Value*> >::iterator MI = inf.loopChildren.begin(); MI != inf.loopChildren.end(); ++MI)
        {
          if( MI->first == pn )
          {
            inf.loopChildren[new_pn] = MI->second;
            inf.loopChildren.erase( MI );
            break;
          }
        }
        //also update the list of loop induction variables itself
        for(std::map<Value*,std::vector<Value*> >::iterator MI = inf.inputBufferLoopIndexes.begin(); MI != inf.inputBufferLoopIndexes.end(); ++MI)
        {
          replace(MI->second.begin(), MI->second.end(), pn, new_pn);
        }
        replace( inf.indexes.begin(), inf.indexes.end(), pn, new_pn );
      }
    }
    */
    //remove any back edges in the loop header
    MadeChange = removeLoopControllerBackEdge( new_df_func, inf ) or MadeChange;
    //get all the loop buffer info
    MadeChange = setLoopControllerBufferInfo( new_df_func, inf ) or MadeChange;
    //finally, set the LoopInformation in the new DFFunc
    new_df_func->setROCCCLoopInfo( inf );

    //as a basic sanity check, make sure we have at least 1 input and output
    bool has_output = false;
    assert(new_df_func);
    assert(new_df_func->getSink());
    for(pred_iterator pred = pred_begin(new_df_func->getSink()); pred != pred_end(new_df_func->getSink()); ++pred)
    {
      for(BasicBlock::iterator II = (*pred)->begin(); II != (*pred)->end(); ++II)
      {
        if( isROCCCOutput(&*II) )
          has_output = true;
      }
    }
    assert( has_output and "Must have at least 1 output!\n" );
    bool has_input = false;
    for(succ_iterator succ = succ_begin(new_df_func->getSource()); succ != succ_end(new_df_func->getSource()); ++succ)
    {
      for(BasicBlock::iterator II = (*succ)->begin(); II != (*succ)->end(); ++II)
      {
        if( isROCCCInput(&*II) )
          has_input = true;
      }
    }
    assert( has_input and "Must have at least 1 input!\n" );
    return MadeChange;
  }
};
	
char RocccCFGtoDFGPass::ID = 0;
RegisterPass<RocccCFGtoDFGPass> X_ROCCC_CFG_TO_DFG_PASS("rocccCFGtoDFG", "Converts the cfg to a dfg.");  


//This pass renames loads to be derivitives of the loaded value's name
struct RenameMemoryVariablesPass : public FunctionPass 
{
  static char ID;
  RenameMemoryVariablesPass() : FunctionPass((intptr_t)&ID) {}
  virtual bool runOnFunction(Function& F)
  {
    //set currentfile, so errors appear correctly
    CurrentFile::set(__FILE__);
    bool changed = false;
    for(Function::iterator BB = F.begin(); BB != F.end(); ++BB)
    {
      for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
      {
        //if its a load instruction, rename it
        if( LoadInst* LI = dynamic_cast<LoadInst*>(&*II) )
        {
          //only rename it if an actual name exists
          if( LI->getOperand(0)->getName() != "" )
            LI->setName(LI->getOperand(0)->getName());
          //we have changed the internal representation
          changed = true;
        }
      }
    }
    return changed;
  }
};
char RenameMemoryVariablesPass::ID = 0;
RegisterPass<RenameMemoryVariablesPass> X_RENAME_MEMORY_VARIABLES_PASS("renameMem", "Renames stores and load variables.");  
	
}

