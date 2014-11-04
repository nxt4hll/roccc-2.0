// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include <common/suif_copyright.h>
#include <common/suif_list.h>

#include <iostream>
#include <iokernel/cast.h>
#include <iokernel/clone_stream.h>
#include <common/i_integer.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>
#include <basicnodes/basic.h>
#include <basicnodes/basic_constants.h>
#include <suifkernel/suifkernel_messages.h>
#include <suifkernel/utilities.h> 
#include <suifkernel/group_walker.h> 
#include "transforms/procedure_walker_utilities.h"
#include <utils/expression_utils.h>
#include <utils/symbol_utils.h>
#include <utils/type_utils.h>
#include <utils/cloning_utils.h>
#include <cfenodes/cfe.h>
#include "annote_utils.h"

/**************************** Declerations ************************************/


/************************** Implementations ***********************************/

Type* get_base_type(Type* t){

      if(is_a<PointerType>(t))
         t = (to<PointerType>(t))->get_reference_type();
      if(is_a<QualifiedType>(t))
         t = (to<QualifiedType>(t))->get_base_type();
      if(is_a<IntegerType>(t))
         t = to<IntegerType>(t);

      return t;
}

DataType* get_array_element_type(ArrayReferenceExpression *a_ref){

      Expression *base_array_address = a_ref;

      do{
         base_array_address = (to<ArrayReferenceExpression>(base_array_address))->get_base_array_address();
      }while(is_a<ArrayReferenceExpression>(base_array_address));

      SymbolAddressExpression *array_sym_expr = to<SymbolAddressExpression>(base_array_address);
      VariableSymbol *array_sym = to<VariableSymbol>(array_sym_expr->get_addressed_symbol());
      DataType *current_type = (array_sym->get_type())->get_base_type();

      do{
         ArrayType *array_type = to<ArrayType>(current_type);
         current_type = (array_type->get_element_type())->get_base_type();
      }while(is_a<ArrayType>(current_type));

      return current_type;
}

DataType* get_array_element_type(SymbolAddressExpression *sym_address_expr){

      VariableSymbol *array_sym = to<VariableSymbol>(sym_address_expr->get_addressed_symbol());
      DataType *current_type = (array_sym->get_type())->get_base_type();

      do{
         ArrayType *array_type = to<ArrayType>(current_type);
         current_type = (array_type->get_element_type())->get_base_type();
      }while(is_a<ArrayType>(current_type));

      return current_type;
}

DataType* get_array_element_type(VariableSymbol *array_sym){

      DataType *current_type = (array_sym->get_type())->get_base_type();

      do{
         ArrayType *array_type = to<ArrayType>(current_type);
         current_type = (array_type->get_element_type())->get_base_type();
      }while(is_a<ArrayType>(current_type));

      return current_type;
}


DataType* get_array_element_type(ArrayType *a_type){

      DataType *current_type = a_type;

      do{
         ArrayType *array_type = to<ArrayType>(current_type);
         current_type = (array_type->get_element_type())->get_base_type();
      }while(is_a<ArrayType>(current_type));

      return current_type;
}

