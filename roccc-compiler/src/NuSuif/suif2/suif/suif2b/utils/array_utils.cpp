#include "array_utils.h"

#include <suifnodes/suif.h>
#include <basicnodes/basic.h>

ArrayTypeProxy::ArrayTypeProxy(DataType *type) : 
    _type(type),_sub_type(0) 
{
        if (is_kind_of<ArrayType>(_type)) {
            _dims = 1;
	_element_type = to<ArrayType>(_type)->get_element_type()->get_base_type();
            }
	else {
            _dims = to<MultiDimArrayType>(_type)->get_lower_bound_count();
	_element_type = to<MultiDimArrayType>(_type)->get_element_type()->get_base_type();
	}
	if (is_kind_of<ArrayType>(_element_type) || 
	is_kind_of<MultiDimArrayType>(_element_type))
	_sub_type = new ArrayTypeProxy(_element_type);
	_extra_dims = get_dimension_count() - _dims;
}

Expression *ArrayTypeProxy::get_lower_bound(int i) {
	if (i < _extra_dims)
	return _sub_type->get_lower_bound(i);
	
    i -= _extra_dims;
    if (is_kind_of<ArrayType>(_type)) {
        suif_assert_message(!i,("index not zero"));
        return to<ArrayType>(_type)->get_lower_bound();
    }
    return to<MultiDimArrayType>(_type)->get_lower_bound(i);
}

Expression *ArrayTypeProxy::get_upper_bound(int i) {
    if (i < _extra_dims)
        return _sub_type->get_upper_bound(i);
	i -= _extra_dims;

    if (is_kind_of<ArrayType>(_type)) {
        suif_assert_message(!i,("index not zero"));
        return to<ArrayType>(_type)->get_upper_bound();
    }
    return to<MultiDimArrayType>(_type)->get_upper_bound(i);
}

int ArrayTypeProxy::get_dimension_count() {
	if (_sub_type) 
	return _dims + _sub_type->get_dimension_count();
	return _dims;
}

bool ArrayTypeProxy::has_non_const_bounds() { // optimized for speed
	if (_sub_type && _sub_type->has_non_const_bounds())
	return true;
	if (is_kind_of<ArrayType>(_type)) {
	ArrayType *type = to<ArrayType>(_type);
	if (!is_kind_of<IntConstant>(type->get_lower_bound()))
		return true;
            if (!is_kind_of<IntConstant>(type->get_upper_bound()))
                return true;
	}
	else {
	MultiDimArrayType *type = to<MultiDimArrayType>(_type);
	int len = type->get_lower_bound_count();
	for (int i = 0; i < len; i++) {
		if (!is_kind_of<IntConstant>(type->get_lower_bound(i)))
                    return true;
                if (!is_kind_of<IntConstant>(type->get_upper_bound(i)))
                    return true;
		}
	}
	return false;
}

DataType* ArrayTypeProxy::get_element_type() {
	if (_sub_type)
		return _sub_type->get_element_type();
	return _element_type;
}

ArrayExpressionProxy::ArrayExpressionProxy(Expression *object) : 
    _object(object),_element_proxy(0) 
{
	if (is_kind_of<ArrayReferenceExpression>(_object)) {
	Expression *base = to<ArrayReferenceExpression>(_object)->get_base_array_address();
	if (is_kind_of<ArrayReferenceExpression>(base) ||
		is_kind_of<MultiDimArrayExpression>(base))
		_element_proxy = new ArrayExpressionProxy(base);
	}
        if (is_kind_of<MultiDimArrayExpression>(_object)) {
            Expression *base = to<MultiDimArrayExpression>(_object)->get_array_address();
            if (is_kind_of<ArrayReferenceExpression>(base) ||
                is_kind_of<MultiDimArrayExpression>(base))
                _element_proxy = new ArrayExpressionProxy(base);
            }
        if (is_kind_of<ArrayReferenceExpression>(_object)) {
            _dims = 1;
            }
	else {
            _dims = to<MultiDimArrayExpression>(_object)->get_index_count();
	}
	_extra_dims = get_dimension_count() - _dims;
}

ArrayExpressionProxy::~ArrayExpressionProxy() {
	delete _element_proxy;
}

int ArrayExpressionProxy::get_dimension_count() {
	if (_element_proxy)
	return _dims + _element_proxy->get_dimension_count();
	return _dims;
}

Expression* ArrayExpressionProxy::get_index(int i) {
	if (i < _extra_dims) {
		return _element_proxy->get_index(i);
	}
	i -= _extra_dims;
    if (is_kind_of<ArrayReferenceExpression>(_object)) {
        return to<ArrayReferenceExpression>(_object)->get_index();
    }
    return to<MultiDimArrayExpression>(_object)->get_index(i);
}

Expression* ArrayExpressionProxy::get_array_address() {
	if (_element_proxy)
	return _element_proxy->get_array_address();
        if (is_kind_of<ArrayReferenceExpression>(_object)) {
            return to<ArrayReferenceExpression>(_object)->get_base_array_address();
            }
        return to<MultiDimArrayExpression>(_object)->get_array_address();
}

DataType* ArrayExpressionProxy::get_array_type() {
	Expression *base;
	if (is_kind_of<ArrayReferenceExpression>(_object)) {
            base = to<ArrayReferenceExpression>(_object)->get_base_array_address();
            }
	else {
	base = to<MultiDimArrayExpression>(_object)->get_array_address();
	}
	DataType *type = base->get_result_type();
	if (is_kind_of<PointerType>(type)) {
	Type *ptype = to<PointerType>(type)->get_reference_type();
	type = to<QualifiedType>(ptype)->get_base_type();
	}
        if (is_kind_of<ReferenceType>(type)) {
            Type *ptype = to<ReferenceType>(type)->get_reference_type();
            type = to<QualifiedType>(ptype)->get_base_type();
            }

	return type;
}
