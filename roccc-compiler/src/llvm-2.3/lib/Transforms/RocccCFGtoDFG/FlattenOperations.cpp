// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

  DESC

 */

#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Constants.h"
#include "llvm/Instructions.h"

#include <list>
#include <set>
#include <algorithm>
#include <map>

#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/MessageLogger.h"
#include "rocccLibrary/ROCCCNames.h"
#include "rocccLibrary/IsValueSigned.h"
#include "rocccLibrary/SizeInBits.h"
#include "rocccLibrary/GetValueName.h"

namespace llvm
{
  class FlattenOperationsPass : public FunctionPass
  {
  private:
  public:
    static char ID ;
    FlattenOperationsPass() ;
    ~FlattenOperationsPass() ;
    virtual bool runOnFunction(Function& b) ;
  } ;
}

using namespace llvm ;

char FlattenOperationsPass::ID = 0 ;

static RegisterPass<FlattenOperationsPass> X ("flattenOperations", 
					"Optimize serial operations for parallel execution");

FlattenOperationsPass::FlattenOperationsPass() : FunctionPass((intptr_t)&ID) 
{
  ; // Nothing in here
}

FlattenOperationsPass::~FlattenOperationsPass()
{
  ; // Nothing to delete either
}

bool isCommutativeOperation(Instruction* II)
{
  return II->isCommutative();
}

bool isAssociativeOperation(Instruction* II)
{
  return II->isAssociative();
}

int getPrecedence(BinaryOperator* BO)
{
  assert(BO);
  switch(BO->getOpcode())
  {
  	case BinaryOperator::Xor:
  	case BinaryOperator::Or:
  	case BinaryOperator::And:
  	  return 1;
    case BinaryOperator::Add:
      return 2;
    case BinaryOperator::Mul:
  	  return 3;
    default:
      assert(0);
  }
}

bool precedence_less_than(BinaryOperator* a, BinaryOperator* b)
{
  assert(a);
  assert(b);
  return (getPrecedence(a) < getPrecedence(b));
}

class weight_less_than {
public:
  bool operator()(std::pair<int,Value*> a, std::pair<int,Value*> b)
  {
    if( a.first == b.first )
    {
      if( a.second == b.second )
        return true;
      return a.second < b.second;
    }
    return (a.first < b.first);
  }
};

int calculateWeight(BinaryOperator* root, std::vector<BinaryOperator*>& roots)
{
  assert(root);
  INTERNAL_MESSAGE("Calculating weight of " << root->getName() << ":\n");
  std::list<Value*> worklist;
  worklist.push_back( root->getOperand(0) );
  worklist.push_back( root->getOperand(1) );
  int weight = 0;
  while( !worklist.empty() )
  {
    Value* v = worklist.front();
    worklist.pop_front();
    assert(v);
    BinaryOperator* T = dynamic_cast<BinaryOperator*>(v);
    if( T and std::find(roots.begin(), roots.end(), T) != roots.end() )
    {
      INTERNAL_MESSAGE("  need to calculate weight of child root.\n");
      weight += calculateWeight(T, roots);
    }
    else if( T and T->getOpcode() == root->getOpcode() )
    {
      worklist.push_back( T->getOperand(0) );
      worklist.push_back( T->getOperand(1) );
    }
    else
    {
      Instruction* I = dynamic_cast<Instruction*>(v);
      if( I )
      {
        weight += 1;
      }
    }
  }
  INTERNAL_MESSAGE("  Weight is " << weight << "\n");
  return weight;
}

bool isDifferentOperation(BinaryOperator* BO, Value* UI);

/*
BalanceTree(root I)
  worklist: set
  leaves: vector
  mark I visited
  Push(worklist, Ra. Rb)
  // find all the leaves of the tree rooted at I
  while worklist not empty
    // look backwards following def-use from use
    T = ’R1 <- op1, Ra1, Rb1’ = Def(Pop(worklist))
    if T is a root
      // balance computes weight in this case
      if T not visited
         BalanceTree(T)
      SortedInsert(leaves, T, Weight(T))
    else if op(T) == op(I)
      // add uses to worklist
      Push(worklist, Ra1, Rb1)
*/
BinaryOperator* balanceTree(BinaryOperator* root, std::map<Instruction*,bool>& visitMap, std::vector<BinaryOperator*>& roots)
{
  assert(root);
  if(visitMap[root])
    return NULL;
  std::list<Value*> worklist;
  std::set<std::pair<int,Value*>,weight_less_than> leaves;
  visitMap[root] = true;
  worklist.push_back( root->getOperand(0) );
  worklist.push_back( root->getOperand(1) );
  while( !worklist.empty() )
  {
    Value* v = worklist.front();
    worklist.pop_front();
    assert(v);
    BinaryOperator* T = dynamic_cast<BinaryOperator*>(v);
    if( T and std::find(roots.begin(), roots.end(), T) != roots.end() ) // T is a binary operator that exists in the root list
    {
      if( !visitMap[T] ) //if we havent visited it, replace it with its balanced version
      {
        T = balanceTree(T, visitMap, roots);
      }
      if( !T )
      {
        INTERNAL_ERROR("balanceTree(" << *root << ") failed while attempting to balance leaf node " << *v << "; balance returned NULL!\n");
      }
      assert( T and "Balancing operation that was a root resulted in NULL being returned from balance function!" );
      leaves.insert(std::pair<int,Instruction*>(calculateWeight(T, roots), T));
    }
    else if( T and !isDifferentOperation(T, root) ) //if T isnt a root, and isnt a different operation than our root, we need to process it
    {
      worklist.push_back( T->getOperand(0) );
      worklist.push_back( T->getOperand(1) );
      //remove all of the signed, name, and size call uses
      for(Value::use_iterator UI = T->use_begin(); UI != T->use_end();)
      {
        CallInst* CI = dynamic_cast<CallInst*>(*UI);
        if( isROCCCFunctionCall(CI, ROCCCNames::VariableName) or
            isROCCCFunctionCall(CI, ROCCCNames::VariableSize) or
            isROCCCFunctionCall(CI, ROCCCNames::VariableSigned) )
        {
          CI->eraseFromParent();
          UI = T->use_begin();
        }
        else
          ++UI;
      }
    }
    else //T isnt a BinaryOperator, or isn't a root, or is a different operation than our root - just add it as a single leaf
    {
      leaves.insert(std::pair<int,Value*>(1, v));
    }
  }
  /*
  // construct a balanced tree from leaves
  while size(leaves) > 1
    Ra1 = Dequeue(leaves)
    Rb1 = Dequeue(leaves)
    T = ’R1 <- op1, Ra1, Rb1’
    insert T before I
    Weight(R1) = Weight(Ra1) + Weight(Rb1)
    SortedInsert(leaves, R1, Weight(R1))
  */
  while( leaves.size() > 1 )
  {
    std::pair<int,Value*> Ra1 = *leaves.begin();
    leaves.erase(leaves.begin());
    std::pair<int,Value*> Rb1 = *leaves.begin();
    leaves.erase(leaves.begin());
    int weight = Ra1.first + Rb1.first;
    //workaround to create a binary instruction with different operand types; create with undefs, then replace
    BinaryOperator* T = BinaryOperator::create(root->getOpcode(), UndefValue::get(root->getType()), UndefValue::get(root->getType()), "tmp", root);
    T->setOperand(0, Ra1.second);
    T->setOperand(1, Rb1.second);
    setSizeInBits(T, getSizeInBits(root));
    setValueSigned(T, isValueSigned(root));
    leaves.insert(std::pair<int,Value*>(weight, T));
  }
  BinaryOperator* last_inserted = NULL;
  if(leaves.begin() != leaves.end())
    last_inserted = dynamic_cast<BinaryOperator*>(leaves.begin()->second);
  if( last_inserted )
  {
    setValueName(last_inserted, getValueName(root));
    root->uncheckedReplaceAllUsesWith(last_inserted);
    std::string name = root->getName();
    root->eraseFromParent();
    last_inserted->setName(name);
    roots.erase(std::find(roots.begin(), roots.end(), root));
    roots.push_back(last_inserted);
    visitMap[last_inserted] = true;
  }
  return last_inserted;
}

int getRealNumUses(Instruction* II)
{
  assert(II);
  int total = 0;
  for(Value::use_iterator UI = II->use_begin(); UI != II->use_end(); ++UI)
  {
    CallInst* CI = dynamic_cast<CallInst*>(*UI);
    if( !isROCCCFunctionCall(CI, ROCCCNames::VariableName) and
        !isROCCCFunctionCall(CI, ROCCCNames::VariableSize) and
        !isROCCCFunctionCall(CI, ROCCCNames::VariableSigned) )
    {
      ++total;
    }
    if( isROCCCFunctionCall(CI, ROCCCNames::OutputScalar) or
             isROCCCOutputStream(CI) )
    {
      total += 1; //so we guarantee that we get added as a root node
    }
  }
  return total;
}

//from IsValueSigned.cpp TODO: move to header file
ConstantInt* getSignedConstantFlag(Value* v);

bool isDifferentOperation(BinaryOperator* BO, Value* UI)
{
  //we dont want to count roots unless they have REAL operations in them, so data
  //  call instructions dont count as different operations
  CallInst* CI = dynamic_cast<CallInst*>(UI);
  if( isROCCCFunctionCall(CI, ROCCCNames::VariableName) or
      isROCCCFunctionCall(CI, ROCCCNames::VariableSize) or
      isROCCCFunctionCall(CI, ROCCCNames::VariableSigned) )
  {
    return false;
  }
  //if the size or signed is different, they are different operations
  if( getSignedConstantFlag(BO) and getSignedConstantFlag(UI) and isValueSigned(BO) != isValueSigned(UI) )
    return true;
  if( getSizeInBits(BO) != getSizeInBits(UI) )
    return true;
  //if its a binary operator, and they are the same opcode, they are the same
  BinaryOperator* BU = dynamic_cast<BinaryOperator*>(UI);
  if( BU and BU->getOpcode() == BO->getOpcode()  )
  {
    return false;
  }
  //if it gets here . . . they are different
  return true;
}

/*
FindRoots()
  for each instruction I = ’R <- op, Ra, Rb’
    if op(I) not associative or commutative
       continue
    // I is a root unless R is a temporary
    //     (temporaries are only used once and by an instruction with the same operator)
    if NumUses(R) > 1 or op(Use(R)) != op(I)
       mark I as root, processed(root) = false
  order roots such that precedence of op(r$_i$) $\leq$ precedence of op(r$_{i+1}$)
  while roots not empty
    I = ’R <- op, Ra, Rb’ = Def(Pop(root))
    BalanceTree(I)
*/
bool findRoots(Function* f)
{
  bool changed = false;
  assert(f);
  std::vector<BinaryOperator*> roots;
  
  for(Function::iterator BB = f->begin(); BB != f->end(); ++BB)
  {
    for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
    {
      BinaryOperator* BO = dynamic_cast<BinaryOperator*>(&*II);
      if( BO and isCommutativeOperation(BO) and isAssociativeOperation(BO) )
      {
        if( getRealNumUses(BO) > 1 )
        {
          roots.push_back(BO);
          INTERNAL_MESSAGE("Root " << BO->getName() << " added for numUses > 1.\n");
        }
        else
        {
          for(Value::use_iterator UI = BO->use_begin(); UI != BO->use_end(); ++UI)
          {
            if( isDifferentOperation(BO, *UI)  )
            {
              roots.push_back(BO);
              INTERNAL_MESSAGE("Root " << BO->getName() << " added because it is different operation than " << (*UI)->getName() << "\n");
            } 
          }
        }
      }
    }
  }
  std::sort(roots.begin(), roots.end(), precedence_less_than);
  std::list<BinaryOperator*> root_queue;
  root_queue.resize(roots.size());
  std::copy(roots.begin(), roots.end(), root_queue.begin());
  std::map<Instruction*,bool> visitMap;
  int roots_balanced = 0;
  while( !root_queue.empty() )
  {
    BinaryOperator* BO = root_queue.front();
    root_queue.pop_front();
    bool root_changed = balanceTree(BO, visitMap, roots);
    if( root_changed )
      ++roots_balanced;
    changed = root_changed or changed;
  }
  std::stringstream ss;
  ss << "Attempted to balance " << roots.size() << " roots (";
  for(std::vector<BinaryOperator*>::iterator RI = roots.begin(); RI != roots.end(); ++RI)
  {
    if( RI != roots.begin() )
      ss << ", ";
    ss << getValueName((*RI));
  }
  ss << "), " << roots_balanced << " needed balancing.\n";
  LOG_MESSAGE1("Balancing", ss.str());
  return changed;
}

bool FlattenOperationsPass::runOnFunction(Function& f)
{
  CurrentFile::set(__FILE__);
  bool changed = false ;
  if (f.isDeclaration() || f.getDFFunction() != NULL) //only process before its a dffunction
  {
    return changed ;
  }

  changed = findRoots(&f);
  return changed ;
}

