// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include "padding.h"

#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "iokernel/cast.h"
#include "suifkernel/group_walker.h"
#include "suifkernel/suif_object.h"
#include "iokernel/object_stream.h"
#include "common/suif_hash_map.h"
#include "typebuilder/type_builder.h"
#include "suifkernel/iter.h"
#include "common/suif_indexed_list.h"
#include "suifkernel/utilities.h"
#include "utils/expression_utils.h"
#include "utils/type_utils.h"

#include <stdio.h>

class Padding : public SelectiveWalker {
	TypeBuilder *type_builder;

	void rebuild_value_block(MultiValueBlock *block,ArrayType *type);
	void rebuild_value_block(MultiValueBlock *block,GroupType *type);
    public:
        Padding(SuifEnv *env)
                : SelectiveWalker(env,MultiValueBlock::get_class_name()),type_builder(get_type_builder(env)) {}

        ApplyStatus operator () (SuifObject *x);
        };

//	Disassemble a multi-value block and rebuild it to correspond to the 
//	fields of the associated type.
//
//	The C front end generates half-hearted multi-value blocks which do not
//	lay out correctly. However, s2c can only eat these half-hearted
//	multi-value blocks. We fix this in this pass. We need to right a
//	pass that can go back the other way as well.

Walker::ApplyStatus Padding::operator () (SuifObject *x) {
    MultiValueBlock *block = to<MultiValueBlock>(x);

    // find the type for the object this is initializing

    SuifObject *parent = block->get_parent();
    while (parent && !is_kind_of<VariableDefinition>(parent))
	parent = parent->get_parent();
    suif_assert_message(parent,("MultiValueBlock has no VariableDefinition parent"));
    VariableDefinition *def = to<VariableDefinition>(parent);
    DataType *def_type = get_data_type(def->get_variable_symbol());
    if (is_kind_of<ArrayType>(def_type)) {
	rebuild_value_block(block,to<ArrayType>(def_type));
	}
    else {
    	rebuild_value_block(block,to<GroupType>(def_type));
	}

    // rebuild_value_block recursively handles sub multi_value blocks, so 
    // Truncate the walk
    return Walker::Truncate;
    }

static bool is_big_endian() {
    int i = 1;
    char *x = (char *)&i;
    return (*x == 0);
    }

void Padding::rebuild_value_block(MultiValueBlock *block,ArrayType *type) {
    Iter<MultiValueBlock::sub_block_pair>  iter = block->get_sub_block_iterator();
    int bit_offset = 0;
    IntegerType *char_type = type_builder->get_smallest_integer_type();
    int bit_size = char_type->get_bit_size().c_int();
    indexed_list<IInteger,ValueBlock* > temp;
    bool changed = false;

    // In order to get the fields in order by offset, this code builds a
    // second list and the replaces the first list with it in an
    // inefficient way. This could run faster if we exposed more in the
    // interface.

    DataType *element_type = to<QualifiedType>(type->get_element_type())->get_base_type();
    int count = 0;
    
    while (iter.is_valid()) {
	MultiValueBlock::sub_block_pair &pair = iter.current();

	// if this is a MultiValueBlock, check it too

	if (is_kind_of<MultiValueBlock>(pair.second)) {
	    if (is_kind_of<ArrayType>(element_type)) {
	        rebuild_value_block(to<MultiValueBlock>(pair.second),to<ArrayType>(element_type));
		}
	    else {
		rebuild_value_block(to<MultiValueBlock>(pair.second),to<GroupType>(element_type));
		}
	    }

	// we valiantly assume that there are no gaps between array elements that
	// need to be filled in. This is probably not true in general but at present
 	// we don't seem to have a way to detect this situation; we would need a span
	// value for the array type. 

	bit_offset += element_type->get_bit_size().c_int();
	count ++;
	iter.next();
	}
    IInteger offset = type->get_bit_size();
    if (offset > bit_offset) {
        int rep = (offset.c_int() - bit_offset) / bit_size;
        //int pad = rep * bit_size;
        UndefinedValueBlock* pad_block = create_undefined_value_block(get_env(),char_type);
        temp.push_back(bit_offset,create_repeat_value_block(get_env(),rep,pad_block,char_type));
        changed = true;
        }

    if (changed) {
	indexed_list<IInteger,ValueBlock* >::iterator iter = temp.begin();
	while (iter != temp.end()) {
	    block->remove_all_from_sub_block((*iter).second);
	    iter ++;
	    }
        iter = temp.begin();
        while (iter != temp.end()) {
	    (*iter).second->set_parent(0); // shoulw have happened in the remove_all above
            block->add_sub_block((*iter).first,(*iter).second);
            iter ++;
            }
	}
    }

static bool has_bit_field_info(FieldSymbol *field,int bit_field_no,int &bit_size,
		int &bit_offset,bool &anonymous) {
    static const LString bitfield("bitfield");
    Annote * bit_field_annote = field->lookup_annote_by_name(bitfield,bit_field_no + 1);
    if (!bit_field_annote)
	return false;
    BrickAnnote *bit_annote = to<BrickAnnote>(bit_field_annote);
    SuifBrick *offset_brick = bit_annote->get_brick(0);
    SuifBrick *size_brick = bit_annote->get_brick(1);
    SuifBrick *name_brick = bit_annote->get_brick(2);
    bit_offset = to<IntegerBrick>(offset_brick)->get_value().c_int();
    bit_size = to<IntegerBrick>(size_brick)->get_value().c_int();
    anonymous = to<StringBrick>(name_brick)->get_value() == emptyLString;
    return true;
    }

static int mask[8] = {1,3,7,15,31,63,127,255};

//	A class to hold bit field values as bytes of data
class bit_field_buffer {
    	simple_stack<unsigned char> _buffer;
	int _offset;	// in bytes 
	int _bit_offset; // _offset in bits
    public:
	bit_field_buffer() : _offset(-1) {}

	int get_byte_count() {return _buffer.len();}
	int get_byte(int n) {return _buffer[n];}

	int get_offset() {return _offset;}
	
	// put data of given value in a bit field at the
	// given offset and size
	void put_data(long value,int bit_offset,int bit_size) {
	    if (_offset < 0) { // first value 
		_offset = bit_offset / 8;
		_bit_offset = _offset * 8;
		}
	    else {
		int i = bit_offset / 8;
		if (_offset > i) { // need to shuffle up
		    int l = _buffer.len() - 1;
		    for (int j = i;j < _offset; j++)
			_buffer.push(0);
		    int k = _buffer.len() - 1;
		    while (l >= 0) {
			_buffer[k] = _buffer[l];
			k--;
			l--;
			}
		    while (k >= 0) {
			_buffer[k] = 0;
			k--;
			}
		    _offset = i;
		    _bit_offset = _offset * 8;
		    }
		}
	    int last_pos = (bit_offset - _bit_offset + bit_size + 7) / 8;
	    while (_buffer.len() < last_pos)
		_buffer.push(0);

	    // The buffer is now large enough for the new value, so shift it a
	    // byte at a time and insert it

	    if (is_big_endian()) {
		int next_pos = last_pos - 1; // change from length to index
		int last_bit = (bit_offset + bit_size - 1) % 8; // no of bits in last byte , less 1
		int field_mask = mask[last_bit];
		if (bit_size < last_bit)
		    field_mask = mask[bit_size - 1];
		field_mask = field_mask << (7 - last_bit);
		_buffer[next_pos] &= ~field_mask; // clear bits of field
		_buffer[next_pos] |= (unsigned char) ((value << (7 - last_bit)) & field_mask);
		next_pos --;
		bit_size -= (last_bit + 1);
		value = value >> (last_bit + 1);
		while (bit_size >= 8) {
		    _buffer[next_pos] = (unsigned char) value;
		    next_pos --;
		    value = value >> 8;
		    bit_size -= 8;
		    }
		if (bit_size > 0) {
		    _buffer[next_pos] &= ~mask[bit_size - 1];
		    _buffer[next_pos] |= (unsigned char) (value & mask[bit_size - 1]);
		    }
		}
	    else {
		int next_pos = (bit_offset - _bit_offset) / 8; // index of first byte to modify
		int last_bit = 7 - bit_offset % 8; // no of bits in first byte , less 1
                int field_mask = mask[last_bit];
                if (bit_size < last_bit)
                    field_mask = mask[bit_size - 1];
                field_mask = field_mask << (7 - last_bit);
                _buffer[next_pos] &= ~field_mask; // clear bits of field
                _buffer[next_pos] |= (unsigned char) ((value << (7 - last_bit)) & field_mask);
		next_pos ++;
		bit_size -= (last_bit + 1);
                value = value >> (last_bit + 1);
                while (bit_size >= 8) {
                    _buffer[next_pos] = (unsigned char) value;
                    next_pos ++;
                    value = value >> 8;
                    bit_size -= 8;
                    }
                if (bit_size > 0) {
                    _buffer[next_pos] &= ~mask[bit_size - 1];
                    _buffer[next_pos] |= (unsigned char) (value & mask[bit_size - 1]);
                    }
		}
	    }
	};

void Padding::rebuild_value_block(MultiValueBlock *block,GroupType *type) {

    Iter<MultiValueBlock::sub_block_pair>  iter = block->get_sub_block_iterator();
    int offset_in_bits = 0;
    IntegerType *char_type = type_builder->get_smallest_integer_type();
    int char_bit_size = char_type->get_bit_size().c_int();
    indexed_list<IInteger,ValueBlock* > temp;
    bool changed = false;

    // In order to get the fields in order by offset, this code builds a
    // second list and the replaces the first list with it in an
    // inefficient way. This could run faster if we exposed more in the
    // interface.

    // There may be bit fields in the data. These
    // complicate matters because there is only one field symbol for
    // multiple bit fields. 

    GroupSymbolTable *symbol_table = type->get_group_symbol_table();

    // Prime the pump with the first field or bit field

    int field_count = symbol_table->get_symbol_table_object_count();

    for (int field_no = 0; field_no < field_count; field_no ++) {
	if (!iter.is_valid())
	    break; // not all fields initialized
    	FieldSymbol *field = to<FieldSymbol>(symbol_table->get_symbol_table_object(field_no));
    	DataType *element_type = get_data_type(field);
    	int bit_size;
    	int bit_offset;
	bool anonymous;
    	bool has_bit_fields = has_bit_field_info(field,0,bit_size,bit_offset,anonymous);

	if (has_bit_fields) {	
	    // process all the bit fields
	    // there will be one iter entry for each bit field
	    bit_field_buffer bit_field_values;
	    int bitfield_pos = 0;
	    while (has_bit_fields) {
		if (!iter.is_valid())
		    break;
		while (has_bit_fields && anonymous) {
		    bitfield_pos ++;
		    has_bit_fields = has_bit_field_info(field,bitfield_pos,bit_size,bit_offset,anonymous);
		    }
		if (!has_bit_fields)
		    break;
		MultiValueBlock::sub_block_pair &pair = iter.current();
		Expression *exp = to<ExpressionValueBlock>(pair.second)->get_expression();
		IInteger value = to<IntConstant>(exp)->get_value();
		bit_field_values.put_data(value.c_long(),bit_offset,bit_size);
		bitfield_pos ++;
		has_bit_fields = has_bit_field_info(field,bitfield_pos,bit_size,bit_offset,anonymous);
		iter.next();
		}
	    int bytes = bit_field_values.get_byte_count();
	    int offset = bit_field_values.get_offset() * char_bit_size;
	    if ((bytes > 0) && (offset_in_bits < offset)) {
        	int rep = (offset - offset_in_bits) / char_bit_size;
        	UndefinedValueBlock* pad_block = create_undefined_value_block(get_env(),char_type);
        	temp.push_back(offset_in_bits,create_repeat_value_block(get_env(),rep,pad_block,char_type));
        	changed = true;
		offset_in_bits = offset;
		}
	    for (int i =0; i < bytes; i++) {
                temp.push_back(offset_in_bits,
                        create_expression_value_block(get_env(),
                                create_int_constant(	get_env(),
							char_type,
							bit_field_values.get_byte(i))));
                offset_in_bits += 8;
		changed = true;
		}
	    }
	else {
	    MultiValueBlock::sub_block_pair &pair = iter.current();

	    //	we now have a field and a value block. 

	    // if this is a MultiValueBlock, check it too

            if (is_kind_of<MultiValueBlock>(pair.second)) {
            	if (is_kind_of<ArrayType>(element_type)) {
            	    rebuild_value_block(
			to<MultiValueBlock>(pair.second),
			to<ArrayType>(element_type));
            	    }
            	else {
                    rebuild_value_block(
			to<MultiValueBlock>(pair.second),
			to<GroupType>(element_type));
		    }
		}


	    // We now have an type and a value to go with it. There can be gaps
	    // between fields, which need padding out, or there can be overlaps
	    // due to the presence of bit fields packed in with other 
	    // elements. 
	    // If there is overlap, we trim the overlapped part and output as
	    // bytes.
 	    // If there are gaps, we load out the gap with bytes of unknown

	    int offset = pair.first.c_int();
	    if (offset > offset_in_bits) {
	        int rep = (offset - offset_in_bits) / char_bit_size;
	        UndefinedValueBlock* pad_block = create_undefined_value_block(get_env(),char_type);
	        temp.push_back(offset_in_bits,create_repeat_value_block(get_env(),rep,pad_block,char_type));
	        changed = true;
	        offset_in_bits += rep * char_bit_size;
	        suif_assert_message(offset_in_bits == offset,("Offset of entry in MultiuValueBlock is not multiple of char size"));
	        }
	    temp.push_back(offset,pair.second);
	    offset_in_bits += element_type->get_bit_size().c_int();
	    iter.next();
	    }
	}

    // finally, pad out the rest of the structure to the size of the type
    IInteger offset = type->get_bit_size();
    if (offset > offset_in_bits) {
        int rep = (offset.c_int() - offset_in_bits) / char_bit_size;
        UndefinedValueBlock* pad_block = create_undefined_value_block(get_env(),char_type);
        temp.push_back(offset_in_bits,create_repeat_value_block(get_env(),rep,pad_block,char_type));
        changed = true;
        }

    if (changed) {
	block->clear_sub_block_list();
        indexed_list<IInteger,ValueBlock* >::iterator iter = temp.begin();
        while (iter != temp.end()) {
	    (*iter).second->set_parent(0); // shoulw have happened in the remove_all above
            block->add_sub_block((*iter).first,(*iter).second);
            iter ++;
            }
	}
	
    }

PaddingPass::PaddingPass(SuifEnv *pEnv, const LString &name) :
        Pass(pEnv, name), 
	_virs_flag_argument(0), _virs_selector(0), virs(false) {
}

void PaddingPass::do_file_set_block(FileSetBlock *pFSB) {
        Padding walker(get_suif_env());
        pFSB->walk(walker);
}


void PaddingPass::initialize() {
    Pass::initialize();

    _virs_flag_argument = new OptionLiteral("-virs", &virs, true);
    _virs_selector = new OptionSelection(true);
    _virs_selector->add(_virs_flag_argument);
    _command_line->add(_virs_selector);
}

StructPaddingPass::StructPaddingPass(SuifEnv *pEnv, const LString &name) :
  Pass(pEnv, name)
{
}

void StructPaddingPass::do_file_set_block(FileSetBlock *fsb) {

  TargetInformationBlock *tinfo = 
    find_target_information_block(get_suif_env());

  if (tinfo == NULL) {
    suif_warning("Skipping StructPaddingPass because Target Info Block is unavailable");
    return;
  }

  for (Iter<StructType> iter = object_iterator<StructType>(fsb);
       iter.is_valid(); iter.next()) {
    StructType *s_t = &iter.current();
    IInteger struct_size = s_t->get_bit_size();

    // Just check the last field
    if (!s_t->get_is_complete()) continue;
    GroupSymbolTable *gst = s_t->get_group_symbol_table();
    int size = gst->get_symbol_table_object_count();
    if (size == 0) continue;
    FieldSymbol *fs = to<FieldSymbol>(gst->get_symbol_table_object(size-1));
    suif_assert(fs != NULL);

    IInteger field_offset = get_expression_constant(fs->get_bit_offset());
    IInteger field_size = get_data_type(fs)->get_bit_size();
    
    IInteger next_bit = field_offset + field_size;
    suif_assert_message(struct_size >= next_bit,
			("struct size(%s) < next_bit(%s)", 
			 struct_size.to_String().c_str(),
			 next_bit.to_String().c_str()));

    IInteger diff = struct_size - next_bit;
    if (diff == 0) continue;

    // try the following sizes
    // word, char, bit

    DataType *word_type = tinfo->get_word_type();
    IInteger word_size = word_type->get_bit_size();
    IInteger byte_size = tinfo->get_byte_size();
    IInteger int_size;
    if (diff >= word_size) {
      int_size = word_size;
    } else if (diff >= byte_size) {
      int_size = byte_size;
    } else {
      int_size = 1;
    }
    TypeBuilder *tb = get_type_builder(get_suif_env());
    QualifiedType *field_type = 
      tb->get_qualified_type(tb->get_integer_type(int_size, int_size.c_int(),
						  false));
    FieldSymbol *new_field = 
      create_field_symbol(get_suif_env(), field_type,
			  create_int_constant(get_suif_env(), 
					      struct_size - int_size));

    gst->append_symbol_table_object(new_field);
  }
}      
