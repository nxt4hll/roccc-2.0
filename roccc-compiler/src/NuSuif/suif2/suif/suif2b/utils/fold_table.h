#ifndef FOLD_UTILS
#define FOLD_UTILS

#include "common/i_integer.h"
#include "common/lstring.h"
#include "common/common_forwarders.h"
#include "basicnodes/basic_forwarders.h"
#include "suifkernel/suifkernel_forwarders.h"
#include "suifkernel/module.h"

class FoldTable : public Module {
  typedef IInteger (*binary_fold_fn)(const IInteger &val1, const IInteger &val2,
				     const IInteger &size, bool is_signed);
  suif_hash_map<LString, binary_fold_fn> *_binary_folds;

  typedef IInteger (*unary_fold_fn)(const IInteger &val,
				    const IInteger &size, bool is_signed);
  suif_hash_map<LString, unary_fold_fn> *_unary_folds;

  typedef Expression *(*expression_fold_fn)(Expression *orig_expr);
  CascadingMap<expression_fold_fn> *_expression_folds;

  typedef Statement *(*statement_fold_fn)(Statement *orig_expr);
  CascadingMap<statement_fold_fn> *_statement_folds;

 public:
  FoldTable(SuifEnv *s, const LString &name = "constant_folding_table");
  ~FoldTable();
  void initialize();
  void initialize_suifnodes_folds();
  Module *clone() const;

  void add_binary_fold_fn(const LString &opcode, binary_fold_fn fn);
  void add_unary_fold_fn(const LString &opcode, unary_fold_fn fn);

  void add_expression_fold_fn(const LString &meta_class, expression_fold_fn fn);
  void add_statement_fold_fn(const LString &meta_class, statement_fold_fn fn);

  IInteger binary_fold(const LString &opcode,
		       const IInteger &val1, const IInteger &val2,
		       const IInteger &bit_size, bool is_signed) const;
  IInteger unary_fold(const LString &opcode,
		      const IInteger &srcval,
		      const IInteger &bit_size, bool is_signed) const;
  Expression *fold_and_replace_expression(Expression *expr);
  Statement *fold_and_replace_statement(Statement *stmt);

  static FoldTable *get_fold_table(SuifEnv *s);
};


#endif /* FOLD_UTILS */
