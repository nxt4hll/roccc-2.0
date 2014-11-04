// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef TYPE_UTILS_H
#define TYPE_UTILS_H

Type* get_base_type(Type* t);

DataType* get_array_element_type(ArrayReferenceExpression *a_ref);
DataType* get_array_element_type(SymbolAddressExpression *sym_address_expr);
DataType* get_array_element_type(VariableSymbol *array_sym);
DataType* get_array_element_type(ArrayType *a_type);

#endif

