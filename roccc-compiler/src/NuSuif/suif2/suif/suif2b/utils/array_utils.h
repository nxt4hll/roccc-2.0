#ifndef ARRAY_UTILS_H
#define ARRAY_UTILS_H

#include <suifnodes/suif_forwarders.h>
#include <basicnodes/basic_forwarders.h>

class ArrayTypeProxy {
    DataType *_type;
	DataType *_element_type;
 	ArrayTypeProxy *_sub_type;
	int _dims;
	int _extra_dims;
public:
	ArrayTypeProxy(DataType *type);

	Expression *get_lower_bound(int i);

    Expression *get_upper_bound(int i);

    int get_dimension_count();

	bool has_non_const_bounds();

    DataType *get_element_type();
};

class ArrayExpressionProxy {
    Expression *_object;
	ArrayExpressionProxy *_element_proxy;
	int _dims;
	int _extra_dims;
public:
	ArrayExpressionProxy(Expression *object);

	~ArrayExpressionProxy();

	int get_dimension_count();

	Expression *get_index(int i);

	Expression *get_array_address();

	DataType *get_array_type();
};

#endif /* ARRAY_UTILS_H */
