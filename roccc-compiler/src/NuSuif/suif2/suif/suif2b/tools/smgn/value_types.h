#ifndef __VALUE_TYPES__
#define __VALUE_TYPES__

class VariableSymbol;
class Instruction;
class SourceOp
    {
        union
      {
        int _int;
        VariableSymbol *_variable_symbol;
        struct
          {
            Instruction *_instruction;
            int _operand_num;
          } _instr_data;
      } _data;
    public:
	static LString ClassName;

    };

class DestinationOp
    {
        union
      {
        int _int;
        VariableSymbol *_variable_symbol;
        struct
          {
            Instruction *_instruction;
            int _operand_num;
          } _instr_data;
      } _data;
    public:
	static LString ClassName;
    };

class IntOrSourceOp
  {
private:
    union
      {
        int _int;
        VariableSymbol *_variable_symbol;
        struct
          {
            Instruction *_instruction;
            int _operand_num;
          } _instr_data;
      } _data;

    void clear(void);
    void copy_from(const IntOrSourceOp &other);

    public:
        static LString ClassName;

  };

class Constant
    {
    	int constant;
    public:
	int get_constant() {return constant;}
	static LString ClassName;
    };    

AggregateMetaClass * int_or_source_op_meta;
AggregateMetaClass * source_op_meta;
AggregateMetaClass * constant_meta;
AggregateMetaClass * destination_op_meta;

#endif
