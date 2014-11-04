/* file "bit_vector.cpp" */


/*
       Copyright (c) 1997-1999 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>


/*
      This is the implementation of the bit_vector class for sty, the
      first-level main library of the SUIF system.
*/


// This is now defined in machine_dependent.h
#define SIZE_T_MAX ((sizeof(size_t) == sizeof(unsigned long)) ? ULONG_MAX : \
		    UINT_MAX)


#include "bit_vector.h"
#include <suifkernel/suifkernel_messages.h>
#include <string.h>


#define BITS_PER_WORD (sizeof(BitVector::ChunkT) * 8)
#define BITS_PER_CHAR 8

class BitVectorBlock
  {
    friend class BitVector;

private:
    BitVector::ChunkT *_data;
    size_t _data_length;
    size_t _data_space;
    bool _infinity_bit_is_one;
    size_t _num_significant_bits;
    size_t _reference_count;

    BitVectorBlock(size_t init_data_space,
                     bool init_infinity_bit_is_one,
                     size_t init_num_significant_bits);
    ~BitVectorBlock(void);

    void expand_data(size_t new_data_length);
    void unpad(void);
    BitVectorBlock(const BitVectorBlock &);
    BitVectorBlock &operator=(const BitVectorBlock &);
  };


BitVectorBlock *BitVector::_zero_block;
BitVectorBlock *BitVector::_ones_block;

BitVectorBlock::BitVectorBlock(size_t init_data_space,
        bool init_infinity_bit_is_one, size_t init_num_significant_bits)
  {
    if (init_data_space == 0)
        _data = NULL;
    else
        _data = new BitVector::ChunkT[init_data_space];
    _data_length = init_data_space;
    _data_space = init_data_space;
    _infinity_bit_is_one = init_infinity_bit_is_one;
    _num_significant_bits = init_num_significant_bits;
    _reference_count = 1;
  }

BitVectorBlock::~BitVectorBlock(void)
  {
    assert(_reference_count == 0);
    if (_data != NULL)
        delete[] _data;
  }

void BitVectorBlock::expand_data(size_t new_data_length)
  {
    suif_assert(new_data_length >= 0);
    if (new_data_length <= _data_length)
        return;
    if (new_data_length > _data_space)
      {
        BitVector::ChunkT *new_data = new BitVector::ChunkT[new_data_length];
        for (size_t chunk_num = 0; chunk_num < _data_length; ++chunk_num)
            new_data[chunk_num] = _data[chunk_num];
        if (_data != NULL)
            delete[] _data;
        _data = new_data;
        _data_space = new_data_length;
      }
    BitVector::ChunkT mask = 0;
    if (_infinity_bit_is_one)
        mask = ~0;
    for (size_t chunk_num = _data_length; chunk_num < new_data_length;
         ++chunk_num)
      {
        _data[chunk_num] = mask;
      }
    _data_length = new_data_length;
  }

void BitVectorBlock::unpad(void)
  {
    BitVector::ChunkT mask = 0;
    if (_infinity_bit_is_one)
        mask = ~0;
    while (_data_length > 0)
      {
        if (_data[_data_length - 1] != mask)
            break;
        --_data_length;
      }
    _num_significant_bits = _data_length * BITS_PER_WORD;
    if (_data_length == 0)
        return;
    mask = ((BitVector::ChunkT)1) << (BITS_PER_WORD - 1);
    BitVector::ChunkT test_block = _data[_data_length - 1];
    if (_infinity_bit_is_one)
        test_block = ~test_block;
    while ((mask & test_block) == 0)
      {
        mask >>= 1;
        assert(mask != 0);
        --_num_significant_bits;
      }
  }



void BitVector::do_initialization(void)
  {
    _zero_block = new BitVectorBlock(0, false, 0);
    _ones_block = new BitVectorBlock(0, true, 0);
  }

void BitVector::add_reference(BitVectorBlock *the_block)
  {
    if (the_block->_reference_count != SIZE_T_MAX)
        ++(the_block->_reference_count);
  }

void BitVector::remove_reference(BitVectorBlock *the_block)
  {
    assert(the_block->_reference_count > 0);
    if (the_block->_reference_count != SIZE_T_MAX)
      {
        --(the_block->_reference_count);
        if (the_block->_reference_count == 0)
            delete the_block;
      }
  }

void BitVector::make_changable(void)
  {
    if (_block->_reference_count > 1)
      {
        BitVectorBlock *new_block =
                new BitVectorBlock(_block->_data_length,
                                     _block->_infinity_bit_is_one,
                                     _block->_num_significant_bits);
        for (size_t chunk_num = 0; chunk_num < _block->_data_length;
             ++chunk_num)
          {
            new_block->_data[chunk_num] = _block->_data[chunk_num];
          }
        remove_reference(_block);
        _block = new_block;
      }
    _has_count = false;
  }

unsigned char BitVector::convert_from_hex(char hex_char)
  {
    if ((hex_char >= '0') && (hex_char <= '9'))
        return hex_char - '0';
    else if ((hex_char >= 'a') && (hex_char <= 'f'))
        return (hex_char - 'a') + 10;
    else if ((hex_char >= 'A') && (hex_char <= 'F'))
        return (hex_char - 'A') + 10;
    else
        return 16;
  }

BitVector::BitVector(void) :
  _block(_zero_block),
  _count(0),
  _has_count(true)
{
  add_reference(_zero_block);
}

BitVector::BitVector(const BitVector &other) :
  _block(other._block),
  _count(other._count),
  _has_count(other._has_count)
{
  add_reference(_block);
}

BitVector::~BitVector(void)
  {
    remove_reference(_block);
  }

bool BitVector::get_bit(size_t bit_num) const
  {
    suif_assert(bit_num >= 0);
    size_t chunk_num = bit_num / (size_t)BITS_PER_WORD;
    if (chunk_num >= _block->_data_length)
        return _block->_infinity_bit_is_one;
    BitVector::ChunkT mask =
            ((BitVector::ChunkT)1) << (bit_num % (size_t)BITS_PER_WORD);
    return (((_block->_data[chunk_num] & mask) == 0) ? false : true);
  }

bool BitVector::get_infinity_bit(void) const
  {
    return _block->_infinity_bit_is_one;
  }

size_t BitVector::num_significant_bits(void) const
  {
    return _block->_num_significant_bits;
  }
size_t BitVector::get_chunk_count() const {
  size_t chunk_count =
    ((num_significant_bits() + (BITS_PER_WORD) - 1) / BITS_PER_WORD);
  return(chunk_count);
}

BitVector::ChunkT BitVector::get_chunk(size_t chunk_num) const
  {
    suif_assert(chunk_num >= 0);
    if (chunk_num >= _block->_data_length)
      {
        return (_block->_infinity_bit_is_one ? ~(BitVector::ChunkT)0 :
                (BitVector::ChunkT)0);
      }
    return _block->_data[chunk_num];
  }

void BitVector::set_bit(size_t bit_num, bool new_value)
  {
    suif_assert(bit_num >= 0);
    if ((bit_num >= _block->_num_significant_bits) &&
        (new_value == _block->_infinity_bit_is_one))
      {
        return;
      }
    make_changable();
    size_t chunk_num = bit_num / (size_t)BITS_PER_WORD;
    if (chunk_num >= _block->_data_length)
        _block->expand_data(chunk_num + 1);
    BitVector::ChunkT mask =
            ((BitVector::ChunkT)1) << (bit_num % (size_t)BITS_PER_WORD);
    if (new_value)
        _block->_data[chunk_num] |= mask;
    else
        _block->_data[chunk_num] &= ~mask;
    if (bit_num >= _block->_num_significant_bits)
        _block->_num_significant_bits = bit_num + 1;
    _block->unpad();
  }

void BitVector::set_chunk(size_t chunk_num, BitVector::ChunkT new_chunk)
  {
    suif_assert(chunk_num >= 0);
    make_changable();
    if (chunk_num >= _block->_data_length)
        _block->expand_data(chunk_num + 1);
    _block->_data[chunk_num] = new_chunk;
    size_t bit_num = (chunk_num + 1) * (size_t)BITS_PER_WORD;
    if (bit_num > _block->_num_significant_bits)
        _block->_num_significant_bits = bit_num;
    _block->unpad();
  }

void BitVector::set_to_zero(void)
  {
    remove_reference(_block);
    _block = _zero_block;
    add_reference(_block);
   _has_count = true;
   _count = 0;
  }

void BitVector::set_to_ones(void)
  {
    _has_count = true;
    _count = 0;
    remove_reference(_block);
    _block = _ones_block;
    add_reference(_block);
  }

void BitVector::operator=(const BitVector &other)
  {
    if (_block != other._block)
      {
        remove_reference(_block);
        _block = other._block;
        add_reference(_block);
	_has_count = other._has_count;
	_count = other._count;
      }
  }

String BitVector::to_string(void) const
  {
    static char *buffer = NULL;
    static size_t buffer_size = 0;

    size_t new_size = written_length() + 1;
    if (new_size > buffer_size)
      {
        if (buffer != NULL)
            delete[] buffer;
        buffer = new char[new_size];
        buffer_size = new_size;
      }
    write(buffer);
    return String(buffer);
  }

void BitVector::from_string(String new_value)
  {
    read(new_value.c_str());
  }

IInteger BitVector::to_i_integer(void) const
  {
    size_t length = _block->_data_length;
    IInteger result = (_block->_infinity_bit_is_one ? -1 : 0);
    for (size_t chunk_num = length; chunk_num > 0; --chunk_num)
      {
        result = ((result << BITS_PER_WORD) |
                  _block->_data[chunk_num - 1]);
      }
    return result;
  }

void BitVector::from_i_integer(IInteger new_value)
  {
    if (new_value < 0)
      {
        from_i_integer(~new_value);
        *this = invert();
        return;
      }
    IInteger ii_hex_length = new_value.written_length(16);
    if (!ii_hex_length.is_c_size_t())
        suif_error("out of memory address space");
    size_t hex_length = ii_hex_length.c_size_t();
    char *buffer = new char[hex_length + 3];
    buffer[0] = '0';
    buffer[1] = 'x';
    new_value.write(&(buffer[2]), 16);
    read(buffer);
    delete[] buffer;
  }

size_t BitVector::written_length(void) const
  {
    return 7 + (_block->_data_length * sizeof(BitVector::ChunkT) * 2);
  }

void BitVector::write(char *location) const
  {
    size_t length = _block->_data_length;
    sprintf(location, "0x...%02x",
            (_block->_infinity_bit_is_one ? 0xff : 0x00));
    char *position = &(location[7]);
    for (size_t chunk_num = length; chunk_num > 0; --chunk_num)
      {
        sprintf(position, "%0*lx", (int)(sizeof(BitVector::ChunkT)) * 2,
                _block->_data[chunk_num - 1]);
        position += sizeof(BitVector::ChunkT) * 2;
      }
  }

void BitVector::read(const char *location)
  {
    _has_count = false;
    _count = 0;
    const char *follow = location;
    suif_assert((*follow == '0') && (follow[1] == 'x'));
    follow += 2;
    bool sign_extend = (*follow == '.');
    while (*follow == '.')
        ++follow;
    const char *end = follow;
    while (*end != 0)
        ++end;
    suif_assert(end > follow);
    remove_reference(_block);
    size_t hex_digits_per_word = sizeof(BitVector::ChunkT) * 2;
    bool infinity_is_one =
            (sign_extend ? ((*follow > '7') || (*follow < '0')) : false);
    size_t word_count =
            ((end - follow) + hex_digits_per_word - 1) / hex_digits_per_word;
    _block = new BitVectorBlock(word_count, infinity_is_one, word_count);
    size_t word_num = word_count;
    if (((end - follow) % hex_digits_per_word) > 0)
      {
        BitVector::ChunkT high_word = (infinity_is_one ? ~0lu : 0lu);
        while (((end - follow) % hex_digits_per_word) > 0)
          {
            unsigned char new_half_byte = convert_from_hex(*follow);
            suif_assert(new_half_byte != 16);
            high_word = (high_word << 4) | new_half_byte;
            ++follow;
          }
        --word_num;
        _block->_data[word_num] = high_word;
      }
    while (follow < end)
      {
        BitVector::ChunkT this_word = 0;
        for (size_t digit_num = 0; digit_num < hex_digits_per_word;
             ++digit_num)
          {
            unsigned char new_half_byte = convert_from_hex(*follow);
            suif_assert(new_half_byte != 16);
            this_word = (this_word << 4) | new_half_byte;
            ++follow;
          }
        --word_num;
        _block->_data[word_num] = this_word;
      }
    assert(follow == end);
    assert(word_num == 0);
    _block->unpad();
  }

void BitVector::print(FILE *fp) const
  {
    file_ion the_ion(fp);
    print(&the_ion);
  }

void BitVector::print(ion *the_ion) const
  {
    size_t length = _block->_data_length;
    the_ion->printf("0x...%02x", (_block->_infinity_bit_is_one ? 0xff : 0x00));
    for (size_t chunk_num = length; chunk_num > 0; --chunk_num)
      {
        the_ion->printf("%0*lx", (int)(sizeof(BitVector::ChunkT)) * 2,
                        _block->_data[chunk_num - 1]);
      }
  }

bool BitVector::is_equal_to(const BitVector &other) const
  {
    if (_block == other._block)
        return true;
    if (_block->_infinity_bit_is_one != other._block->_infinity_bit_is_one)
        return false;
    if (_block->_num_significant_bits != other._block->_num_significant_bits)
        return false;
    size_t length = _block->_data_length;
    assert(length == other._block->_data_length);
    for (size_t chunk_num = 0; chunk_num < length; ++chunk_num)
      {
        if (_block->_data[chunk_num] != other._block->_data[chunk_num])
            return false;
      }
    return true;
  }

bool BitVector::is_less_than(const BitVector &other) const
  {
    if (_block == other._block)
        return false;
    if (_block->_infinity_bit_is_one && (!other._block->_infinity_bit_is_one))
        return true;
    if ((!_block->_infinity_bit_is_one) && other._block->_infinity_bit_is_one)
        return false;
    if (_block->_num_significant_bits > other._block->_num_significant_bits)
        return _block->_infinity_bit_is_one;
    if (_block->_num_significant_bits < other._block->_num_significant_bits)
        return !(_block->_infinity_bit_is_one);
    size_t length = _block->_data_length;
    assert(length == other._block->_data_length);
    size_t chunk_num = length;
    while (chunk_num > 0)
      {
        --chunk_num;
        if (_block->_data[chunk_num] > other._block->_data[chunk_num])
            return _block->_infinity_bit_is_one;
        if (_block->_data[chunk_num] < other._block->_data[chunk_num])
            return !(_block->_infinity_bit_is_one);
      }
    return false;
  }

BitVector BitVector::invert(void) const
  {
    BitVector result(*this);
    result.make_changable();
    size_t length = _block->_data_length;
    for (size_t chunk_num = 0; chunk_num < length; ++chunk_num)
        result._block->_data[chunk_num] = ~(_block->_data[chunk_num]);
    result._block->_infinity_bit_is_one = !(_block->_infinity_bit_is_one);
    return result;
  }

BitVector BitVector::operator^(const BitVector &other) const
  {
    BitVector result = *this;
    result ^= other;
    return result;
  }

BitVector BitVector::operator&(const BitVector &other) const
  {
    BitVector result = *this;
    result &= other;
    return result;
  }

BitVector BitVector::operator|(const BitVector &other) const
  {
    BitVector result = *this;
    result |= other;
    return result;
  }

BitVector BitVector::operator~(void) const
  {
    return invert();
  }

BitVector BitVector::operator<<(size_t shift_amount) const
  {
    BitVector result = *this;
    result <<= shift_amount;
    return result;
  }

BitVector BitVector::operator>>(size_t shift_amount) const
  {
    BitVector result = *this;
    result >>= shift_amount;
    return result;
  }

bool BitVector::operator!(void) const
  {
    return ((_block->_data_length == 0) && (!_block->_infinity_bit_is_one));
  }

void BitVector::operator^=(const BitVector &other)
  {
    make_changable();
    size_t length = _block->_data_length;
    size_t other_length = other._block->_data_length;
    size_t common_length =
            ((other_length > length) ? length : other_length);
    for (size_t chunk_num = 0; chunk_num < common_length; ++chunk_num)
        _block->_data[chunk_num] ^= other._block->_data[chunk_num];
    if (other_length > length)
      {
        _block->expand_data(other_length);
        if (_block->_infinity_bit_is_one)
          {
            for (size_t chunk_num = length; chunk_num < other_length;
                 ++chunk_num)
              {
                _block->_data[chunk_num] = ~(other._block->_data[chunk_num]);
              }
          }
        else
          {
            for (size_t chunk_num = length; chunk_num < other_length;
                 ++chunk_num)
              {
                _block->_data[chunk_num] = other._block->_data[chunk_num];
              }
          }
      }
    else
      {
        if (other._block->_infinity_bit_is_one)
          {
            for (size_t chunk_num = other_length; chunk_num < length;
                 ++chunk_num)
              {
                 _block->_data[chunk_num] = ~(_block->_data[chunk_num]);
              }
          }
      }
    _block->_infinity_bit_is_one =
            ((_block->_infinity_bit_is_one ? 1 : 0) ^
             (other._block->_infinity_bit_is_one ? 1 : 0) == 1);
    if (other._block->_num_significant_bits > _block->_num_significant_bits)
        _block->_num_significant_bits = other._block->_num_significant_bits;
    _block->unpad();
  }

void BitVector::operator&=(const BitVector &other)
  {
    make_changable();
    size_t length = _block->_data_length;
    size_t other_length = other._block->_data_length;
    size_t common_length =
            ((other_length > length) ? length : other_length);
    for (size_t chunk_num = 0; chunk_num < common_length; ++chunk_num)
        _block->_data[chunk_num] &= other._block->_data[chunk_num];
    if (other_length > length)
      {
        if (_block->_infinity_bit_is_one)
          {
            _block->expand_data(other_length);
            for (size_t chunk_num = length; chunk_num < other_length;
                 ++chunk_num)
              {
                _block->_data[chunk_num] = other._block->_data[chunk_num];
              }
            _block->_num_significant_bits =
                    other._block->_num_significant_bits;
          }
      }
    else if (other_length < length)
      {
        if (!other._block->_infinity_bit_is_one)
          {
            _block->_data_length = other_length;
            _block->_num_significant_bits =
                    other._block->_num_significant_bits;
          }
      }
    else
      {
        if (other._block->_num_significant_bits >
            _block->_num_significant_bits)
          {
            _block->_num_significant_bits =
                    other._block->_num_significant_bits;
          }
      }
    _block->_infinity_bit_is_one =
            ((_block->_infinity_bit_is_one ? 1 : 0) &
             (other._block->_infinity_bit_is_one ? 1 : 0) == 1);
    _block->unpad();
  }

void BitVector::operator|=(const BitVector &other)
  {
    make_changable();
    size_t length = _block->_data_length;
    size_t other_length = other._block->_data_length;
    size_t common_length =
            ((other_length > length) ? length : other_length);
    for (size_t chunk_num = 0; chunk_num < common_length; ++chunk_num)
        _block->_data[chunk_num] |= other._block->_data[chunk_num];
    if (other_length > length)
      {
        if (!_block->_infinity_bit_is_one)
          {
            _block->expand_data(other_length);
            for (size_t chunk_num = length; chunk_num < other_length;
                 ++chunk_num)
              {
                _block->_data[chunk_num] = other._block->_data[chunk_num];
              }
            _block->_num_significant_bits =
                    other._block->_num_significant_bits;
          }
      }
    else if (other_length < length)
      {
        if (other._block->_infinity_bit_is_one)
          {
            _block->_data_length = other_length;
            _block->_num_significant_bits =
                    other._block->_num_significant_bits;
          }
      }
    else
      {
        if (other._block->_num_significant_bits >
            _block->_num_significant_bits)
          {
            _block->_num_significant_bits =
                    other._block->_num_significant_bits;
          }
      }
    _block->_infinity_bit_is_one =
            ((_block->_infinity_bit_is_one ? 1 : 0) |
             (other._block->_infinity_bit_is_one ? 1 : 0) == 1);
    _block->unpad();
  }

void BitVector::operator>>=(size_t shift_amount)
  {
    if (shift_amount < 0)
      {
        (*this) <<= -shift_amount;
        return;
      }
    if (_block->_num_significant_bits == 0)
        return;
    if (_block->_num_significant_bits <= shift_amount)
      {
        if (_block->_infinity_bit_is_one)
            set_to_ones();
        else
            set_to_zero();
        return;
      }
    make_changable();
    size_t orig_significant_bits = _block->_num_significant_bits;
    size_t new_significant_bits = orig_significant_bits - shift_amount;
    _block->_num_significant_bits = new_significant_bits;
    size_t length = _block->_data_length;
    size_t shift_div = shift_amount / BITS_PER_WORD;
    size_t shift_mod = shift_amount % BITS_PER_WORD;
    size_t new_length =
            (new_significant_bits + (BITS_PER_WORD - 1)) / BITS_PER_WORD;
    if (shift_amount % 8 == 0)
      {
        memmove((char *)(_block->_data),
                ((char *)(_block->_data)) + (shift_amount / 8),
                (new_significant_bits + 7) / 8);
        size_t leftover_bytes_written =
                ((((new_significant_bits + 7) / 8)) % sizeof(BitVector::ChunkT));
        if (leftover_bytes_written != 0)
          {
            if (_block->_infinity_bit_is_one)
              {
                _block->_data[new_length - 1] |=
                        ((~0ul) << (leftover_bytes_written * 8));
              }
            else
              {
                _block->_data[new_length - 1] &=
                        ~((~0ul) << (leftover_bytes_written * 8));
              }
          }
      }
    else
      {
        BitVector::ChunkT carry_bits = (_block->_data[shift_div] >> shift_mod);
        for (size_t chunk_num = 0; chunk_num < new_length - 1; --chunk_num)
          {
            BitVector::ChunkT word_in = _block->_data[chunk_num + shift_div + 1];
            BitVector::ChunkT carry_out = (word_in >> shift_mod);
            BitVector::ChunkT word_out =
                    ((word_in << (BITS_PER_WORD - shift_mod)) | carry_bits);
            _block->_data[chunk_num] = word_out;
            carry_bits = carry_out;
          }
        BitVector::ChunkT last_word_in;
        if (new_length + shift_div < length)
            last_word_in = _block->_data[new_length + shift_div];
        else
            last_word_in = (_block->_infinity_bit_is_one ? ~0ul : 0ul);
        _block->_data[new_length - 1] =
                ((last_word_in << (BITS_PER_WORD - shift_mod)) | carry_bits);
      }
    _block->_data_length = new_length;
  }

void BitVector::operator<<=(size_t shift_amount)
  {
    if (shift_amount < 0)
      {
        (*this) >>= -shift_amount;
        return;
      }
    if ((_block->_num_significant_bits == 0) &&
        (!_block->_infinity_bit_is_one))
      {
        return;
      }
    make_changable();
    size_t orig_significant_bits = _block->_num_significant_bits;
    size_t new_significant_bits = orig_significant_bits + shift_amount;
    _block->_num_significant_bits = new_significant_bits;
    size_t length = _block->_data_length;
    size_t shift_div = shift_amount / BITS_PER_WORD;
    size_t shift_mod = shift_amount % BITS_PER_WORD;
    size_t new_length =
            (new_significant_bits + (BITS_PER_WORD - 1)) / BITS_PER_WORD;
    _block->expand_data(new_length);
    if (shift_amount % 8 == 0)
      {
        memmove(((char *)(_block->_data)) + (shift_amount / 8),
                (char *)(_block->_data), (orig_significant_bits + 7) / 8);
        memset((char *)(_block->_data), 0, shift_amount / 8);
        size_t leftover_bytes_written =
                ((((new_significant_bits + 7) / 8)) % sizeof(BitVector::ChunkT));
        if (leftover_bytes_written != 0)
          {
            if (_block->_infinity_bit_is_one)
              {
                _block->_data[new_length - 1] |=
                        ((~0ul) << (leftover_bytes_written * 8));
              }
            else
              {
                _block->_data[new_length - 1] &=
                        ~((~0ul) << (leftover_bytes_written * 8));
              }
          }
      }
    else
      {
        BitVector::ChunkT carry_bits;
        size_t start_chunk;
        if (length + shift_div < new_length)
          {
            carry_bits =
                    ((_block->_infinity_bit_is_one ? ~0ul : 0ul) << shift_mod);
            start_chunk = length;
          }
        else
          {
            carry_bits = (_block->_data[length - 1] << shift_mod);
            start_chunk = length - 1;
          }
        for (size_t chunk_num = start_chunk; chunk_num > 0; --chunk_num)
          {
            BitVector::ChunkT word_in = _block->_data[chunk_num - 1];
            BitVector::ChunkT carry_out = (word_in << shift_mod);
            BitVector::ChunkT word_out =
                    ((word_in >> (BITS_PER_WORD - shift_mod)) | carry_bits);
            _block->_data[chunk_num + shift_div] = word_out;
            carry_bits = carry_out;
          }
        _block->_data[shift_div] = carry_bits;
        memset((char *)(_block->_data), 0, shift_div);
      }
  }

bool BitVector::do_and_with_test(const BitVector &other)
  {
    bool result = false;
    make_changable();
    size_t length = _block->_data_length;
    size_t other_length = other._block->_data_length;
    size_t common_length =
            ((other_length > length) ? length : other_length);
    for (size_t chunk_num = 0; chunk_num < common_length; ++chunk_num)
      {
        BitVector::ChunkT orig_chunk = _block->_data[chunk_num];
        BitVector::ChunkT new_chunk = orig_chunk & other._block->_data[chunk_num];
        _block->_data[chunk_num] = new_chunk;
        if (orig_chunk != new_chunk)
            result = true;
      }
    if (other_length > length)
      {
        if (_block->_infinity_bit_is_one)
          {
            _block->expand_data(other_length);
            for (size_t chunk_num = length; chunk_num < other_length;
                 ++chunk_num)
              {
                _block->_data[chunk_num] = other._block->_data[chunk_num];
              }
            _block->_num_significant_bits =
                    other._block->_num_significant_bits;
            result = true;
          }
      }
    else if (other_length < length)
      {
        if (!other._block->_infinity_bit_is_one)
          {
            _block->_data_length = other_length;
            _block->_num_significant_bits =
                    other._block->_num_significant_bits;
            result = true;
          }
      }
    else
      {
        if (other._block->_num_significant_bits >
            _block->_num_significant_bits)
          {
            _block->_num_significant_bits =
                    other._block->_num_significant_bits;
            result = true;
          }
      }
    if (_block->_infinity_bit_is_one && !(other._block->_infinity_bit_is_one))
        result = true;
    _block->_infinity_bit_is_one =
            ((_block->_infinity_bit_is_one ? 1 : 0) &
             (other._block->_infinity_bit_is_one ? 1 : 0) == 1);
    if (result)
        _block->unpad();
    return result;
  }

bool BitVector::do_or_with_test(const BitVector &other)
  {
    bool result = false;
    make_changable();
    size_t length = _block->_data_length;
    size_t other_length = other._block->_data_length;
    size_t common_length =
            ((other_length > length) ? length : other_length);
    for (size_t chunk_num = 0; chunk_num < common_length; ++chunk_num)
      {
        BitVector::ChunkT orig_chunk = _block->_data[chunk_num];
        BitVector::ChunkT new_chunk = orig_chunk | other._block->_data[chunk_num];
        _block->_data[chunk_num] = new_chunk;
        if (orig_chunk != new_chunk)
            result = true;
      }
    if (other_length > length)
      {
        if (!_block->_infinity_bit_is_one)
          {
            _block->expand_data(other_length);
            for (size_t chunk_num = length; chunk_num < other_length;
                 ++chunk_num)
              {
                _block->_data[chunk_num] = other._block->_data[chunk_num];
              }
            _block->_num_significant_bits =
                    other._block->_num_significant_bits;
            result = true;
          }
      }
    else if (other_length < length)
      {
        if (other._block->_infinity_bit_is_one)
          {
            _block->_data_length = other_length;
            _block->_num_significant_bits =
                    other._block->_num_significant_bits;
            result = true;
          }
      }
    else
      {
        if (other._block->_num_significant_bits >
            _block->_num_significant_bits)
          {
            _block->_num_significant_bits =
                    other._block->_num_significant_bits;
            result = true;
          }
      }
    if ((!(_block->_infinity_bit_is_one)) &&
        other._block->_infinity_bit_is_one)
      {
        result = true;
      }
    _block->_infinity_bit_is_one =
            ((_block->_infinity_bit_is_one ? 1 : 0) |
             (other._block->_infinity_bit_is_one ? 1 : 0) == 1);
    if (result)
        _block->unpad();
    return result;
  }

bool BitVector::do_subtract_with_test(const BitVector &other)
  {
    BitVector old(*this);
    this->subtract(other);
    return(old == *this);
  }

void BitVector::subtract(const BitVector &other)
  {
    BitVector old = ~other;
    *this &= old;
    return;
  }

BitVector BitVector::operator-(const BitVector &other) const
  {
    return((*this) & (~other));
  }






/*
 *  There are two pre-computed tables here, the byte_count_table and
 *  the one_bit_num_table.  They can be generated by this little C
 *  program:

-------------------------BEGIN-PROGRAM-----------------------------
static void print_bit_comment(int byte_value);

extern int main(int argc, char *argv[])
  {
    int counter;

    for (counter = 0; counter < 256; ++counter)
      {
        int count = 0;
	unsigned int i;

        if ((counter != 0) && ((counter % 16) == 0))
            printf("\n");
	for (i = 0; i < 8; i++) {
	  if (counter & (1U << i)) count++;
	}
        printf("    %d", count);
        if (counter < 255)
            printf(", ");
        else
            printf("  ");
        print_bit_comment(counter);
        printf("\n");
      }

    for (counter = 0; counter < 256; ++counter)
      {
        int last_one_bit = 8; // when 0
	unsigned i;

        if ((counter != 0) && ((counter % 16) == 0))
            printf("\n");
	for (i = 0; i < 8; i++) {
	  if (counter & (1U << (i))) {
	    last_one_bit = (i);
	    break;
	  }
	}
        printf("    %d", last_one_bit);
	printf(counter < 255 ? ",  " : "   " );
        print_bit_comment(counter);
        printf("\n");
      }

    return 0;
  }

static void print_bit_comment(int byte_value)
  {
    unsigned i;
    printf("/");
    printf("* ");
    for (i = 0; i < 8; i++) {
      printf("%d", (byte_value & (1U << (7-i))) ? 1 : 0);
    }
    printf(" *");
    printf("/");
  }
				 
--------------------------END-PROGRAM------------------------------
 */

/*  This table maps from a byte to the number of ``one'' bits in that
 *  byte. */
static unsigned char byte_count_table[256] =
  {
    0, /* 00000000 */
    1, /* 00000001 */
    1, /* 00000010 */
    2, /* 00000011 */
    1, /* 00000100 */
    2, /* 00000101 */
    2, /* 00000110 */
    3, /* 00000111 */
    1, /* 00001000 */
    2, /* 00001001 */
    2, /* 00001010 */
    3, /* 00001011 */
    2, /* 00001100 */
    3, /* 00001101 */
    3, /* 00001110 */
    4, /* 00001111 */

    1, /* 00010000 */
    2, /* 00010001 */
    2, /* 00010010 */
    3, /* 00010011 */
    2, /* 00010100 */
    3, /* 00010101 */
    3, /* 00010110 */
    4, /* 00010111 */
    2, /* 00011000 */
    3, /* 00011001 */
    3, /* 00011010 */
    4, /* 00011011 */
    3, /* 00011100 */
    4, /* 00011101 */
    4, /* 00011110 */
    5, /* 00011111 */

    1, /* 00100000 */
    2, /* 00100001 */
    2, /* 00100010 */
    3, /* 00100011 */
    2, /* 00100100 */
    3, /* 00100101 */
    3, /* 00100110 */
    4, /* 00100111 */
    2, /* 00101000 */
    3, /* 00101001 */
    3, /* 00101010 */
    4, /* 00101011 */
    3, /* 00101100 */
    4, /* 00101101 */
    4, /* 00101110 */
    5, /* 00101111 */

    2, /* 00110000 */
    3, /* 00110001 */
    3, /* 00110010 */
    4, /* 00110011 */
    3, /* 00110100 */
    4, /* 00110101 */
    4, /* 00110110 */
    5, /* 00110111 */
    3, /* 00111000 */
    4, /* 00111001 */
    4, /* 00111010 */
    5, /* 00111011 */
    4, /* 00111100 */
    5, /* 00111101 */
    5, /* 00111110 */
    6, /* 00111111 */

    1, /* 01000000 */
    2, /* 01000001 */
    2, /* 01000010 */
    3, /* 01000011 */
    2, /* 01000100 */
    3, /* 01000101 */
    3, /* 01000110 */
    4, /* 01000111 */
    2, /* 01001000 */
    3, /* 01001001 */
    3, /* 01001010 */
    4, /* 01001011 */
    3, /* 01001100 */
    4, /* 01001101 */
    4, /* 01001110 */
    5, /* 01001111 */

    2, /* 01010000 */
    3, /* 01010001 */
    3, /* 01010010 */
    4, /* 01010011 */
    3, /* 01010100 */
    4, /* 01010101 */
    4, /* 01010110 */
    5, /* 01010111 */
    3, /* 01011000 */
    4, /* 01011001 */
    4, /* 01011010 */
    5, /* 01011011 */
    4, /* 01011100 */
    5, /* 01011101 */
    5, /* 01011110 */
    6, /* 01011111 */

    2, /* 01100000 */
    3, /* 01100001 */
    3, /* 01100010 */
    4, /* 01100011 */
    3, /* 01100100 */
    4, /* 01100101 */
    4, /* 01100110 */
    5, /* 01100111 */
    3, /* 01101000 */
    4, /* 01101001 */
    4, /* 01101010 */
    5, /* 01101011 */
    4, /* 01101100 */
    5, /* 01101101 */
    5, /* 01101110 */
    6, /* 01101111 */

    3, /* 01110000 */
    4, /* 01110001 */
    4, /* 01110010 */
    5, /* 01110011 */
    4, /* 01110100 */
    5, /* 01110101 */
    5, /* 01110110 */
    6, /* 01110111 */
    4, /* 01111000 */
    5, /* 01111001 */
    5, /* 01111010 */
    6, /* 01111011 */
    5, /* 01111100 */
    6, /* 01111101 */
    6, /* 01111110 */
    7, /* 01111111 */

    1, /* 10000000 */
    2, /* 10000001 */
    2, /* 10000010 */
    3, /* 10000011 */
    2, /* 10000100 */
    3, /* 10000101 */
    3, /* 10000110 */
    4, /* 10000111 */
    2, /* 10001000 */
    3, /* 10001001 */
    3, /* 10001010 */
    4, /* 10001011 */
    3, /* 10001100 */
    4, /* 10001101 */
    4, /* 10001110 */
    5, /* 10001111 */

    2, /* 10010000 */
    3, /* 10010001 */
    3, /* 10010010 */
    4, /* 10010011 */
    3, /* 10010100 */
    4, /* 10010101 */
    4, /* 10010110 */
    5, /* 10010111 */
    3, /* 10011000 */
    4, /* 10011001 */
    4, /* 10011010 */
    5, /* 10011011 */
    4, /* 10011100 */
    5, /* 10011101 */
    5, /* 10011110 */
    6, /* 10011111 */

    2, /* 10100000 */
    3, /* 10100001 */
    3, /* 10100010 */
    4, /* 10100011 */
    3, /* 10100100 */
    4, /* 10100101 */
    4, /* 10100110 */
    5, /* 10100111 */
    3, /* 10101000 */
    4, /* 10101001 */
    4, /* 10101010 */
    5, /* 10101011 */
    4, /* 10101100 */
    5, /* 10101101 */
    5, /* 10101110 */
    6, /* 10101111 */

    3, /* 10110000 */
    4, /* 10110001 */
    4, /* 10110010 */
    5, /* 10110011 */
    4, /* 10110100 */
    5, /* 10110101 */
    5, /* 10110110 */
    6, /* 10110111 */
    4, /* 10111000 */
    5, /* 10111001 */
    5, /* 10111010 */
    6, /* 10111011 */
    5, /* 10111100 */
    6, /* 10111101 */
    6, /* 10111110 */
    7, /* 10111111 */

    2, /* 11000000 */
    3, /* 11000001 */
    3, /* 11000010 */
    4, /* 11000011 */
    3, /* 11000100 */
    4, /* 11000101 */
    4, /* 11000110 */
    5, /* 11000111 */
    3, /* 11001000 */
    4, /* 11001001 */
    4, /* 11001010 */
    5, /* 11001011 */
    4, /* 11001100 */
    5, /* 11001101 */
    5, /* 11001110 */
    6, /* 11001111 */

    3, /* 11010000 */
    4, /* 11010001 */
    4, /* 11010010 */
    5, /* 11010011 */
    4, /* 11010100 */
    5, /* 11010101 */
    5, /* 11010110 */
    6, /* 11010111 */
    4, /* 11011000 */
    5, /* 11011001 */
    5, /* 11011010 */
    6, /* 11011011 */
    5, /* 11011100 */
    6, /* 11011101 */
    6, /* 11011110 */
    7, /* 11011111 */

    3, /* 11100000 */
    4, /* 11100001 */
    4, /* 11100010 */
    5, /* 11100011 */
    4, /* 11100100 */
    5, /* 11100101 */
    5, /* 11100110 */
    6, /* 11100111 */
    4, /* 11101000 */
    5, /* 11101001 */
    5, /* 11101010 */
    6, /* 11101011 */
    5, /* 11101100 */
    6, /* 11101101 */
    6, /* 11101110 */
    7, /* 11101111 */

    4, /* 11110000 */
    5, /* 11110001 */
    5, /* 11110010 */
    6, /* 11110011 */
    5, /* 11110100 */
    6, /* 11110101 */
    6, /* 11110110 */
    7, /* 11110111 */
    5, /* 11111000 */
    6, /* 11111001 */
    6, /* 11111010 */
    7, /* 11111011 */
    6, /* 11111100 */
    7, /* 11111101 */
    7, /* 11111110 */
    8  /* 11111111 */
  };
static unsigned char first_one_bit[256] =
  {
    8,  /* 00000000 */
    0,  /* 00000001 */
    1,  /* 00000010 */
    0,  /* 00000011 */
    2,  /* 00000100 */
    0,  /* 00000101 */
    1,  /* 00000110 */
    0,  /* 00000111 */
    3,  /* 00001000 */
    0,  /* 00001001 */
    1,  /* 00001010 */
    0,  /* 00001011 */
    2,  /* 00001100 */
    0,  /* 00001101 */
    1,  /* 00001110 */
    0,  /* 00001111 */

    4,  /* 00010000 */
    0,  /* 00010001 */
    1,  /* 00010010 */
    0,  /* 00010011 */
    2,  /* 00010100 */
    0,  /* 00010101 */
    1,  /* 00010110 */
    0,  /* 00010111 */
    3,  /* 00011000 */
    0,  /* 00011001 */
    1,  /* 00011010 */
    0,  /* 00011011 */
    2,  /* 00011100 */
    0,  /* 00011101 */
    1,  /* 00011110 */
    0,  /* 00011111 */

    5,  /* 00100000 */
    0,  /* 00100001 */
    1,  /* 00100010 */
    0,  /* 00100011 */
    2,  /* 00100100 */
    0,  /* 00100101 */
    1,  /* 00100110 */
    0,  /* 00100111 */
    3,  /* 00101000 */
    0,  /* 00101001 */
    1,  /* 00101010 */
    0,  /* 00101011 */
    2,  /* 00101100 */
    0,  /* 00101101 */
    1,  /* 00101110 */
    0,  /* 00101111 */

    4,  /* 00110000 */
    0,  /* 00110001 */
    1,  /* 00110010 */
    0,  /* 00110011 */
    2,  /* 00110100 */
    0,  /* 00110101 */
    1,  /* 00110110 */
    0,  /* 00110111 */
    3,  /* 00111000 */
    0,  /* 00111001 */
    1,  /* 00111010 */
    0,  /* 00111011 */
    2,  /* 00111100 */
    0,  /* 00111101 */
    1,  /* 00111110 */
    0,  /* 00111111 */

    6,  /* 01000000 */
    0,  /* 01000001 */
    1,  /* 01000010 */
    0,  /* 01000011 */
    2,  /* 01000100 */
    0,  /* 01000101 */
    1,  /* 01000110 */
    0,  /* 01000111 */
    3,  /* 01001000 */
    0,  /* 01001001 */
    1,  /* 01001010 */
    0,  /* 01001011 */
    2,  /* 01001100 */
    0,  /* 01001101 */
    1,  /* 01001110 */
    0,  /* 01001111 */

    4,  /* 01010000 */
    0,  /* 01010001 */
    1,  /* 01010010 */
    0,  /* 01010011 */
    2,  /* 01010100 */
    0,  /* 01010101 */
    1,  /* 01010110 */
    0,  /* 01010111 */
    3,  /* 01011000 */
    0,  /* 01011001 */
    1,  /* 01011010 */
    0,  /* 01011011 */
    2,  /* 01011100 */
    0,  /* 01011101 */
    1,  /* 01011110 */
    0,  /* 01011111 */

    5,  /* 01100000 */
    0,  /* 01100001 */
    1,  /* 01100010 */
    0,  /* 01100011 */
    2,  /* 01100100 */
    0,  /* 01100101 */
    1,  /* 01100110 */
    0,  /* 01100111 */
    3,  /* 01101000 */
    0,  /* 01101001 */
    1,  /* 01101010 */
    0,  /* 01101011 */
    2,  /* 01101100 */
    0,  /* 01101101 */
    1,  /* 01101110 */
    0,  /* 01101111 */

    4,  /* 01110000 */
    0,  /* 01110001 */
    1,  /* 01110010 */
    0,  /* 01110011 */
    2,  /* 01110100 */
    0,  /* 01110101 */
    1,  /* 01110110 */
    0,  /* 01110111 */
    3,  /* 01111000 */
    0,  /* 01111001 */
    1,  /* 01111010 */
    0,  /* 01111011 */
    2,  /* 01111100 */
    0,  /* 01111101 */
    1,  /* 01111110 */
    0,  /* 01111111 */

    7,  /* 10000000 */
    0,  /* 10000001 */
    1,  /* 10000010 */
    0,  /* 10000011 */
    2,  /* 10000100 */
    0,  /* 10000101 */
    1,  /* 10000110 */
    0,  /* 10000111 */
    3,  /* 10001000 */
    0,  /* 10001001 */
    1,  /* 10001010 */
    0,  /* 10001011 */
    2,  /* 10001100 */
    0,  /* 10001101 */
    1,  /* 10001110 */
    0,  /* 10001111 */

    4,  /* 10010000 */
    0,  /* 10010001 */
    1,  /* 10010010 */
    0,  /* 10010011 */
    2,  /* 10010100 */
    0,  /* 10010101 */
    1,  /* 10010110 */
    0,  /* 10010111 */
    3,  /* 10011000 */
    0,  /* 10011001 */
    1,  /* 10011010 */
    0,  /* 10011011 */
    2,  /* 10011100 */
    0,  /* 10011101 */
    1,  /* 10011110 */
    0,  /* 10011111 */

    5,  /* 10100000 */
    0,  /* 10100001 */
    1,  /* 10100010 */
    0,  /* 10100011 */
    2,  /* 10100100 */
    0,  /* 10100101 */
    1,  /* 10100110 */
    0,  /* 10100111 */
    3,  /* 10101000 */
    0,  /* 10101001 */
    1,  /* 10101010 */
    0,  /* 10101011 */
    2,  /* 10101100 */
    0,  /* 10101101 */
    1,  /* 10101110 */
    0,  /* 10101111 */

    4,  /* 10110000 */
    0,  /* 10110001 */
    1,  /* 10110010 */
    0,  /* 10110011 */
    2,  /* 10110100 */
    0,  /* 10110101 */
    1,  /* 10110110 */
    0,  /* 10110111 */
    3,  /* 10111000 */
    0,  /* 10111001 */
    1,  /* 10111010 */
    0,  /* 10111011 */
    2,  /* 10111100 */
    0,  /* 10111101 */
    1,  /* 10111110 */
    0,  /* 10111111 */

    6,  /* 11000000 */
    0,  /* 11000001 */
    1,  /* 11000010 */
    0,  /* 11000011 */
    2,  /* 11000100 */
    0,  /* 11000101 */
    1,  /* 11000110 */
    0,  /* 11000111 */
    3,  /* 11001000 */
    0,  /* 11001001 */
    1,  /* 11001010 */
    0,  /* 11001011 */
    2,  /* 11001100 */
    0,  /* 11001101 */
    1,  /* 11001110 */
    0,  /* 11001111 */

    4,  /* 11010000 */
    0,  /* 11010001 */
    1,  /* 11010010 */
    0,  /* 11010011 */
    2,  /* 11010100 */
    0,  /* 11010101 */
    1,  /* 11010110 */
    0,  /* 11010111 */
    3,  /* 11011000 */
    0,  /* 11011001 */
    1,  /* 11011010 */
    0,  /* 11011011 */
    2,  /* 11011100 */
    0,  /* 11011101 */
    1,  /* 11011110 */
    0,  /* 11011111 */

    5,  /* 11100000 */
    0,  /* 11100001 */
    1,  /* 11100010 */
    0,  /* 11100011 */
    2,  /* 11100100 */
    0,  /* 11100101 */
    1,  /* 11100110 */
    0,  /* 11100111 */
    3,  /* 11101000 */
    0,  /* 11101001 */
    1,  /* 11101010 */
    0,  /* 11101011 */
    2,  /* 11101100 */
    0,  /* 11101101 */
    1,  /* 11101110 */
    0,  /* 11101111 */

    4,  /* 11110000 */
    0,  /* 11110001 */
    1,  /* 11110010 */
    0,  /* 11110011 */
    2,  /* 11110100 */
    0,  /* 11110101 */
    1,  /* 11110110 */
    0,  /* 11110111 */
    3,  /* 11111000 */
    0,  /* 11111001 */
    1,  /* 11111010 */
    0,  /* 11111011 */
    2,  /* 11111100 */
    0,  /* 11111101 */
    1,  /* 11111110 */
    0   /* 11111111 */
  };



size_t BitVector::count() const {
  if (_has_count) return _count;

  size_t chunk_count = get_chunk_count();
  size_t count = 0;
  for (size_t chunk_num = 0; chunk_num < chunk_count; ++chunk_num)
    {
      BitVector::ChunkT this_chunk = get_chunk(chunk_num);
      // invert this if sign bit is 0
      if (get_infinity_bit()) { this_chunk = ~this_chunk; }

      {
      for (BitVector::ChunkT this_chunk = get_chunk(chunk_num);
	   this_chunk != 0; this_chunk >>= (BITS_PER_CHAR)) {
	unsigned char c = this_chunk & ((1U << BITS_PER_CHAR)-1);
	count += byte_count_table[c];
      }
      }
    }
  ((BitVector *)this)->_count = count;
  ((BitVector *)this)->_has_count = true;
  return(count);
}





BitVectorIter::BitVectorIter(const BitVector *bv):
  _done(false),
  _current_bit(0),
  _remaining_byte(0),
  _bv(bv)
{
  first();
}

BitVectorIter::BitVectorIter(const BitVectorIter &other) :
  _done(other._done),
  _current_bit(other._current_bit),
  _remaining_byte(other._remaining_byte),
  _bv(other._bv)
{}

BitVectorIter &BitVectorIter::operator=(const BitVectorIter &other) {
  _done = other._done;
  _current_bit = other._current_bit;
  _remaining_byte = other._remaining_byte;
  _bv = other._bv;
  return(*this);
}

  // support the old-style increment(), done()
  // and the new-style is_valid(); next()
bool BitVectorIter::is_valid() const { return !done(); }
void BitVectorIter::increment() { next(); }
bool BitVectorIter::done() const { return(_done); }
size_t BitVectorIter::get() const { return current(); }
void BitVectorIter::reset() { first(); }


void BitVectorIter::first() 
  {
    if (_bv->num_significant_bits() == 0) {
      _done = true;
      return;
    }
    // set up the first byte
    BitVector::ChunkT this_chunk = _bv->get_chunk(0);
    bool ones_bit = _bv->get_infinity_bit();
    if (ones_bit) this_chunk = ~this_chunk;
    unsigned byte_num = 0; 
    unsigned char this_byte = 
      (this_chunk >> (BITS_PER_CHAR * byte_num));
    _remaining_byte = this_byte;
    _done = false;
    _current_bit = 0;
    next();
  }

size_t BitVectorIter::current() const {
  suif_assert(!_done);
  return(_current_bit);
}


void BitVectorIter::next()
  {
    suif_assert(!_done);
    // First the simple case:
    // there is still more in the current byte.
    if (_remaining_byte != 0) {
      unsigned this_byte = _remaining_byte;
      size_t first_bit = first_one_bit[this_byte];
      size_t current_bit = _current_bit;
      size_t current_byte = current_bit/BITS_PER_CHAR;
      current_bit = current_byte * BITS_PER_CHAR + first_bit;

      _current_bit = current_bit;
      // we could use the mask, but why bother. We know the bit is set.
      unsigned char next_byte = this_byte - ( 1U << first_bit);
      _remaining_byte = next_byte;
      return;
    }
    size_t current_bit = _current_bit;
    current_bit = current_bit + (BITS_PER_CHAR - 
				 (current_bit % (BITS_PER_CHAR)));
    
    // walk through each chunk...
    size_t chunk_count = _bv->get_chunk_count();
    bool ones_bit = _bv->get_infinity_bit();
    unsigned chunk_num = current_bit / BITS_PER_WORD;
    unsigned byte_num = (current_bit % BITS_PER_WORD) / BITS_PER_CHAR;
    for (; chunk_num < chunk_count; chunk_num++, byte_num = 0) 
      {
	BitVector::ChunkT this_chunk = _bv->get_chunk(chunk_num);
	if (ones_bit) this_chunk = ~this_chunk;
	if (this_chunk == 0) continue;
	for (;
	     byte_num < (BITS_PER_WORD/BITS_PER_CHAR); 
	     byte_num++) {
	  unsigned char this_byte = 
	    (this_chunk >> (BITS_PER_CHAR * byte_num));
	  if (this_byte == 0) continue;
	  size_t first_bit = first_one_bit[this_byte];
	  _current_bit = chunk_num * BITS_PER_WORD
	    + byte_num * BITS_PER_CHAR
	    + first_bit;
	  unsigned char next_byte = this_byte - ( 1U << first_bit);
	  _remaining_byte = next_byte;
	  _done = false;
	  return;
	}
      }
    _done = true;
  }


	
