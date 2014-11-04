
%{

  #include <iostream>
  #include <common/suif_list.h>
  #include <common/lstring.h>
  #include <cstdio>
  #include <cstdlib>

  #include "nodes.h"
  #include "typeNodes.h"
  #include "declNodes.h"
  #include "stmtNodes.h"
  #include "exprNodes.h"
  #include "cstNodes.h"
  #include "option.h"
  #include "function.h"
  #include "program.h"
 
  int yyerror(char* s);
  extern int yylex();

  Program* theProgram ;

%}

%union {
  char stringValue[1024];
  int intValue;
  char charValue;
  double realValue ;
  Node* nodePointerValue;
  Option* optionPointerValue;
  opOptions opOptionsValue;
  list<LString>* stringListValue;
  list<opOptions>* opOptionsListValue;
  suif_vector<Node*>* nodePointerListValue;
  Function* functionPointerValue;
}

%start program

%token ERROR_MARK IDENTIFIER_NODE TREE_LIST TREE_VEC BLOCK VOID_TYPE
%token INTEGER_TYPE ENUMERAL_TYPE REAL_TYPE COMPLEX_TYPE VECTOR_TYPE
%token POINTER_TYPE REFERENCE_TYPE METHOD_TYPE FUNCTION_TYPE ARRAY_TYPE
%token RECORD_TYPE UNION_TYPE BOOLEAN_TYPE CHAR_TYPE OFFSET_TYPE FILE_TYPE
%token SET_TYPE QUAL_UNION_TYPE LANG_TYPE CONST_DECL VAR_DECL
%token PARM_DECL FIELD_DECL RESULT_DECL FUNCTION_DECL LABEL_DECL
%token TYPE_DECL NAMESPACE_DECL ASM_STMT BREAK_STMT CONTINUE_STMT
%token CASE_LABEL COMPOUND_STMT DECL_STMT DO_STMT EXPR_STMT FOR_STMT
%token GOTO_STMT IF_STMT LABEL_STMT RETURN_STMT SWITCH_STMT WHILE_STMT
%token SCOPE_STMT INTEGER_CST STRING_CST REAL_CST COMPLEX_CST
%token TRUTH_NOT_EXPR ADDR_EXPR INDIRECT_REF CLEANUP_POINT_EXPR
%token SAVE_EXPR TRUTH_ANDIF_EXPR TRUTH_ORIF_EXPR INIT_EXPR
%token MODIFY_EXPR COMPONENT_REF COMPOUND_EXPR ARRAY_REF
%token BIT_FIELD_REF BUFFER_REF PREDECREMENT_EXPR
%token PREINCREMENT_EXPR POSTDECREMENT_EXPR POSTINCREMENT_EXPR
%token COND_EXPR CALL_EXPR CONSTRUCTOR STMT_EXPR BIND_EXPR LOOP_EXPR
%token EXIT_EXPR TARGET_EXPR METHOD_CALL_EXPR WITH_CLEANUP_EXPR CLEANUP_STMT
%token PLACEHOLDER_EXPR WITH_RECORD_EXPR PLUS_EXPR MINUS_EXPR
%token MULT_EXPR TRUNC_DIV_EXPR CEIL_DIV_EXPR FLOOR_DIV_EXPR ROUND_DIV_EXPR
%token TRUNC_MOD_EXPR CEIL_MOD_EXPR FLOOR_MOD_EXPR ROUND_MOD_EXPR
%token RDIV_EXPR EXACT_DIV_EXPR FIX_TRUNC_EXPR FIX_CEIL_EXPR 
%token FIX_FLOOR_EXPR FIX_ROUND_EXPR FLOAT_EXPR EXPON_EXPR NEGATE_EXPR
%token MIN_EXPR MAX_EXPR ABS_EXPR FFS_EXPR LSHIFT_EXPR RSHIFT_EXPR
%token LROTATE_EXPR RROTATE_EXPR BIT_IOR_EXPR BIT_XOR_EXPR BIT_AND_EXPR
%token BIT_ANDTC_EXPR BIT_NOT_EXPR TRUTH_AND_EXPR TRUTH_OR_EXPR
%token TRUTH_XOR_EXPR LT_EXPR LE_EXPR GT_EXPR GE_EXPR
%token EQ_EXPR NE_EXPR UNORDERED_EXPR ORDERED_EXPR UNLT_EXPR
%token UNLE_EXPR UNGT_EXPR EXPR_WITH_FILE_LOCATION IN_EXPR SET_EL_EXPR
%token CAR_EXPR RANGE_EXPR CONVERT_EXPR NOP_EXPR NON_LVALUE_EXPR
%token UNSAVE_EXPR RTL_EXPR REFERENCE_EXPR 
%token ENTRY_VALUE_EXPR FDESC_EXPR COMPLEX_EXPR CONJ_EXPR REALPART_EXPR
%token IMAGPART_EXPR VA_ARG_EXPR TRY_CATCH_EXPR TRY_FINALLY_EXPR
%token GOTO_SUBROUTINE_EXPR LABEL_EXPR GOTO_EXPR RETURN_EXPR
%token LABELED_BLOCK_EXPR EXIT_BLOCK_EXPR
%token SWITCH_EXPR EXC_PTR_EXPR BINFO NODENUMBER NUMBER NUMBERCOL 
%token REALCONSTANT
%token UNGE_EXPR UNEQ_EXPR STRING CHARACTER
%token NAME TYPE SRCP ARGS EXTERN STRUCT ARTIFICIAL PUBLIC OPERATOR
%token ASSIGN MEMBER PRIVATE PROTECTED CLASS VIRTUAL DESTRUCTOR UNDEFINED
%token BODY STRG LNGT SIZE ALGN RETN PRMS ARGT USED LINE LOW HIGH PREC MIN
%token MAX VALU CHAN QUAL UNQL CSTS PTD REFD CLAS ELTS DOMN VFLD FLDS
%token FNCS BINF SCPE CNST MNGL BPOS NEXT DECL EXPR COND THEN ELSE LABL
%token OP ZERO ONE TWO FN CLNP BASE BASES STMT INIT COP BEGN CLNPCOLON
%token JUNK UNSIGNED END SPEC ZEROCOLON ONECOLON TWOCOLON
%token THREE FOUR TEN PURP FIVE NINE PURE THUNK VCLL DLTA STATIC
%token NULLTOKEN FILE_STMT DELETE EIGHT CTOR_STMT SUBOBJECT LSHIFT
%token PTRMEM CLS MUTABLE EQ CONVERSION REF SUBS GLOBALINIT
%token PRIO DEST MYEOF CALL NEW
%token EH_FILTER_EXPR TYPE_EXPR DYNAMIC_CAST_EXPR MODOP_EXPR SCOPE_REF
%token DECL_EXPR TEMPLATE_TYPE_PARM COMPOUND_LITERAL_EXPR AGGR_INIT_EXPR
%token WITH_SIZE_EXPR OVERLOAD STATEMENT_LIST TRANSLATION_UNIT_DECL
%token SSA_NAME MEMBER_REF CASE_LABEL_EXPR CAST_EXPR DOTSTAR_EXPR
%token ALIGN_INDIRECT_REF THROW_EXPR NW_EXPR REALIGN_LOAD_EXPR
%token BOUND_TEMPLATE_TEMPLATE_PARM FILTER_EXPR TEMPLATE_DECL
%token PHI_NODE BASELINK EMPTY_CLASS_EXPR RESX_EXPR VEC_NW_EXPR
%token MISALIGNED_INDIRECT_REF REINTERPRET_CAST_EXPR VEC_COND_EXPR
%token SIZEOF_EXPR UNBOUND_CLASS_TEMPLATE SCEV_KNOWN VALUE_HANDLE
%token TEMPLATE_ID_EXPR USING_DECL ASM_EXPR DL_EXPR CONST_CAST_EXPR
%token TEMPLATE_PARM_INDEX LTGT_EXPR ALIGNOF_EXPR SCEV_NOT_KNOWN
%token ARRAY_RANGE_REF TREE_BINFO CATCH_EXPR TYPEID_EXPR USING_STMT
%token PSEUDO_DTOR_EXPR PTRMEM_CST VEC_DL_EXPR VIEW_CONVERT_EXPR
%token STATIC_CAST_EXPR TEMPLATE_TEMPLATE_PARM ARROW_EXPR 
%token NON_DEPENDENT_EXPR POLYNOMIAL_CHREC OFFSET_REF
%token OBJ_TYPE_REF DEFAULT_ARG
%token VECTOR_CST TYPENAME_TYPE TYPEOF_TYPE
%token VARS

%type <functionPointerValue> function

%type <nodePointerListValue> tree
%type <stringListValue> attributes
%type <stringListValue> otherattributes

%type <nodePointerValue> gccnode
%type <nodePointerValue> typednode

%type <stringValue> STRING
%type <stringValue> strings
%type <stringValue> attribute
%type <stringValue> otherattribute

%type <charValue> CHARACTER

%type <intValue> NODENUMBER
%type <intValue> NUMBER
%type <intValue> NUMBERCOL

%type <realValue> REALCONSTANT

%type <realValue> realoption

%type <opOptionsListValue> opoptions
%type <opOptionsListValue> numberoptions

%type <opOptionsValue> opoption
%type <opOptionsValue> numberoption
%type <opOptionsValue> othernumberoptions

%type <optionPointerValue> bodyoption
%type <optionPointerValue> strgoption
%type <optionPointerValue> lngtoption
%type <optionPointerValue> sizeoption
%type <optionPointerValue> algnoption
%type <optionPointerValue> retnoption
%type <optionPointerValue> prmsoption
%type <optionPointerValue> argtoption
%type <optionPointerValue> usedoption
%type <optionPointerValue> lineoption
%type <optionPointerValue> lowoption
%type <optionPointerValue> lownodeoption
%type <optionPointerValue> highoption
%type <optionPointerValue> precoption
%type <optionPointerValue> minoption
%type <optionPointerValue> maxoption
%type <optionPointerValue> valuoption
%type <optionPointerValue> nameoption
%type <optionPointerValue> typeoption
%type <optionPointerValue> varsoption
%type <optionPointerValue> srcpoption
%type <optionPointerValue> coption
%type <optionPointerValue> argsoption
%type <optionPointerValue> chanoption
%type <optionPointerValue> qualoption
%type <optionPointerValue> unqloption
%type <optionPointerValue> cstsoption
%type <optionPointerValue> ptdoption
%type <optionPointerValue> refdoption
%type <optionPointerValue> clasoption
%type <optionPointerValue> eltsoption
%type <optionPointerValue> domnoption
%type <optionPointerValue> vfldoption
%type <optionPointerValue> fldsoption
%type <optionPointerValue> fncsoption
%type <optionPointerValue> binfoption
%type <optionPointerValue> scpeoption
%type <optionPointerValue> cnstoption
%type <optionPointerValue> mngloption
%type <optionPointerValue> bposoption
%type <optionPointerValue> nextoption
%type <optionPointerValue> decloption
%type <optionPointerValue> exproption
%type <optionPointerValue> condoption
%type <optionPointerValue> thenoption
%type <optionPointerValue> elseoption
%type <optionPointerValue> fnoption
%type <optionPointerValue> clnpoption
%type <optionPointerValue> stmtoption
%type <optionPointerValue> initoption
%type <optionPointerValue> purpoption
%type <optionPointerValue> dltaoption
%type <optionPointerValue> vclloption
%type <optionPointerValue> clsoption
%type <optionPointerValue> priooption
%type <optionPointerValue> basesoption
%type <optionPointerValue> labloption

%%

program : program2 MYEOF
        {
	  YYACCEPT
	}
        ;

program2 : program2 function
        {
	  theProgram->addFunction($2) ;
	}
        | function 
        {
	  theProgram->addFunction($1) ;
	}
	;

function : header tree
         {
	   $$ = new Function($2);
	 }
         ;

header : JUNK
       ;

tree : gccnode
     {
       $$ = new suif_vector<Node*>();
       $$->push_back($1);
     }
     | tree gccnode
     {
       $1->push_back($2);
       $$ = $1;
     }
     ;

gccnode : NODENUMBER typednode
        {
	  $2->setNumber($1);
	  $$ = $2;
	  cout << $1 << endl ;
        }
        ;

typednode : ERROR_MARK
          { 
	    $$ = new ErrorMark();
	  }
	  | IDENTIFIER_NODE strgoption lngtoption attributes
          { 
	    $$ = new IdentifierNode($2, $3, $4);
	  }
	  | TREE_LIST purpoption valuoption chanoption
          { 
	    $$ = new TreeList($2, $3, $4);
	  }
	  | TREE_VEC lngtoption numberoptions
          { 
	    $$ = new TreeVec($2, $3);
	  }
	  | BLOCK
	  {
	    $$ = new Block();
	  }
	  | VOID_TYPE qualoption unqloption nameoption algnoption
          { 
	    $$ = new VoidTypeMine($2, $3, $4, $5);
	  }
	  | INTEGER_TYPE qualoption nameoption unqloption sizeoption 
	    algnoption precoption attributes minoption maxoption
          { 
	    $$ = new IntegerTypeMine($2, $3, $4, $5, $6, $7, $8, $9, $10);
	  }
	  | ENUMERAL_TYPE qualoption nameoption unqloption
	    sizeoption algnoption precoption attributes minoption
	    maxoption cstsoption
          { 
	    $$ = new EnumeralType($2, $3, $4, $5, $6, $7, $8, $9, $10, $11);
	  }
	  | REAL_TYPE qualoption nameoption unqloption sizeoption algnoption precoption
	  {
	    $$ = new RealType($2, $3, $4, $5, $6, $7);
	  }
	  | COMPLEX_TYPE
	  {
	    yyerror("Complex type is not yet supported\n");
	  }
	  | VECTOR_TYPE
	  {
	    yyerror("Vector type is not yet supported\n");
	  }
	  | POINTER_TYPE qualoption nameoption unqloption sizeoption algnoption
	    ptdoption
          { 
	    $$ = new PointerTypeMine($2, $3, $4, $5, $6, $7);
	  }
	  | REFERENCE_TYPE sizeoption algnoption refdoption
          { 
	    $$ = new ReferenceTypeMine($2, $3, $4); 
	  }
	  | METHOD_TYPE unqloption sizeoption algnoption clasoption retnoption
	    prmsoption
          { 
	    $$ = new MethodType($2, $3, $4, $5, $6, $7);
	  }
	  | FUNCTION_TYPE unqloption sizeoption algnoption
	    retnoption prmsoption
          {
	    $$ = new FunctionType($2, $3, $4, $5, $6);
	  }
	  | ARRAY_TYPE qualoption unqloption sizeoption algnoption 
	    eltsoption domnoption
          { 
	    $$ = new ArrayTypeMine($2, $3, $4, $5, $6, $7);
	  }
	  | RECORD_TYPE qualoption nameoption unqloption sizeoption 
	    algnoption vfldoption attributes fldsoption fncsoption binfoption
	    ptdoption clsoption
          { 
	    $$ = new RecordType($2, $3, $4, $5, $6, $7, $8, $9, $10,
				$11, $12, $13);
	  }
	  | UNION_TYPE
	  {
	    yyerror("Union type is not yet supported\n");
	  }
	  | BOOLEAN_TYPE qualoption nameoption unqloption sizeoption algnoption
          { 
	    $$ = new BooleanTypeMine($2, $3, $4, $5, $6); 
	  }
	  | CHAR_TYPE
	  {
	    yyerror("Char type is not yet supported\n");
	  }
	  | OFFSET_TYPE
	  {
	    yyerror("Offset type is not yet supported\n");
	  }
	  | FILE_TYPE
	  {
	    yyerror("File type is not yet supported\n");
	  }
	  | SET_TYPE
	  {
	    yyerror("Set type is not yet supported\n");
	  }
	  | QUAL_UNION_TYPE
	  {
	    yyerror("Qualified union types are not yet supported\n");
	  }
	  | LANG_TYPE
	  {
	    yyerror("Language defined types are not yet supported\n");
	  }
	  | CONST_DECL nameoption typeoption scpeoption
	    srcpoption cnstoption
          { 
	    $$ = new ConstDecl($2, $3, $4, $5, $6);
	  }
	  | VAR_DECL nameoption mngloption typeoption scpeoption srcpoption
	    attributes otherattributes initoption sizeoption algnoption 
	    usedoption chanoption
          { 
	    $$ = new VarDecl($2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13);
	  }
	  | PARM_DECL nameoption typeoption scpeoption srcpoption
	    attributes argtoption sizeoption algnoption usedoption chanoption
          { 
	    $$ = new ParmDecl($2, $3, $4, $5, $6, $7, $8, $9, $10, $11);
	  }
	  | FIELD_DECL nameoption mngloption typeoption scpeoption
	    srcpoption attributes sizeoption algnoption bposoption chanoption
          { 
	    $$ = new FieldDecl($2, $3, $4, $5, $6, $7, $8, $9, $10, $11);
	  }
	  | RESULT_DECL typeoption scpeoption srcpoption attributes
	    sizeoption algnoption
          { 
	    $$ = new ResultDecl($2, $3, $4, $5, $6, $7);
	  }
	  | FUNCTION_DECL nameoption mngloption typeoption scpeoption 
	    srcpoption attributes priooption dltaoption vclloption 
	    fnoption argsoption otherattributes bodyoption
          { 
	    $$ = new FunctionDecl($2, $3, $4, $5, $6, $7, $8, $9, $10,
				  $11, $12, $13, $14);
	  }
	  | LABEL_DECL nameoption typeoption scpeoption srcpoption attributes
          { 
	    $$ = new LabelDecl($2, $3, $4, $5);
	  }
	  | TYPE_DECL nameoption typeoption scpeoption srcpoption attributes
          { 
	    $$ = new TypeDecl($2, $3, $4, $5, $6);
	  }
	  | NAMESPACE_DECL nameoption typeoption srcpoption coption
          { 
	    $$ = new NamespaceDecl($2, $3, $4, $5);
	  }
	  | ASM_STMT
	  {
	    yyerror("Inline assembly is not yet supported\n");
	  }
	  | BREAK_STMT typeoption lineoption
	  {
	    $$ = new BreakStmt($2, $3) ;
	  }
	  | CONTINUE_STMT
	  {
	    yyerror("Continue statments are not yet supported\n");
	  }
	  | CASE_LABEL
	  {
	    yyerror("Case labels in this form are not yet supported\n");
	  }
	  | DO_STMT typeoption lineoption bodyoption condoption
	  {
	    $$ = new DoStmt($2, $3, $4, $5) ;
	  }
	  | EXPR_STMT typeoption lineoption exproption nextoption
          { 
	    $$ = new ExprStmt($2, $3, $4, $5);
	  }
	  | FOR_STMT typeoption lineoption initoption condoption exproption
	             bodyoption nextoption
	  {
	    $$ = new ForStmt($2, $3, $4, $5, $6, $7, $8);
	  }
	  | IF_STMT typeoption lineoption condoption thenoption elseoption 
	            nextoption
          { 
	    $$ = new IfStmt($2, $3, $4, $5, $6, $7); 
	  }
	  | SWITCH_STMT typeoption lineoption condoption bodyoption
	  {
	    $$ = new SwitchStmt($2, $3, $4, $5) ;
	  }
	  | WHILE_STMT typeoption lineoption condoption bodyoption
	  {
	    $$ = new WhileStmt($2, $3, $4, $5) ;
	  }
	  | INTEGER_CST typeoption highoption lowoption
          { 
	    $$ = new IntegerCst($2, $3, $4);
	  }
	  | STRING_CST typeoption strgoption lngtoption
          { 
	    $$ = new StringCst($2, $3, $4);
	  }
	  | REAL_CST typeoption realoption 
	  {
	    $$ = new RealCst($2, $3);
	  }
	  | COMPLEX_CST
	  {
	    yyerror("Complext constants are not yet supported\n");
	  }
	  | TRUTH_NOT_EXPR typeoption opoptions
	  {
	    $$ = new TruthNotExpr($2, $3);
	  }
	  | ADDR_EXPR typeoption opoptions
          { 
	    $$ = new AddrExpr($2, $3); 
	  }
	  | INDIRECT_REF typeoption opoptions
          { 
	    $$ = new IndirectRef($2, $3); 
	  }
	  | CLEANUP_POINT_EXPR typeoption opoptions
	  {
	    $$ = new CleanupPointExpr($2, $3) ;
	  }
	  | SAVE_EXPR typeoption opoptions
          { 
	    $$ = new SaveExpr($2, $3); 
	  }
	  | TRUTH_ANDIF_EXPR typeoption opoptions
          { 
	    $$ = new TruthAndIfExpr($2, $3) ;
	  }
	  | TRUTH_ORIF_EXPR typeoption opoptions
          { 
	    $$ = new TruthOrIfExpr($2, $3) ;
	  }
	  | INIT_EXPR typeoption opoptions
          { 
	    $$ = new InitExpr($2, $3);
	  }
	  | MODIFY_EXPR typeoption opoptions
          { 
	    $$ = new ModifyExpr($2, $3); 
	  }
	  | COMPONENT_REF typeoption opoptions
          { 
	    $$ = new ComponentRef($2, $3);
	  }
	  | COMPOUND_EXPR typeoption opoptions
          { 
	    $$ = new CompoundExpr($2, $3); 
	  }
	  | ARRAY_REF typeoption opoptions
	  {
	    $$ = new ArrayRef($2, $3);
	  }
	  | BIT_FIELD_REF
	  {
	    yyerror("Bit field references are not yet supported\n");
	  }
	  | BUFFER_REF
	  {
	    yyerror("Buffer references are not yet supported\n");
	  }
	  | PREDECREMENT_EXPR typeoption opoptions
	  {
	    $$ = new PredecrementExpr($2, $3) ;
	  }
	  | PREINCREMENT_EXPR typeoption opoptions
          { 
	    $$ = new PreincrementExpr($2, $3) ; 
	  }
	  | POSTDECREMENT_EXPR typeoption opoptions
	  {
	    $$ = new PostdecrementExpr($2, $3) ;
	  }
	  | POSTINCREMENT_EXPR typeoption opoptions
	  {
	    $$ = new PostincrementExpr($2, $3);
	  }
	  | COND_EXPR typeoption opoptions
          { 
	    $$ = new CondExpr($2, $3); 
	  }
	  | CALL_EXPR typeoption fnoption argsoption
          { 
	    $$ = new CallExpr($2, $3, $4); 
	  }
	  | CONSTRUCTOR typeoption eltsoption
          {
	    $$ = new Constructor($2, $3); 
	  }
	  | STMT_EXPR typeoption stmtoption
          { 
	    $$ = new StmtExpr($2, $3);
	  }
	  | BIND_EXPR typeoption varsoption bodyoption
	  {
	    $$ = new BindExpr($2, $3, $4) ;
	  }
	  | LOOP_EXPR
	  {
	    yyerror("Loop expressions are not yet supported\n");
	  }
	  | TARGET_EXPR typeoption decloption initoption clnpoption
          { 
	    $$ = new TargetExpr($2, $3, $4, $5) ; 
	  }
	  | METHOD_CALL_EXPR
	  {
	    yyerror("Method call expressions are not yet supported\n");
	  }
	  | WITH_CLEANUP_EXPR
	  {
	    yyerror("With cleanup expressions are not yet supported\n");
	  }
	  | CLEANUP_STMT lineoption decloption exproption nextoption 
          { 
	    $$ = new CleanupStmt($2, $3, $4, $5); 
	  }
	  | PLACEHOLDER_EXPR
	  {
	    yyerror("Placeholder expressions are not yet supported\n");
	  }
	  | WITH_RECORD_EXPR
	  {
	    yyerror("With record expressions are not yet supported\n");
	  }
	  | PLUS_EXPR typeoption opoptions
          { 
	    $$ = new PlusExpr($2, $3); 
	  }
	  | MINUS_EXPR typeoption opoptions
	  {
	    $$ = new MinusExpr($2, $3);
	  }
	  | MULT_EXPR typeoption opoptions
          { 
	    $$ = new MultExpr($2, $3); 
	  }
	  | TRUNC_DIV_EXPR typeoption opoptions
	  {
	    $$ = new TruncDivExpr($2, $3) ;
	  }
	  | CEIL_DIV_EXPR
	  {
	    yyerror("Ceiling Division expressions are not yet supported\n");
	  }
	  | FLOOR_DIV_EXPR
	  {
	    yyerror("Floor division expressions are not yet supported\n");
	  }
	  | ROUND_DIV_EXPR
	  {
	    yyerror("Rounding division is not yet supported\n");
	  }
	  | TRUNC_MOD_EXPR typeoption opoptions
	  {
	    $$ = new TruncModExpr($2, $3) ;
	  }
	  | CEIL_MOD_EXPR
	  {
	    yyerror("Ceiling mod expressions are not yet supported\n");
	  }
	  | FLOOR_MOD_EXPR
	  {
	    yyerror("Floor mod expressions are not yet supported\n");
	  }
	  | ROUND_MOD_EXPR
	  {
	    yyerror("Rounding mod expressions are not yet supported\n");
	  }
	  | RDIV_EXPR typeoption opoptions 
	  {
	    $$ = new RdivExpr($2, $3) ;
	  }
	  | EXACT_DIV_EXPR
	  {
	    yyerror("Exact div expressions are not yet supported\n");
	  }
	  | FIX_TRUNC_EXPR typeoption opoptions
	  {
	    // This is a casting operation!
	    $$ = new FixTruncExpr($2, $3) ;
	  }
	  | FIX_CEIL_EXPR
	  {
	    yyerror("Fixed ceiling expressions are not yet supported\n");
	  }
	  | FIX_FLOOR_EXPR
	  {
	    yyerror("Fixed floor expressions are not yet supported\n");
	  }
	  | FIX_ROUND_EXPR
	  {
	    yyerror("Fixed rounding expressions are not yet supported\n");
	  }
	  | FLOAT_EXPR typeoption opoptions
	  {
	    $$ = new FloatExpr($2, $3) ;
	  }
	  | EXPON_EXPR
	  {
	    yyerror("Exponential expressions are not yet supported\n");
	  }
	  | NEGATE_EXPR typeoption opoptions
	  {
	    $$ = new NegateExpr($2, $3) ;
	  }
	  | MIN_EXPR
	  {
	    yyerror("Min expressions are not yet supported\n");
	  }
	  | MAX_EXPR typeoption opoptions
	  {
	    $$ = new MaxExpr($2, $3) ;
	  }
	  | ABS_EXPR
	  {
	    yyerror("Absolute value expressions are not yet supported\n");
	  }
	  | FFS_EXPR
	  {
	    yyerror("FFS expressions (?) are not yet supported\n");
	  }
	  | LSHIFT_EXPR typeoption opoptions
	  {
	    $$ = new LshiftExpr($2, $3);
	  }
	  | RSHIFT_EXPR typeoption opoptions
	  {
	    $$ = new RshiftExpr($2, $3) ;
	  }
	  | LROTATE_EXPR
	  {
	    yyerror("Rotate left expressions are not yet supported\n");
	  }
	  | RROTATE_EXPR
	  {
	    yyerror("Rotatae right expressions are not yet supported\n");
	  }
	  | BIT_IOR_EXPR typeoption opoptions 
	  {
	    $$ = new BitIorExpr($2, $3) ; 
	  }
	  | BIT_XOR_EXPR typeoption opoptions
	  {
	    $$= new BitXorExpr($2, $3);
	  }
	  | BIT_AND_EXPR typeoption opoptions
          { 
	    $$ = new BitAndExpr($2, $3); 
	  }
	  | BIT_ANDTC_EXPR
	  {
	    yyerror("Bitwise AND_TC expressions are not yet supported\n");
	  }
	  | BIT_NOT_EXPR typeoption opoptions
	  {
	    $$ = new BitNotExpr($2, $3);
	  }
	  | TRUTH_AND_EXPR typeoption opoptions
	  {
	    $$ = new TruthAndExpr($2, $3) ;
	  }
	  | TRUTH_OR_EXPR typeoption opoptions 
	  {
	    $$ = new TruthOrExpr($2, $3) ;
	  }
	  | TRUTH_XOR_EXPR
	  {
	    yyerror("Truth xor expressions are not yet supported\n");
	  }
	  | LT_EXPR typeoption opoptions
          { 
	    $$ = new LtExpr($2, $3); 
	  }
	  | LE_EXPR typeoption opoptions
	  {
	    $$ = new LeExpr($2, $3);
	  }
	  | GT_EXPR typeoption opoptions
          { 
	    $$ = new GtExpr($2, $3); 
	  }
	  | GE_EXPR typeoption opoptions 
	  {
	    $$ = new GeExpr($2, $3) ;
	  }
	  | EQ_EXPR typeoption opoptions
          { 
	    $$ = new EqExpr($2, $3); 
	  }
	  | NE_EXPR typeoption opoptions
          { 
	    $$ = new NeExpr($2, $3); 
	  }
	  | UNORDERED_EXPR
	  {
	    yyerror("Unordered expressions are not yet supported\n");
	  }
	  | ORDERED_EXPR
	  {
	    yyerror("Ordered expressions are not yet supported\n");
	  }
	  | UNLT_EXPR
	  {
	    yyerror("UNLT (unsigned less thhan?) is not yet supported\n");
	  }
	  | UNLE_EXPR
	  {
	    yyerror("UNLE is not yet supported\n");
	  }
	  | UNGT_EXPR
	  {
	    yyerror("UNGT is not yet supported\n");
	  }
	  | UNGE_EXPR
	  {
	    yyerror("UNGE is not yet supported\n");
	  }
	  | UNEQ_EXPR
	  {
	    yyerror("UNEQ is not yet supported\n");
	  }
	  | EXPR_WITH_FILE_LOCATION
	  {
	    yyerror("Expression with file location is not yet supported\n");
	  }
	  | IN_EXPR
	  {
	    yyerror("IN expression is not yet supported\n");
	  }
	  | SET_EL_EXPR
	  {
	    yyerror("Set El expression is not yet supported\n");
	  }
	  | CAR_EXPR
	  {
	    yyerror("Car expression is not yet supported\n");
	  }
	  | RANGE_EXPR
	  {
	    yyerror("Range expression is not yet supported\n");
	  }
	  | CONVERT_EXPR typeoption opoptions
          { 
	    $$ = new ConvertExpr($2, $3); 
	  }
	  | NOP_EXPR typeoption opoptions
          { 
	    $$ = new NopExpr($2, $3);
	  }
	  | NON_LVALUE_EXPR typeoption opoptions
          { 
	    $$ = new NonLvalueExpr($2, $3); 
	  }
	  | UNSAVE_EXPR
	  {
	    yyerror("Unsave expression is not yet supported\n");
	  }
	  | RTL_EXPR
	  {
	    yyerror("RTL expression is not yet supported\n");
	  }
	  | REFERENCE_EXPR
	  {
	    yyerror("Reference expressions are not yet supported\n");
	  }
	  | ENTRY_VALUE_EXPR
	  {
	    yyerror("Entry value expressions are not yet supported\n");
	  }
	  | FDESC_EXPR
	  {
	    yyerror("File descriptor expressions are not yet supported\n");
	  }
	  | COMPLEX_EXPR
	  {
	    yyerror("Complex expressions are not yet supported\n");
	  }
	  | CONJ_EXPR
	  {
	    yyerror("CONJ expressions are not yet supported\n");
	  }
	  | REALPART_EXPR
	  {
	    yyerror("Real-part expressions are not yet supported\n");
	  }
	  | IMAGPART_EXPR
	  {
	    yyerror("Imaginary part expressions are not yet supported\n");
	  }
	  | VA_ARG_EXPR
	  {
	    yyerror("Variable arguments are not yet supported\n");
	  }
	  | TRY_CATCH_EXPR
	  {
	    yyerror("Try-catch expressions are not yet supported\n");
	  }
	  | TRY_FINALLY_EXPR
	  {
	    yyerror("Try-finally expressions are not yet supported\n");
	  }
	  | GOTO_SUBROUTINE_EXPR
	  {
	    yyerror("Goto subroutine expressions are not yet supported\n");
	  }
	  | LABEL_EXPR typeoption nameoption
	  {
	    $$ = new LabelExpr($2, $3) ;
	  }
	  | GOTO_EXPR typeoption labloption
	  {
	    $$ = new GotoExpr($2, $3) ;
	    //	    yyerror("Goto expressions are not yet supported\n");
	  }
	  | RETURN_EXPR typeoption exproption
	  {
	    // This reflects the 4.1 tree, but it still works with 4.0
	    $$ = new ReturnExpr($2, $3) ;
	  }
	  | EXIT_EXPR
	  {
	    yyerror("Exit expressions are not yet supported\n");
	  }
	  | LABELED_BLOCK_EXPR
	  {
	    yyerror("Labeled Block expressions are not yet supported\n");
	  }
	  | EXIT_BLOCK_EXPR
	  {
	    yyerror("Exit block expressions are not yet supported\n");
	  }
	  | SWITCH_EXPR
	  {
	    yyerror("Switch expressions are not yet supported\n");
	  }
	  | EXC_PTR_EXPR
	  {
	    yyerror("Exc-pointer expressions are not yet supported\n");
	  }
          | ALIGN_INDIRECT_REF
          {
            yyerror("Allign indirect references are not yet supported\n") ;
          }
          | MISALIGNED_INDIRECT_REF
          { 
            yyerror("Misaligned indirect references are not yet supported\n") ;
          }
          | ARRAY_RANGE_REF
          {
            yyerror("Array range references are not yet supported\n") ;
          }
          | OBJ_TYPE_REF
          {
            yyerror("Object type references are not yet supported\n") ;
          }
          | SSA_NAME
          {
            yyerror("SSA name is not yet supported\n") ;
          }
          | PHI_NODE
          {
            yyerror("Phi nodes are not yet supported\n") ;
          }
          | SCEV_KNOWN
          {
            yyerror("SCEV_KNOWN is not yet supported\n") ;
          }
          | SCEV_NOT_KNOWN
          {
            yyerror("SCEV_NOT_KNOWN is not yet supported\n") ;
          }
          | POLYNOMIAL_CHREC
          {
            yyerror("Polynomial_cherc is not yet supported\n") ;
          }
          | STATEMENT_LIST numberoptions
          {
	    $$ = new StatementListMine($2) ;
          }
          | VALUE_HANDLE
          {
            yyerror("Value handles are not yet supported\n") ;
          }
          | TREE_BINFO
          {
            yyerror("Tree-binfo is not yet supported\n") ;
          }
          | OFFSET_REF
          {
            yyerror("Offset References are not yet supported\n") ;
          }
          | SCOPE_REF
          {
            yyerror("Scope references are not yet supported\n") ;
          }
          | MEMBER_REF
          {
            yyerror("Member references are not yet supported\n") ;
          }
          | BASELINK
          {
            yyerror("Baselink is not yet supported\n") ;
          }
          | TEMPLATE_PARM_INDEX
          {
            yyerror("Template parameter indexes are not yet supported\n") ;
          }
          | TEMPLATE_TEMPLATE_PARM
          {
            yyerror("Template-template parameters are not yet supported\n") ;
          }
          | TEMPLATE_TYPE_PARM
          {
            yyerror("Template type parameters are not yet supported\n") ;
          }
          | BOUND_TEMPLATE_TEMPLATE_PARM
          {
            yyerror("Bound template-template parameters are not yet supported\n") ;
          }
          | UNBOUND_CLASS_TEMPLATE
          {
            yyerror("Unbound class templates are not yet supported\n") ;
          }
          | USING_STMT
          {
            yyerror("Using statements are not yet supported\n") ;
          }
          | DEFAULT_ARG
          {
            yyerror("Default arguments are not yet supported\n") ;
          }
          | OVERLOAD
          {
            yyerror("Overload is not yet supported\n") ;
          }
          | FILTER_EXPR 
          {
            yyerror("Filter expressions are not yet supported\n") ;
          }
          | VEC_COND_EXPR
          {
            yyerror("Vectored conditional expressions are not yet supported\n") ;
          }
          | LTGT_EXPR
          {
            yyerror("LTGT expressions are not yet supported\n") ;
          }
          | VIEW_CONVERT_EXPR
          {
            yyerror("View convert expressions are not yet supported\n") ;
          }
          | DECL_EXPR typeoption
          {
	    $$ = new DeclExpr($2) ;
          }
          | CASE_LABEL_EXPR typeoption lownodeoption
          {
	    $$ = new CaseLabelExpr($2, $3) ;
          }
          | RESX_EXPR
          {
            yyerror("RESX expressions are not yet supported\n") ;
          }
          | ASM_EXPR
          {
            yyerror("Asm expressions are not yet supported\n") ;
          }
          | CATCH_EXPR
          {
            yyerror("Catch expressions are not yet supported\n") ;
          }
          | EH_FILTER_EXPR
          {
            yyerror("Eh-filter expressions are not yet supported\n") ;
          }
          | WITH_SIZE_EXPR
          {
            yyerror("With-size expressions are not yet supported\n") ;
          }
          | REALIGN_LOAD_EXPR
          {
            yyerror("Realign-load expressions are not yet supported\n") ;
          }
          | SIZEOF_EXPR
          {
            yyerror("Sizeof expression is not yet supported\n") ;
          }
          | ALIGNOF_EXPR
          {
            yyerror("Align-of expressions is not yet supported\n") ;
          }
          | ARROW_EXPR
          {
            yyerror("Arrow expression is not yet supported\n") ;
          }
          | COMPOUND_LITERAL_EXPR 
          {
            yyerror("Compound literal expressions are not yet supported\n") ;
          }
          | NW_EXPR
          {
            yyerror("NW expressions are not yet supported\n") ;
          }
          | VEC_NW_EXPR 
          {
            yyerror("Vec-NW expressions are not yet supported\n") ;
          }
          | DL_EXPR
          {
            yyerror("DL expressions are not yet supported\n") ;
          }
          | VEC_DL_EXPR
          {
            yyerror("VEC-DL expressions are not yet supported\n") ;
          }
          | TYPE_EXPR
          {
            yyerror("Type expressions are not yet supported\n") ;
          }
          | AGGR_INIT_EXPR
          {
            yyerror("Aggr-init expressions are not yet supported\n") ;
          }
          | THROW_EXPR
          {
            yyerror("Throw expressions are not yet supported\n") ;
          }
          | EMPTY_CLASS_EXPR
          {
            yyerror("Empty class expressions are not yet supported\n") ;
          }
          | TEMPLATE_ID_EXPR
          {
            yyerror("Template id expressions are not yet supported\n") ;
          }
          | PSEUDO_DTOR_EXPR
          {
            yyerror("Pseudo-destructors are not yet supported\n") ;
          }
          | MODOP_EXPR
          {
            yyerror("Mod operations are not yet supported\n") ;
          }
          | CAST_EXPR
          {
            yyerror("Casting is not yet supported\n") ;
          }
          | REINTERPRET_CAST_EXPR
          {
            yyerror("Reinterpret-casts are not yet supported\n") ;
          }
          | CONST_CAST_EXPR
          {
            yyerror("Const-casts are not yet supported\n") ;
          }
          | STATIC_CAST_EXPR
          {
            yyerror("Static-casts are not yet supported\n") ;
          }
          | DYNAMIC_CAST_EXPR
          {
            yyerror("Dynamic-casts are not yet supported\n") ;
          }
          | DOTSTAR_EXPR
          {
            yyerror("The dot-star expression is not yet supported\n") ;
          }
          | TYPEID_EXPR
          {
            yyerror("Typeid expressions are not yet supported\n") ;
          }
          | NON_DEPENDENT_EXPR
          {
            yyerror("Non-dependent expressions are not yet supported\n") ;
          }
          | TRANSLATION_UNIT_DECL
          {
            yyerror("Translation unit declarations are not yet supported\n") ;
          }
          | TEMPLATE_DECL
          {
            yyerror("Template declarations are not yet supported\n") ;
          }
          | USING_DECL
          {
            yyerror("Using declarations are not yet supported\n") ;
          }
          | PTRMEM_CST
          {
            yyerror("Pointer to memory constants are not yet supported\n") ;
          }
          | VECTOR_CST
          {
            yyerror("Vector constants are not yet supported\n") ;
          }
          | TYPENAME_TYPE
          {
            yyerror("Typename types are not yet supported\n") ;
          }
          | TYPEOF_TYPE
          {
            yyerror("Typeop types are not yet supported\n") ;
          }
          | BINFO typeoption basesoption 
          {
	    $$ = new Binfo($2, $3) ;
	  }
	  ;

nameoption : NAME NODENUMBER
           {
	     $$ = new NameOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

typeoption : TYPE NODENUMBER
           {
	     $$ = new TypeOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

varsoption : VARS NODENUMBER
           {
	     $$ = new VarsOption($2) ;
	   }
           |
           {
	     $$ = NULL ;
	   }
           ;

srcpoption : SRCP strings
           {
	     $$ = new SrcpOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

coption : COP
        {
	  $$ = new COption();
	}
	|
        {
	  $$ = NULL;
	}
	;

argsoption : ARGS NODENUMBER
           {
	     $$ = new ArgsOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

labloption : LABL NODENUMBER
           {
	     $$ = new LablOption($2) ;
           }
           |
           {
	     $$ = NULL ;
           }
           ;

attributes : attributes attribute
           {
	     $1->push_back($2);
	     $$ = $1;
	   }
	   |
           {
	     $$ = new list<LString>();
	   }
	   ;

attribute : STRUCT
          {
	    strcpy($$, "struct");
          }
	  | ARTIFICIAL
          {
	    strcpy($$, "artificial") ;
	  }
	  | PUBLIC
          {
	    strcpy($$, "public") ;
	  }
	  | OPERATOR
          {
	    strcpy($$, "operator");
	  }
	  | ASSIGN
          {
	    strcpy($$, "assign") ;
	  }
	  | MEMBER
          {
	    strcpy($$, "member") ;
	  }
	  | PRIVATE
          {
	    strcpy($$, "private");
	  }
	  | PROTECTED
          {
	    strcpy($$, "protected");
	  }
          | MUTABLE
          {
	    strcpy($$, "mutable") ;
	  }
	  | CLASS
          {
	    strcpy($$, "class") ;
	  }
	  | CONSTRUCTOR
          {
	    strcpy($$, "constructor") ;
	  }
	  | VIRTUAL
          {
	    strcpy($$, "virtual") ;
	  }
	  | DESTRUCTOR
          {
	    strcpy($$, "destructor") ;
	  }
          | BEGN
          {
	    strcpy($$, "begn") ;
	  }
          | CLNP
          {
	    strcpy($$, "clnp") ;
	  }
          | UNSIGNED
          {
	    strcpy($$, "unsigned") ;
	  }
          | END
          {
	    strcpy($$, "end") ;
	  }
          | SPEC
          {
	    strcpy($$, "spec") ;
	  }
          | PURE
          {
	    strcpy($$, "pure") ;
	  }
          | THUNK
          {
	    strcpy($$, "thunk") ;
	  }
          | NULLTOKEN
          {
	    strcpy($$, "null") ;
	  }
          | DELETE
          {
	    strcpy($$, "delete") ;
	  }
          | LSHIFT
          {
	    strcpy($$, "lshift") ;
	  }
          | PTRMEM
          {
	    strcpy($$, "ptrmem") ;
	  }
          | COP
          {
	    strcpy($$, "C") ;
	  }
          | EQ
          {
	    strcpy($$, "eq") ;
	  }
          | CONVERSION
          {
	    strcpy($$, "conversion") ;
	  }
          | REF
          {
	    strcpy($$, "ref") ;
	  }
          | SUBS
          {
	    strcpy($$, "subs") ; 
	  }
          | GLOBALINIT
          {
	    strcpy($$, "global init") ;
	  }
          | CALL
          {
	    strcpy($$, "call") ;
	  }
	  ;

otherattributes : otherattributes otherattribute
                {
		  $1->push_back($2);
		  $$ = $1;
		}
                |
                {
		  $$ = new list<LString>();
		}
                ;

otherattribute : UNDEFINED
               {
		 strcpy($$, "undefined");
	       }
	       | EXTERN
               {
		 strcpy($$, "extern");
	       }
               | STATIC
               {
		 strcpy($$, "static");
	       }
               | NEW
               {
		 strcpy($$, "new") ;
               }
	       ;

bodyoption : BODY NODENUMBER
           {
	     $$ = new BodyOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

strgoption : STRG strings
           {
	     $$ = new StrgOption($2);
	   }
           | STRG BLOCK
           {
             /* Just in case the user creates a variable named "block"  */
	     $$ = new StrgOption(LString("block")) ;
	   }
	   |
           {
	     $$ = NULL;
           }
	   ;

strings : strings STRING
        {
	  ;
	  //strcpy($$,  strcat($1, $2));;
	}
        | strings CHARACTER
        {
	  ;
	  //$1[strlen($1)+1] = '\0';
	  //$1[strlen($1)] = $2 ;
	  //strcpy($$, $1);
	}
        | strings NUMBER
        {
	  ;
	}
        | STRING 
        {
	  ;
	  strcpy($$,$1);
	}
        | CHARACTER
        {
	  ;
	  //cout << "Just read the character: " << $1 << endl ;
	  //;
	  $$[0] = $1;
	  $$[1] = '\0';
	}
        | COP
        {
	  $$[0] = 'C' ;
	  $$[1] = '\0' ;
          ; // Just in case anyone calls a variable 'C'
        }
        | END
        {
          $$[0] = 'e' ;
	  $$[1] = 'n' ;
	  $$[2] = 'd' ;
	  $$[3] = '\0' ;
        }
        | NUMBER
        {
	  ;
	}
        ;

lngtoption : LNGT NUMBER
           {
	     $$ = new LngtOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

sizeoption : SIZE NODENUMBER
           {
	     $$ = new SizeOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

algnoption : ALGN NUMBER
           {
	     $$ = new AlgnOption($2);
           }
	   |           
           {
	     $$ = NULL;
	   }
	   ;

retnoption : RETN NODENUMBER
           {
	     $$ = new RetnOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

prmsoption : PRMS NODENUMBER
           {
	     $$ = new PrmsOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

argtoption : ARGT NODENUMBER
           {
	     $$ = new ArgtOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

usedoption : USED NUMBER
           {
	     $$ = new UsedOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

lineoption : LINE NUMBER
           {
	     $$ = new LineOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

lowoption : LOW NUMBER
          {
	    $$ = new LowOption($2);
	  }
	  |
          {
	    $$ = NULL;
	  }
	  ;

lownodeoption : LOW NODENUMBER
                {
		  $$ = new LowNodeOption($2) ;
		}
                |
                {
		  $$ = NULL ;
                }

highoption : HIGH NUMBER
           {
	     $$ = new HighOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

precoption : PREC NUMBER
           {
	     $$ = new PrecOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

minoption : MIN NODENUMBER
          {
	    $$ = new MinOption($2);
	  }
	  |
          {
	    $$ = NULL;
	  }
	  ;

maxoption : MAX NODENUMBER
          {
	    $$ = new MaxOption($2);
	  }
	  |
          {
	    $$ = NULL;
	  }
	  ;

valuoption : VALU NODENUMBER
           {
	     $$ = new ValuOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

chanoption : CHAN NODENUMBER
           {
	     $$ = new ChanOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

numberoptions : numberoptions numberoption
              {
		$1->push_back($2);
		$$ = $1;
	      }
              |
              {
		$$ = new list<opOptions>();
	      }
	      ;

qualoption : QUAL CHARACTER
           {
	     $$ = new QualOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

unqloption : UNQL NODENUMBER
           {
	     $$ = new UnqlOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

cstsoption : CSTS NODENUMBER
           {
	     $$ = new CstsOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

ptdoption : PTD NODENUMBER
          {
	    $$ = new PtdOption($2);
	  }
	  |
          {
	    $$ = NULL;
	  }
	  ;

refdoption : REFD NODENUMBER
           {
	     $$ = new RefdOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

clasoption : CLAS NODENUMBER
           {
	     $$ = new ClasOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

eltsoption : ELTS NODENUMBER
           {
	     $$ = new EltsOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

domnoption : DOMN NODENUMBER
           {
	     $$ = new DomnOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

vfldoption : VFLD NODENUMBER
           {
	     $$ = new VfldOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

fldsoption : FLDS NODENUMBER
           {
	     $$ = new FldsOption($2);
	   }
	   |
           {
	     $$ = NULL;
           }
	   ;

fncsoption : FNCS NODENUMBER
           {
	     $$ = new FncsOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

binfoption : BINF NODENUMBER
           {
	     $$ = new BinfOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

scpeoption : SCPE NODENUMBER
           {
	     $$ = new ScpeOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

cnstoption : CNST NODENUMBER
           {
	     $$ = new CnstOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

mngloption : MNGL NODENUMBER
           {
	     $$ = new MnglOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

bposoption : BPOS NODENUMBER
           {
	     $$ = new BposOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

nextoption : NEXT NODENUMBER
           {
	     $$ = new NextOption($2);
           }
	   |
           {
	     $$ = NULL;
	   }
	   ;

decloption : DECL NODENUMBER
           {
	     $$ = new DeclOption($2);
           }
	   |
           {
	     $$ = NULL;
	   }
	   ;

exproption : EXPR NODENUMBER
           {
	     $$ = new ExprOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

condoption : COND NODENUMBER
           {
	     $$ = new CondOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

thenoption : THEN NODENUMBER
           {
	     $$ = new ThenOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

elseoption : ELSE NODENUMBER
           {
	     $$ = new ElseOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

fnoption : FN NODENUMBER
         {
	   $$ = new FnOption($2);
	 }
	 |
         {
	   $$ = NULL;
         }
	 ;

clnpoption : CLNPCOLON NODENUMBER
           {
	     $$ = new ClnpOption($2);
	   }
	   |
           {
	     $$ = NULL;
	   }
	   ;

stmtoption : STMT NODENUMBER
           {
	     $$ = new StmtOption($2);
	   }
	   |
           {
	     $$ = NULL;
           }
	   ;

initoption : INIT NODENUMBER
           {
	     $$ = new InitOption($2);
	   }
	   |
           {
	     $$ = NULL ;
           }
	   ;

purpoption : PURP NODENUMBER
           {
	     $$ = new PurpOption($2);
	   }
           |
           {
	     $$ = NULL;
           }
           ;

dltaoption : DLTA NUMBER
           {
	     $$ = new DltaOption($2);
           }
           |
           {
	     $$ = NULL;
           }
           ;

vclloption : VCLL NODENUMBER
           {
	     $$ = new VcllOption($2);
           }
           |
           {
	     $$ = NULL;
	   }
           ;

clsoption : CLS NODENUMBER
          {
	    $$ = new ClsOption($2)
	  }
          |
          {
	    $$ = NULL;
	  }
          ;

basesoption : BASES NUMBER
            {
	      $$ = new BasesOption($2) ;
	    }
            |
            {
	      $$ = NULL ;
	    }

priooption : PRIO NUMBER
           {
	     $$ = new PrioOption($2);
           }
           |
           {
	     $$ = NULL;
           }
           ;


opoptions : opoptions opoption
          {
	    $1->push_back($2);
	    $$ = $1;
	  }
	  | 
          {
	    $$ = new list<opOptions>();
	  }
	  ;

opoption : OP othernumberoptions
         {
	   $$ = $2;
	 }
	 ;

othernumberoptions : ZEROCOLON NODENUMBER
                   {
		     opOptions x;
		     x.op = 0;
		     x.nodeNumber = $2;
		     $$ = x;
		   }
                   | ONECOLON NODENUMBER
                   {
		     opOptions x;
		     x.op = 1;
		     x.nodeNumber = $2;
		     $$ = x;
		   }
                   | TWOCOLON NODENUMBER
                   {
		     opOptions x;
		     x.op = 2;
		     x.nodeNumber = $2;
		     $$ = x;
		   }
                   ;

numberoption : ZERO NODENUMBER
             {
	       opOptions x;
	       x.op = 0;
	       x.nodeNumber = $2;
	       $$ = x;
	     }
	     | ONE NODENUMBER
             {
	       opOptions x;
	       x.op = 1;
	       x.nodeNumber = $2;
	       $$ = x;
	     }
	     | TWO NODENUMBER
             {
	       opOptions x;
	       x.op = 2;
	       x.nodeNumber = $2;
	       $$ = x;
	     }
             | THREE NODENUMBER
             {
	       opOptions x;
	       x.op = 3;
	       x.nodeNumber = $2;
	       $$ = x;
	     }
             | FOUR NODENUMBER
             {
	       opOptions x;
	       x.op = 4;
	       x.nodeNumber = $2;
	       $$ = x;
	     }
             | FIVE NODENUMBER
             {
	       opOptions x;
	       x.op = 5;
	       x.nodeNumber = $2;
	       $$ = x;
	     }
             | EIGHT NODENUMBER
             {
	       opOptions x;
	       x.op = 8;
	       x.nodeNumber = $2;
	       $$ = x;
	     }
             | NINE NODENUMBER
             {
	       opOptions x;
	       x.op = 9;
	       x.nodeNumber = $2;
	       $$ = x;
	     }
             | TEN NODENUMBER
             {
	       opOptions x;
	       x.op = 10;
	       x.nodeNumber = $2;
	       $$ = x;
	     }
             | NUMBER ':' NODENUMBER
             {
	       opOptions x ;
               x.op = $1 ;
               x.nodeNumber = $3 ;
               $$ = x ;
             }
             | NUMBERCOL NODENUMBER
             {
	       opOptions x ;
	       x.op = $1 ;
	       x.nodeNumber = $2 ;
	       $$ = x ;
	     }
	     ;

realoption : REALCONSTANT
             {
	       $$ = $1 ;
	     }
             |
             {
	       $$ = 0 ;
	     }

%%

extern FILE* yyin ;

int main(int argc, char* argv[])
{

  if (argc <= 1 || argc > 3 )
  {
    printf("Usage: %s INPUT_FILE [OUTPUT_FILE]\n", argv[0]) ;
    return 0 ;
  }

  if (argc >= 2)
  {
    yyin = fopen(argv[1], "r") ;
    if (!yyin)
    {
      return 0 ;
    }
    theProgram = new Program(argv[1]) ;
  }
  else
  {
    theProgram = new Program() ;
  }
  
  if (argc == 3)
  {
    LString fName = argv[2] ;
    theProgram->setOutputFile(fName) ;
  }

  yyparse();

  theProgram->connect() ;

  theProgram->generateSuif() ;

  delete theProgram ;

  return 0;

}

int yyerror(char* s)
{
  cerr << s  << endl ;
  return 0;
}
