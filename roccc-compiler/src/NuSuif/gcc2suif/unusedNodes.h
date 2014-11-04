
/*

  This file contains all of the nodes that may be used in the future, but
   currently are only declared because the gcc AST might contain them. 
   I have not run into these nodes in any of my tests yet, so I do not 
   exactly know what features these nodes need.

  When these classes actually get declared I will move them from this 
   file into one of the other files I have, eventually leading to the
   deletion of this file.

  This file should not be compiled as it does not contribute anything to the 
   actual working of the compiler as of right now.

*/

#include "baseNode.h"

class ComplexType : public Node
{
 private:
 public:
};

class VectorType : public Node
{
 private:
 public:
};

class UnionType : public Node
{
 private:
 public:
};

class CharType : public Node
{
 private:
 public:
};

class OffsetType : public Node
{
 private:
 public:
};

class FileType : public Node
{
 private:
 public:
};

class QualUnionType : public Node
{
 private:
 public:
};

class LangType : public Node
{
 private:
 public:
};

class BreakStmt : public Node
{
 private:
 public:
};

class ContinueStmt : public Node
{
 private:
 public:
};


class DoStmt : public Node
{
 private:
 public:
};

class CleanupPointStmt : public Node
{
 private:
 public:
};

class BitFieldRef : public Node
{
 private:
 public:
};


class PredecrementExpr : public Node
{
 private:
 public:
};

class PostdecrementExpr : public Node
{
 private:
 public:
};

class LoopExpr : public Node
{
 private:
 public:
};

class ExitExpr : public Node
{
 private:
 public:
};

class WithCleanupExpr : public Node
{
 private:
 public:
};

class PlaceholderExpr : public Node
{
 private:
 public:
};

class CeilDivExpr : public Node
{
 private:
 public:
};

class FloorDivExpr : public Node
{
 private:
 public:
};

class RoundDivExpr : public Node
{
 private:
 public:
};

class CeilModExpr : public Node
{
 private:
 public:
};

class FloorModExpr : public Node
{
 private:
 public:
};

class RoundModExpr : public Node
{
 private:
 public:
};

class ExactDivExpr : public Node
{
 private:
 public:
};

class FixCeilExpr : public Node
{
 private:
 public:
};

class FixFloorExpr : public Node
{
 private:
 public:
};

class FixRoundExpr : public Node
{
 private: 
 public:
};

class MinExpr : public Node
{
 private:
 public:
};

class AbsExpr : public Node
{
 private:
 public:
};

class LrotateExpr : public Node
{
 private:
 public:
};

class RrotateExpr : public Node
{
 private:
 public:
};

class TruthOrExpr : public Node
{
 private:
 public:
};

class TruthXorExpr : public Node
{
 private:
 public:
};

class UnorderedExpr : public Node
{
 private:
 public:
};

class OrderedExpr : public Node
{
 private:
 public:
};

class UnLtExpr : public Node
{
 private:
 public:
};

class UnLeExpr : public Node
{
 private:
 public:
};

class UnGeExpr : public Node
{
 private:
 public:
};

class UnEqExpr : public Node
{
 private:
 public:
};

class RangeExpr : public Node
{
 private:
 public:
};

class FdescExpr : public Node
{
 private:
 public:
};

class ComplexExpr : public Node
{
 private:
 public:
};

class ConjExpr : public Node
{
 private:
 public:
};

class RealpartExpr : public Node
{
 private:
 public:
};

class ImagpartExpr : public Node
{
 private:
 public:
};

class VaArgExpr : public Node
{
 private:
 public:
};

class TryCatchExpr : public Node
{
 private:
 public:
};

class TryFinallyExpr : public Node
{
 private:
 public:
};

class SwitchExpr : public Node
{
 private:
 public:
};

class ExcPtrExpr : public Node
{
 private:
 public:
};
