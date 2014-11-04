#include "common/system_specific.h"
#include "basic.h"
#include "suif.h"
#include "suif_type_utils.h"
bool is_kind_of_SizedType(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(SizedType::get_class_name());
  #else
  return (dynamic_cast<SizedType*>(x) != 0);
  #endif
  }
SizedType* to_SizedType(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_SizedType(x))
    return (SizedType*) x;
  else
    return 0;
  #else
  return dynamic_cast<SizedType*>(x);
  #endif
  }
bool is_kind_of_BooleanType(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(BooleanType::get_class_name());
  #else
  return (dynamic_cast<BooleanType*>(x) != 0);
  #endif
  }
BooleanType* to_BooleanType(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_BooleanType(x))
    return (BooleanType*) x;
  else
    return 0;
  #else
  return dynamic_cast<BooleanType*>(x);
  #endif
  }
bool is_kind_of_NumericType(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(NumericType::get_class_name());
  #else
  return (dynamic_cast<NumericType*>(x) != 0);
  #endif
  }
NumericType* to_NumericType(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_NumericType(x))
    return (NumericType*) x;
  else
    return 0;
  #else
  return dynamic_cast<NumericType*>(x);
  #endif
  }
bool is_kind_of_IntegerType(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(IntegerType::get_class_name());
  #else
  return (dynamic_cast<IntegerType*>(x) != 0);
  #endif
  }
IntegerType* to_IntegerType(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_IntegerType(x))
    return (IntegerType*) x;
  else
    return 0;
  #else
  return dynamic_cast<IntegerType*>(x);
  #endif
  }
bool is_kind_of_FloatingPointType(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(FloatingPointType::get_class_name());
  #else
  return (dynamic_cast<FloatingPointType*>(x) != 0);
  #endif
  }
FloatingPointType* to_FloatingPointType(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_FloatingPointType(x))
    return (FloatingPointType*) x;
  else
    return 0;
  #else
  return dynamic_cast<FloatingPointType*>(x);
  #endif
  }
bool is_kind_of_EnumeratedType(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(EnumeratedType::get_class_name());
  #else
  return (dynamic_cast<EnumeratedType*>(x) != 0);
  #endif
  }
EnumeratedType* to_EnumeratedType(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_EnumeratedType(x))
    return (EnumeratedType*) x;
  else
    return 0;
  #else
  return dynamic_cast<EnumeratedType*>(x);
  #endif
  }
bool is_kind_of_PointerType(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(PointerType::get_class_name());
  #else
  return (dynamic_cast<PointerType*>(x) != 0);
  #endif
  }
PointerType* to_PointerType(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_PointerType(x))
    return (PointerType*) x;
  else
    return 0;
  #else
  return dynamic_cast<PointerType*>(x);
  #endif
  }
bool is_kind_of_ArrayType(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(ArrayType::get_class_name());
  #else
  return (dynamic_cast<ArrayType*>(x) != 0);
  #endif
  }
ArrayType* to_ArrayType(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_ArrayType(x))
    return (ArrayType*) x;
  else
    return 0;
  #else
  return dynamic_cast<ArrayType*>(x);
  #endif
  }
bool is_kind_of_GroupType(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(GroupType::get_class_name());
  #else
  return (dynamic_cast<GroupType*>(x) != 0);
  #endif
  }
GroupType* to_GroupType(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_GroupType(x))
    return (GroupType*) x;
  else
    return 0;
  #else
  return dynamic_cast<GroupType*>(x);
  #endif
  }
bool is_kind_of_BasicProcedureType(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(BasicProcedureType::get_class_name());
  #else
  return (dynamic_cast<BasicProcedureType*>(x) != 0);
  #endif
  }
BasicProcedureType* to_BasicProcedureType(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_BasicProcedureType(x))
    return (BasicProcedureType*) x;
  else
    return 0;
  #else
  return dynamic_cast<BasicProcedureType*>(x);
  #endif
  }
bool is_kind_of_FieldSymbol(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(FieldSymbol::get_class_name());
  #else
  return (dynamic_cast<FieldSymbol*>(x) != 0);
  #endif
  }
FieldSymbol* to_FieldSymbol(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_FieldSymbol(x))
    return (FieldSymbol*) x;
  else
    return 0;
  #else
  return dynamic_cast<FieldSymbol*>(x);
  #endif
  }
bool is_kind_of_NestingVariableSymbol(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(NestingVariableSymbol::get_class_name());
  #else
  return (dynamic_cast<NestingVariableSymbol*>(x) != 0);
  #endif
  }
NestingVariableSymbol* to_NestingVariableSymbol(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_NestingVariableSymbol(x))
    return (NestingVariableSymbol*) x;
  else
    return 0;
  #else
  return dynamic_cast<NestingVariableSymbol*>(x);
  #endif
  }
bool is_kind_of_EvalStatement(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(EvalStatement::get_class_name());
  #else
  return (dynamic_cast<EvalStatement*>(x) != 0);
  #endif
  }
EvalStatement* to_EvalStatement(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_EvalStatement(x))
    return (EvalStatement*) x;
  else
    return 0;
  #else
  return dynamic_cast<EvalStatement*>(x);
  #endif
  }
bool is_kind_of_IfStatement(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(IfStatement::get_class_name());
  #else
  return (dynamic_cast<IfStatement*>(x) != 0);
  #endif
  }
IfStatement* to_IfStatement(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_IfStatement(x))
    return (IfStatement*) x;
  else
    return 0;
  #else
  return dynamic_cast<IfStatement*>(x);
  #endif
  }
bool is_kind_of_WhileStatement(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(WhileStatement::get_class_name());
  #else
  return (dynamic_cast<WhileStatement*>(x) != 0);
  #endif
  }
WhileStatement* to_WhileStatement(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_WhileStatement(x))
    return (WhileStatement*) x;
  else
    return 0;
  #else
  return dynamic_cast<WhileStatement*>(x);
  #endif
  }
bool is_kind_of_DoWhileStatement(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(DoWhileStatement::get_class_name());
  #else
  return (dynamic_cast<DoWhileStatement*>(x) != 0);
  #endif
  }
DoWhileStatement* to_DoWhileStatement(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_DoWhileStatement(x))
    return (DoWhileStatement*) x;
  else
    return 0;
  #else
  return dynamic_cast<DoWhileStatement*>(x);
  #endif
  }
bool is_kind_of_ForStatement(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(ForStatement::get_class_name());
  #else
  return (dynamic_cast<ForStatement*>(x) != 0);
  #endif
  }
ForStatement* to_ForStatement(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_ForStatement(x))
    return (ForStatement*) x;
  else
    return 0;
  #else
  return dynamic_cast<ForStatement*>(x);
  #endif
  }
bool is_kind_of_ScopeStatement(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(ScopeStatement::get_class_name());
  #else
  return (dynamic_cast<ScopeStatement*>(x) != 0);
  #endif
  }
ScopeStatement* to_ScopeStatement(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_ScopeStatement(x))
    return (ScopeStatement*) x;
  else
    return 0;
  #else
  return dynamic_cast<ScopeStatement*>(x);
  #endif
  }
bool is_kind_of_VaStartStatement(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(VaStartStatement::get_class_name());
  #else
  return (dynamic_cast<VaStartStatement*>(x) != 0);
  #endif
  }
VaStartStatement* to_VaStartStatement(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_VaStartStatement(x))
    return (VaStartStatement*) x;
  else
    return 0;
  #else
  return dynamic_cast<VaStartStatement*>(x);
  #endif
  }
bool is_kind_of_VaStartOldStatement(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(VaStartOldStatement::get_class_name());
  #else
  return (dynamic_cast<VaStartOldStatement*>(x) != 0);
  #endif
  }
VaStartOldStatement* to_VaStartOldStatement(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_VaStartOldStatement(x))
    return (VaStartOldStatement*) x;
  else
    return 0;
  #else
  return dynamic_cast<VaStartOldStatement*>(x);
  #endif
  }
bool is_kind_of_VaEndStatement(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(VaEndStatement::get_class_name());
  #else
  return (dynamic_cast<VaEndStatement*>(x) != 0);
  #endif
  }
VaEndStatement* to_VaEndStatement(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_VaEndStatement(x))
    return (VaEndStatement*) x;
  else
    return 0;
  #else
  return dynamic_cast<VaEndStatement*>(x);
  #endif
  }
bool is_kind_of_StoreStatement(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(StoreStatement::get_class_name());
  #else
  return (dynamic_cast<StoreStatement*>(x) != 0);
  #endif
  }
StoreStatement* to_StoreStatement(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_StoreStatement(x))
    return (StoreStatement*) x;
  else
    return 0;
  #else
  return dynamic_cast<StoreStatement*>(x);
  #endif
  }
bool is_kind_of_ReturnStatement(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(ReturnStatement::get_class_name());
  #else
  return (dynamic_cast<ReturnStatement*>(x) != 0);
  #endif
  }
ReturnStatement* to_ReturnStatement(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_ReturnStatement(x))
    return (ReturnStatement*) x;
  else
    return 0;
  #else
  return dynamic_cast<ReturnStatement*>(x);
  #endif
  }
bool is_kind_of_JumpStatement(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(JumpStatement::get_class_name());
  #else
  return (dynamic_cast<JumpStatement*>(x) != 0);
  #endif
  }
JumpStatement* to_JumpStatement(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_JumpStatement(x))
    return (JumpStatement*) x;
  else
    return 0;
  #else
  return dynamic_cast<JumpStatement*>(x);
  #endif
  }
bool is_kind_of_JumpIndirectStatement(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(JumpIndirectStatement::get_class_name());
  #else
  return (dynamic_cast<JumpIndirectStatement*>(x) != 0);
  #endif
  }
JumpIndirectStatement* to_JumpIndirectStatement(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_JumpIndirectStatement(x))
    return (JumpIndirectStatement*) x;
  else
    return 0;
  #else
  return dynamic_cast<JumpIndirectStatement*>(x);
  #endif
  }
bool is_kind_of_BranchStatement(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(BranchStatement::get_class_name());
  #else
  return (dynamic_cast<BranchStatement*>(x) != 0);
  #endif
  }
BranchStatement* to_BranchStatement(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_BranchStatement(x))
    return (BranchStatement*) x;
  else
    return 0;
  #else
  return dynamic_cast<BranchStatement*>(x);
  #endif
  }
bool is_kind_of_MultiWayBranchStatement(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(MultiWayBranchStatement::get_class_name());
  #else
  return (dynamic_cast<MultiWayBranchStatement*>(x) != 0);
  #endif
  }
MultiWayBranchStatement* to_MultiWayBranchStatement(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_MultiWayBranchStatement(x))
    return (MultiWayBranchStatement*) x;
  else
    return 0;
  #else
  return dynamic_cast<MultiWayBranchStatement*>(x);
  #endif
  }
bool is_kind_of_LabelLocationStatement(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(LabelLocationStatement::get_class_name());
  #else
  return (dynamic_cast<LabelLocationStatement*>(x) != 0);
  #endif
  }
LabelLocationStatement* to_LabelLocationStatement(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_LabelLocationStatement(x))
    return (LabelLocationStatement*) x;
  else
    return 0;
  #else
  return dynamic_cast<LabelLocationStatement*>(x);
  #endif
  }
bool is_kind_of_AssertStatement(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(AssertStatement::get_class_name());
  #else
  return (dynamic_cast<AssertStatement*>(x) != 0);
  #endif
  }
AssertStatement* to_AssertStatement(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_AssertStatement(x))
    return (AssertStatement*) x;
  else
    return 0;
  #else
  return dynamic_cast<AssertStatement*>(x);
  #endif
  }
bool is_kind_of_MarkStatement(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(MarkStatement::get_class_name());
  #else
  return (dynamic_cast<MarkStatement*>(x) != 0);
  #endif
  }
MarkStatement* to_MarkStatement(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_MarkStatement(x))
    return (MarkStatement*) x;
  else
    return 0;
  #else
  return dynamic_cast<MarkStatement*>(x);
  #endif
  }
bool is_kind_of_BinaryArithmeticExpression(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(BinaryArithmeticExpression::get_class_name());
  #else
  return (dynamic_cast<BinaryArithmeticExpression*>(x) != 0);
  #endif
  }
BinaryArithmeticExpression* to_BinaryArithmeticExpression(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_BinaryArithmeticExpression(x))
    return (BinaryArithmeticExpression*) x;
  else
    return 0;
  #else
  return dynamic_cast<BinaryArithmeticExpression*>(x);
  #endif
  }
bool is_kind_of_UnaryArithmeticExpression(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(UnaryArithmeticExpression::get_class_name());
  #else
  return (dynamic_cast<UnaryArithmeticExpression*>(x) != 0);
  #endif
  }
UnaryArithmeticExpression* to_UnaryArithmeticExpression(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_UnaryArithmeticExpression(x))
    return (UnaryArithmeticExpression*) x;
  else
    return 0;
  #else
  return dynamic_cast<UnaryArithmeticExpression*>(x);
  #endif
  }
bool is_kind_of_CopyExpression(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(CopyExpression::get_class_name());
  #else
  return (dynamic_cast<CopyExpression*>(x) != 0);
  #endif
  }
CopyExpression* to_CopyExpression(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_CopyExpression(x))
    return (CopyExpression*) x;
  else
    return 0;
  #else
  return dynamic_cast<CopyExpression*>(x);
  #endif
  }
bool is_kind_of_SelectExpression(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(SelectExpression::get_class_name());
  #else
  return (dynamic_cast<SelectExpression*>(x) != 0);
  #endif
  }
SelectExpression* to_SelectExpression(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_SelectExpression(x))
    return (SelectExpression*) x;
  else
    return 0;
  #else
  return dynamic_cast<SelectExpression*>(x);
  #endif
  }
bool is_kind_of_ArrayReferenceExpression(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(ArrayReferenceExpression::get_class_name());
  #else
  return (dynamic_cast<ArrayReferenceExpression*>(x) != 0);
  #endif
  }
ArrayReferenceExpression* to_ArrayReferenceExpression(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_ArrayReferenceExpression(x))
    return (ArrayReferenceExpression*) x;
  else
    return 0;
  #else
  return dynamic_cast<ArrayReferenceExpression*>(x);
  #endif
  }
bool is_kind_of_FieldAccessExpression(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(FieldAccessExpression::get_class_name());
  #else
  return (dynamic_cast<FieldAccessExpression*>(x) != 0);
  #endif
  }
FieldAccessExpression* to_FieldAccessExpression(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_FieldAccessExpression(x))
    return (FieldAccessExpression*) x;
  else
    return 0;
  #else
  return dynamic_cast<FieldAccessExpression*>(x);
  #endif
  }
bool is_kind_of_BitSizeOfExpression(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(BitSizeOfExpression::get_class_name());
  #else
  return (dynamic_cast<BitSizeOfExpression*>(x) != 0);
  #endif
  }
BitSizeOfExpression* to_BitSizeOfExpression(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_BitSizeOfExpression(x))
    return (BitSizeOfExpression*) x;
  else
    return 0;
  #else
  return dynamic_cast<BitSizeOfExpression*>(x);
  #endif
  }
bool is_kind_of_ByteSizeOfExpression(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(ByteSizeOfExpression::get_class_name());
  #else
  return (dynamic_cast<ByteSizeOfExpression*>(x) != 0);
  #endif
  }
ByteSizeOfExpression* to_ByteSizeOfExpression(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_ByteSizeOfExpression(x))
    return (ByteSizeOfExpression*) x;
  else
    return 0;
  #else
  return dynamic_cast<ByteSizeOfExpression*>(x);
  #endif
  }
bool is_kind_of_BitAlignmentOfExpression(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(BitAlignmentOfExpression::get_class_name());
  #else
  return (dynamic_cast<BitAlignmentOfExpression*>(x) != 0);
  #endif
  }
BitAlignmentOfExpression* to_BitAlignmentOfExpression(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_BitAlignmentOfExpression(x))
    return (BitAlignmentOfExpression*) x;
  else
    return 0;
  #else
  return dynamic_cast<BitAlignmentOfExpression*>(x);
  #endif
  }
bool is_kind_of_ByteAlignmentOfExpression(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(ByteAlignmentOfExpression::get_class_name());
  #else
  return (dynamic_cast<ByteAlignmentOfExpression*>(x) != 0);
  #endif
  }
ByteAlignmentOfExpression* to_ByteAlignmentOfExpression(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_ByteAlignmentOfExpression(x))
    return (ByteAlignmentOfExpression*) x;
  else
    return 0;
  #else
  return dynamic_cast<ByteAlignmentOfExpression*>(x);
  #endif
  }
bool is_kind_of_BitOffsetOfExpression(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(BitOffsetOfExpression::get_class_name());
  #else
  return (dynamic_cast<BitOffsetOfExpression*>(x) != 0);
  #endif
  }
BitOffsetOfExpression* to_BitOffsetOfExpression(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_BitOffsetOfExpression(x))
    return (BitOffsetOfExpression*) x;
  else
    return 0;
  #else
  return dynamic_cast<BitOffsetOfExpression*>(x);
  #endif
  }
bool is_kind_of_ByteOffsetOfExpression(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(ByteOffsetOfExpression::get_class_name());
  #else
  return (dynamic_cast<ByteOffsetOfExpression*>(x) != 0);
  #endif
  }
ByteOffsetOfExpression* to_ByteOffsetOfExpression(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_ByteOffsetOfExpression(x))
    return (ByteOffsetOfExpression*) x;
  else
    return 0;
  #else
  return dynamic_cast<ByteOffsetOfExpression*>(x);
  #endif
  }
bool is_kind_of_ScAndExpression(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(ScAndExpression::get_class_name());
  #else
  return (dynamic_cast<ScAndExpression*>(x) != 0);
  #endif
  }
ScAndExpression* to_ScAndExpression(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_ScAndExpression(x))
    return (ScAndExpression*) x;
  else
    return 0;
  #else
  return dynamic_cast<ScAndExpression*>(x);
  #endif
  }
bool is_kind_of_ScOrExpression(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(ScOrExpression::get_class_name());
  #else
  return (dynamic_cast<ScOrExpression*>(x) != 0);
  #endif
  }
ScOrExpression* to_ScOrExpression(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_ScOrExpression(x))
    return (ScOrExpression*) x;
  else
    return 0;
  #else
  return dynamic_cast<ScOrExpression*>(x);
  #endif
  }
bool is_kind_of_ScSelectExpression(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(ScSelectExpression::get_class_name());
  #else
  return (dynamic_cast<ScSelectExpression*>(x) != 0);
  #endif
  }
ScSelectExpression* to_ScSelectExpression(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_ScSelectExpression(x))
    return (ScSelectExpression*) x;
  else
    return 0;
  #else
  return dynamic_cast<ScSelectExpression*>(x);
  #endif
  }
bool is_kind_of_LoadExpression(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(LoadExpression::get_class_name());
  #else
  return (dynamic_cast<LoadExpression*>(x) != 0);
  #endif
  }
LoadExpression* to_LoadExpression(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_LoadExpression(x))
    return (LoadExpression*) x;
  else
    return 0;
  #else
  return dynamic_cast<LoadExpression*>(x);
  #endif
  }
bool is_kind_of_LoadAddressExpression(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(LoadAddressExpression::get_class_name());
  #else
  return (dynamic_cast<LoadAddressExpression*>(x) != 0);
  #endif
  }
LoadAddressExpression* to_LoadAddressExpression(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_LoadAddressExpression(x))
    return (LoadAddressExpression*) x;
  else
    return 0;
  #else
  return dynamic_cast<LoadAddressExpression*>(x);
  #endif
  }
bool is_kind_of_LoadConstantExpression(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(LoadConstantExpression::get_class_name());
  #else
  return (dynamic_cast<LoadConstantExpression*>(x) != 0);
  #endif
  }
LoadConstantExpression* to_LoadConstantExpression(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_LoadConstantExpression(x))
    return (LoadConstantExpression*) x;
  else
    return 0;
  #else
  return dynamic_cast<LoadConstantExpression*>(x);
  #endif
  }
bool is_kind_of_LoadValueBlockExpression(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(LoadValueBlockExpression::get_class_name());
  #else
  return (dynamic_cast<LoadValueBlockExpression*>(x) != 0);
  #endif
  }
LoadValueBlockExpression* to_LoadValueBlockExpression(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_LoadValueBlockExpression(x))
    return (LoadValueBlockExpression*) x;
  else
    return 0;
  #else
  return dynamic_cast<LoadValueBlockExpression*>(x);
  #endif
  }
bool is_kind_of_CallExpression(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(CallExpression::get_class_name());
  #else
  return (dynamic_cast<CallExpression*>(x) != 0);
  #endif
  }
CallExpression* to_CallExpression(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_CallExpression(x))
    return (CallExpression*) x;
  else
    return 0;
  #else
  return dynamic_cast<CallExpression*>(x);
  #endif
  }
bool is_kind_of_SsaPhiExpression(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(SsaPhiExpression::get_class_name());
  #else
  return (dynamic_cast<SsaPhiExpression*>(x) != 0);
  #endif
  }
SsaPhiExpression* to_SsaPhiExpression(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_SsaPhiExpression(x))
    return (SsaPhiExpression*) x;
  else
    return 0;
  #else
  return dynamic_cast<SsaPhiExpression*>(x);
  #endif
  }
bool is_kind_of_ConstantValueBlock(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(ConstantValueBlock::get_class_name());
  #else
  return (dynamic_cast<ConstantValueBlock*>(x) != 0);
  #endif
  }
ConstantValueBlock* to_ConstantValueBlock(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_ConstantValueBlock(x))
    return (ConstantValueBlock*) x;
  else
    return 0;
  #else
  return dynamic_cast<ConstantValueBlock*>(x);
  #endif
  }
bool is_kind_of_ExpressionValueBlock(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(ExpressionValueBlock::get_class_name());
  #else
  return (dynamic_cast<ExpressionValueBlock*>(x) != 0);
  #endif
  }
ExpressionValueBlock* to_ExpressionValueBlock(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_ExpressionValueBlock(x))
    return (ExpressionValueBlock*) x;
  else
    return 0;
  #else
  return dynamic_cast<ExpressionValueBlock*>(x);
  #endif
  }
bool is_kind_of_MultiValueBlock(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(MultiValueBlock::get_class_name());
  #else
  return (dynamic_cast<MultiValueBlock*>(x) != 0);
  #endif
  }
MultiValueBlock* to_MultiValueBlock(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_MultiValueBlock(x))
    return (MultiValueBlock*) x;
  else
    return 0;
  #else
  return dynamic_cast<MultiValueBlock*>(x);
  #endif
  }
bool is_kind_of_RepeatValueBlock(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(RepeatValueBlock::get_class_name());
  #else
  return (dynamic_cast<RepeatValueBlock*>(x) != 0);
  #endif
  }
RepeatValueBlock* to_RepeatValueBlock(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_RepeatValueBlock(x))
    return (RepeatValueBlock*) x;
  else
    return 0;
  #else
  return dynamic_cast<RepeatValueBlock*>(x);
  #endif
  }
bool is_kind_of_UndefinedValueBlock(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(UndefinedValueBlock::get_class_name());
  #else
  return (dynamic_cast<UndefinedValueBlock*>(x) != 0);
  #endif
  }
UndefinedValueBlock* to_UndefinedValueBlock(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_UndefinedValueBlock(x))
    return (UndefinedValueBlock*) x;
  else
    return 0;
  #else
  return dynamic_cast<UndefinedValueBlock*>(x);
  #endif
  }
bool is_kind_of_GroupSymbolTable(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(GroupSymbolTable::get_class_name());
  #else
  return (dynamic_cast<GroupSymbolTable*>(x) != 0);
  #endif
  }
GroupSymbolTable* to_GroupSymbolTable(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_GroupSymbolTable(x))
    return (GroupSymbolTable*) x;
  else
    return 0;
  #else
  return dynamic_cast<GroupSymbolTable*>(x);
  #endif
  }
bool is_kind_of_TargetInformationBlock(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(TargetInformationBlock::get_class_name());
  #else
  return (dynamic_cast<TargetInformationBlock*>(x) != 0);
  #endif
  }
TargetInformationBlock* to_TargetInformationBlock(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_TargetInformationBlock(x))
    return (TargetInformationBlock*) x;
  else
    return 0;
  #else
  return dynamic_cast<TargetInformationBlock*>(x);
  #endif
  }
bool is_kind_of_CInformationBlock(SuifObject* x) {
  #ifndef USE_DYNAMIC_CAST
  if (x == 0)
  return false;
  return x->isKindOf(CInformationBlock::get_class_name());
  #else
  return (dynamic_cast<CInformationBlock*>(x) != 0);
  #endif
  }
CInformationBlock* to_CInformationBlock(SuifObject *x) {
  #ifndef USE_DYNAMIC_CAST
  if (is_kind_of_CInformationBlock(x))
    return (CInformationBlock*) x;
  else
    return 0;
  #else
  return dynamic_cast<CInformationBlock*>(x);
  #endif
  }
