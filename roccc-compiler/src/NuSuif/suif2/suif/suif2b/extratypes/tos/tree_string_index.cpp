/* file "tree_string_index.cc" */


/*
       Copyright (c) 1996, 1997 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>
#include <string.h>
#include <common/lstring.h>

/*
      This is the implementation of the tree_string_index template and
      related templates, which are templates for classes implementing
      mappings from character strings to arbitrary types using a tree
      representation for fast lookups, for sty, the first-level main
      library of the SUIF system.
*/


#ifndef DOING_TEMPLATE_INLINING

//#define _MODULE_ "libsty"

//#pragma implementation "tree_string_index.h"


#include <common/machine_dependent.h>
#include "referenced_item.h"
#include "ion/ion.h"
#include <common/i_integer.h>
#include "index.h"
#include "tree_string_index.h"
#include <string.h>

#ifndef ANTIQUE_CPP
void (*indirection_function)(void *, ion *) = 0;

extern void indirect_print(void_holder data, ion *ip)
  {
    (*indirection_function)(data, ip);
  }
#endif /* not ANTIQUE_CPP */

#endif /* not DOING_TEMPLATE_INLINING */


#if ((!defined(INLINE_ALL_TEMPLATES)) || defined(DOING_TEMPLATE_INLINING))

enum tsi_node_kind { TSI_SUBSTRING, TSI_CHAR_LIST, TSI_CHAR_TABLE, TSI_LEAF };

template <class elem_t> class tsi_node;

template <class elem_t> class tsi_entry: public referenced_item
  {
private:
    size_t _ref_count;

public:
    elem_t _data;
    tsi_node<elem_t> *back_pointer;

    tsi_entry(elem_t init_data) : _ref_count(1), _data(init_data),
            back_pointer(0)  { }
    ~tsi_entry(void)  { suif_assert(_ref_count == 0); }

    void add_ref(void)
      {
        if (_ref_count < SIZE_T_MAX)
            ++_ref_count;
      }
    void remove_ref(void)
      {
        assert(_ref_count > 0);
        if (_ref_count < SIZE_T_MAX)
            --_ref_count;
      }
    bool delete_me(void)  { return (_ref_count == 0); }
  };

template <class elem_t> class tsi_substring;
template <class elem_t> class tsi_char_list;
template <class elem_t> class tsi_char_table;
template <class elem_t> class tsi_leaf;

template <class elem_t> class tsi_node
  {
    friend class tsi_substring<elem_t>;
    friend class tsi_char_list<elem_t>;
    friend class tsi_char_table<elem_t>;
    friend class tsi_leaf<elem_t>;

private:
    tsi_entry<elem_t> *_entry;

protected:
    tsi_node<elem_t> *_parent_node;
    char _parent_index;

    tsi_node(void) : 
      _entry(0), _parent_node(0), _parent_index(0) { }

public:
    virtual ~tsi_node(void)
      {
        if (_entry != 0)
          {
            _entry->remove_ref();
            if (_entry->delete_me())
                delete _entry;
          }
      }

    virtual tsi_node_kind kind(void) = 0;

    void replace(tsi_node<elem_t> *replacement);
    void remove(void);

    tsi_entry<elem_t> *base_entry(void) const  { return _entry; }
    void replace_base_entry(tsi_entry<elem_t> *new_entry);
    tsi_node<elem_t> *find_branchpoint(void);

    tsi_node<elem_t> *parent_node(void) const  { return _parent_node; }
  };

template <class elem_t> class tsi_substring : public tsi_node<elem_t>
  {
private:
    char *_chars;
    tsi_node<elem_t> *_child;

public:
    tsi_substring(size_t char_count, const char *init_chars,
                  tsi_node<elem_t> *init_child);
    ~tsi_substring(void);

    tsi_node_kind kind(void)  { return TSI_SUBSTRING; }

    char *chars(void)  { return _chars; }
    tsi_node<elem_t> *child(void)  { return _child; }

    void replace_child(tsi_node<elem_t> *new_child);
  };

template <class elem_t> class tsi_char_list : public tsi_node<elem_t>
  {
private:
    size_t _table_size;
    size_t _num_entries;
    tsi_node<elem_t> **_children;
    char *_chars;

public:
    tsi_char_list(size_t init_table_size);
    ~tsi_char_list(void);

    tsi_node_kind kind(void)  { return TSI_CHAR_LIST; }

    size_t count(void)  { return _num_entries; }
    tsi_node<elem_t> *child(size_t child_num)
      { assert(child_num < _num_entries); return _children[child_num]; }
    char which_char(size_t child_num)
      { assert(child_num < _num_entries); return _chars[child_num]; }

    void add_child(size_t child_num, tsi_node<elem_t> *new_child,
                   char this_char);
    void remove_child(size_t child_num);
    void replace_child(size_t child_num, tsi_node<elem_t> *new_child);
  };

template <class elem_t> class tsi_char_table : public tsi_node<elem_t>
  {
private:
    tsi_node<elem_t> **_children;
    char _first_char;
    char _last_char;

public:
    tsi_char_table(char init_lower, char init_upper);
    ~tsi_char_table(void);

    tsi_node_kind kind(void)  { return TSI_CHAR_TABLE; }

    tsi_node<elem_t> *child(char which_char)
      {
        if ((which_char >= _first_char) && (which_char <= _last_char))
            return _children[which_char - _first_char];
        else
            return 0;
      }
    char lower_char(void)  { return _first_char; }
    char upper_char(void)  { return _last_char; }

    void replace_child(char which_char, tsi_node<elem_t> *new_child);
  };

template <class elem_t> class tsi_leaf : public tsi_node<elem_t>
  {
public:
    tsi_leaf(tsi_entry<elem_t> *init_entry)
      { replace_base_entry(init_entry); }
    ~tsi_leaf(void)  { }

    tsi_node_kind kind(void)  { return TSI_LEAF; }
  };


template <class elem_t> void tsi_node<elem_t>::replace(
        tsi_node<elem_t> *replacement)
  {
    assert(_parent_node != 0);
    switch (_parent_node->kind())
      {
        case TSI_SUBSTRING:
          {
            tsi_substring<elem_t> *par_substring =
                    (tsi_substring<elem_t> *)_parent_node;
            assert(par_substring->child() == this);
            par_substring->replace_child(replacement);
            break;
          }
        case TSI_CHAR_LIST:
          {
            tsi_char_list<elem_t> *par_char_list =
                    (tsi_char_list<elem_t> *)_parent_node;
            size_t count = par_char_list->count();
            for (size_t child_num = 0; child_num < count; ++child_num)
              {
                if (par_char_list->child(child_num) == this)
                  {
                    par_char_list->replace_child(child_num, replacement);
                    return;
                  }
              }
            assert(false);
            break;
          }
        case TSI_CHAR_TABLE:
          {
            tsi_char_table<elem_t> *par_char_table =
                    (tsi_char_table<elem_t> *)_parent_node;
            assert(par_char_table->child(_parent_index) == this);
            par_char_table->replace_child(_parent_index, replacement);
            break;
          }
        case TSI_LEAF:
            assert(false);
        default:
            assert(false);
      }
  }

template <class elem_t> void tsi_node<elem_t>::remove(void)
  {
    assert(_parent_node != 0);
    switch (_parent_node->kind())
      {
        case TSI_SUBSTRING:
          {
            tsi_substring<elem_t> *par_substring =
                    (tsi_substring<elem_t> *)_parent_node;
            assert(par_substring->child() == this);
            par_substring->replace_child(0);
            break;
          }
        case TSI_CHAR_LIST:
          {
            tsi_char_list<elem_t> *par_char_list =
                    (tsi_char_list<elem_t> *)_parent_node;
            size_t count = par_char_list->count();
            for (size_t child_num = 0; child_num < count; ++child_num)
              {
                if (par_char_list->child(child_num) == this)
                  {
                    par_char_list->remove_child(child_num);
                    return;
                  }
              }
            assert(false);
            break;
          }
        case TSI_CHAR_TABLE:
          {
            tsi_char_table<elem_t> *par_char_table =
                    (tsi_char_table<elem_t> *)_parent_node;
            assert(par_char_table->child(_parent_index) == this);
            par_char_table->replace_child(_parent_index, 0);
            break;
          }
        case TSI_LEAF:
            assert(false);
        default:
            assert(false);
      }
  }

template <class elem_t> void tsi_node<elem_t>::replace_base_entry(
        tsi_entry<elem_t> *new_entry)
  {
    if (_entry == new_entry)
        return;
    if (_entry != 0)
      {
        assert(_entry->back_pointer == this);
        _entry->back_pointer = 0;
        _entry->remove_ref();
        if (_entry->delete_me())
            delete _entry;
      }
    _entry = new_entry;
    if (new_entry != 0)
      {
        assert(new_entry->back_pointer == 0);
        new_entry->back_pointer = this;
        new_entry->add_ref();
      }
  }

template <class elem_t> tsi_node<elem_t> *
        tsi_node<elem_t>::find_branchpoint(void)
  {
    if (_parent_node == 0)
        return this;
    if (_parent_node->kind() != TSI_SUBSTRING)
        return this;
    tsi_substring<elem_t> *parent_substring =
        (tsi_substring<elem_t> *)_parent_node;
    if (parent_substring->base_entry() != 0)
        return parent_substring;
    return parent_substring->find_branchpoint();
  }


template <class elem_t> tsi_substring<elem_t>::tsi_substring(
        size_t char_count, const char *init_chars,
        tsi_node<elem_t> *init_child) :
  _chars(new char[char_count + 1]),
  _child(init_child)
  {
    assert((init_child == 0) || (init_child->_parent_node == 0));
    //    _chars = new char[char_count + 1];
    memcpy(_chars, init_chars, char_count);
    _chars[char_count] = 0;
    //    _child = init_child;
    if (init_child != 0)
        init_child->_parent_node = this;
  }

template <class elem_t> tsi_substring<elem_t>::~tsi_substring(void)
  {
    delete[] _chars;
    if (_child != 0)
        delete _child;
  }

template <class elem_t> void tsi_substring<elem_t>::replace_child(
        tsi_node<elem_t> *new_child)
  {
    assert((new_child == 0) || (new_child->_parent_node == 0));
    if (_child != 0)
        _child->_parent_node = 0;
    _child = new_child;
    if (new_child != 0)
        new_child->_parent_node = this;
  }


template <class elem_t> tsi_char_list<elem_t>::tsi_char_list(
     size_t init_table_size) :
  _table_size(init_table_size),
  _num_entries(0),
  _children(new tsi_node<elem_t> *[init_table_size]),
  _chars(new char[init_table_size])

  {
    //    _table_size = init_table_size;
    //    _num_entries = 0;
    //    _children = new tsi_node<elem_t> *[init_table_size];
    //    _chars = new char[init_table_size];
  }

template <class elem_t> tsi_char_list<elem_t>::~tsi_char_list(void)
  {
    for (size_t entry_num = 0; entry_num < _num_entries; ++entry_num)
      {
        tsi_node<elem_t> *this_child = _children[entry_num];
        if (this_child != 0)
            delete this_child;
      }
    delete[] _children;
    delete[] _chars;
  }

template <class elem_t> void tsi_char_list<elem_t>::add_child(
        size_t child_num, tsi_node<elem_t> *new_child, char this_char)
  {
    assert((new_child == 0) || (new_child->_parent_node == 0));
    assert(child_num <= _num_entries);
    if (_num_entries == _table_size)
      {
        tsi_node<elem_t> **new_child_list =
                new tsi_node<elem_t> *[_table_size * 2];
        char *new_char_list = new char[_table_size * 2];
        memcpy(new_child_list, _children,
               _table_size * sizeof(tsi_node<elem_t> *));
        memcpy(new_char_list, _chars, _table_size * sizeof(char));
        delete[] _children;
        delete[] _chars;
        _children = new_child_list;
        _chars = new_char_list;
        _table_size *= 2;
      }
    if (child_num < _num_entries)
      {
        memmove(&(_children[child_num + 1]), &(_children[child_num]),
                (_num_entries - child_num) * sizeof(tsi_node<elem_t> *));
        memmove(&(_chars[child_num + 1]), &(_chars[child_num]),
                (_num_entries - child_num) * sizeof(char));
      }
    _children[child_num] = new_child;
    _chars[child_num] = this_char;
    ++_num_entries;
    if (new_child != 0)
        new_child->_parent_node = this;
  }

template <class elem_t> void tsi_char_list<elem_t>::remove_child(
        size_t child_num)
  {
    assert(child_num < _num_entries);
    if (_children[child_num] != 0)
        _children[child_num]->_parent_node = 0;
    --_num_entries;
    if (child_num < _num_entries)
      {
        memmove(&(_children[child_num]), &(_children[child_num + 1]),
                (_num_entries - child_num) * sizeof(tsi_node<elem_t> *));
        memmove(&(_chars[child_num]), &(_chars[child_num + 1]),
                (_num_entries - child_num) * sizeof(char));
      }
  }

template <class elem_t> void tsi_char_list<elem_t>::replace_child(
        size_t child_num, tsi_node<elem_t> *new_child)
  {
    assert((new_child == 0) || (new_child->_parent_node == 0));
    assert(child_num < _num_entries);
    if (_children[child_num] != 0)
        _children[child_num]->_parent_node = 0;
    _children[child_num] = new_child;
    if (new_child != 0)
        new_child->_parent_node = this;
  }


template <class elem_t> tsi_char_table<elem_t>::tsi_char_table(char init_lower,
	       char init_upper) :
  _children(0),
  _first_char(0),
  _last_char(0)
  {
    assert(init_lower <= init_upper);
    _first_char = init_lower;
    _last_char = init_upper;
    size_t init_size =
            (size_t)(unsigned char)(init_upper - init_lower) + 1;
    _children = new tsi_node<elem_t> *[init_size];
    for (char test_char = init_lower; test_char < init_upper; ++test_char)
        _children[test_char - init_lower] = 0;
    _children[init_upper - init_lower] = 0;
  }

template <class elem_t> tsi_char_table<elem_t>::~tsi_char_table(void)
  {
    for (char test_char = _first_char; test_char < _last_char; ++test_char)
      {
        tsi_node<elem_t> *this_child = _children[test_char - _first_char];
        if (this_child != 0)
            delete this_child;
      }
    tsi_node<elem_t> *this_child = _children[_last_char - _first_char];
    if (this_child != 0)
        delete this_child;
    delete[] _children;
  }

template <class elem_t> void tsi_char_table<elem_t>::replace_child(
        char which_char, tsi_node<elem_t> *new_child)
  {
    assert((new_child == 0) || (new_child->_parent_node == 0));
    if (which_char < _first_char)
      {
        size_t init_size =
                (size_t)(unsigned char)(_last_char - which_char) + 1;
        tsi_node<elem_t> **new_child_list = new tsi_node<elem_t> *[init_size];
        size_t extra = (size_t)(unsigned char)(_first_char - which_char);
        size_t orig_size =
                (size_t)(unsigned char)(_last_char - _first_char) + 1;
        memcpy(&(new_child_list[extra]), _children,
               orig_size * sizeof(tsi_node<elem_t> *));
        delete[] _children;
        _children = new_child_list;
        for (char test_char = which_char; test_char < _first_char; ++test_char)
            _children[test_char - which_char] = 0;
        _first_char = which_char;
      }
    else if (which_char > _last_char)
      {
        size_t init_size =
                (size_t)(unsigned char)(which_char - _first_char) + 1;
        tsi_node<elem_t> **new_child_list = new tsi_node<elem_t> *[init_size];
        size_t orig_size =
                (size_t)(unsigned char)(_last_char - _first_char) + 1;
        memcpy(new_child_list, _children,
               orig_size * sizeof(tsi_node<elem_t> *));
        delete[] _children;
        _children = new_child_list;
        for (char test_char = which_char; test_char > _last_char; --test_char)
            _children[test_char - _first_char] = 0;
        _last_char = which_char;
      }
    if (_children[which_char - _first_char] != 0)
        _children[which_char - _first_char]->_parent_node = 0;
    _children[which_char - _first_char] = new_child;
    if (new_child != 0)
      {
        new_child->_parent_node = this;
        new_child->_parent_index = which_char;
      }
  }


template <class elem_t> tree_string_index<elem_t>::tree_string_index(
     char expected_lower, char expected_upper, size_t list_size) :
  _root_node(0),
  _list_size(list_size),
  _expected_lower(expected_lower),
  _expected_upper(expected_upper)
{
  assert(list_size >= 2);
}

template <class elem_t> tree_string_index<elem_t>::~tree_string_index(void)
  {
    if (_root_node != 0)
        delete _root_node;
  }

template <class elem_t> tsi_node<elem_t> *tree_string_index<elem_t>::
        follow_prefix_match(const char *string, size_t *prefix_length) const
  {
    if (_root_node == 0)
        return 0;
    size_t position = 0;
    tsi_node<elem_t> *current_node = _root_node;
    while (string[position] != 0)
      {
        bool done = false;
        switch (current_node->kind())
          {
            case TSI_SUBSTRING:
              {
                tsi_substring<elem_t> *the_tsi_substring =
                        (tsi_substring<elem_t> *)current_node;
                char *the_chars = the_tsi_substring->chars();
                size_t num_chars = strlen(the_chars);
                if (strncmp(&(string[position]), the_chars, num_chars) == 0)
                  {
                    position += num_chars;
                    current_node = the_tsi_substring->child();
                  }
                else
                  {
                    done = true;
                  }
                break;
              }
            case TSI_CHAR_LIST:
              {
                tsi_char_list<elem_t> *the_tsi_char_list =
                        (tsi_char_list<elem_t> *)current_node;
                size_t count = the_tsi_char_list->count();
                char this_char = string[position];
                size_t child_num;
                for (child_num = 0; child_num < count; ++child_num)
                  {
                    if (the_tsi_char_list->which_char(child_num) >= this_char)
                        break;
                  }
                if ((child_num < count) &&
                    (the_tsi_char_list->which_char(child_num) == this_char))
                  {
                    ++position;
                    current_node = the_tsi_char_list->child(child_num);
                  }
                else
                  {
                    done = true;
                  }
                break;
              }
            case TSI_CHAR_TABLE:
              {
                tsi_char_table<elem_t> *the_tsi_char_table =
                        (tsi_char_table<elem_t> *)current_node;
                tsi_node<elem_t> *this_child =
                        the_tsi_char_table->child(string[position]);
                if (this_child != 0)
                  {
                    ++position;
                    current_node = this_child;
                  }
                else
                  {
                    done = true;
                  }
                break;
              }
            case TSI_LEAF:
              {
                done = true;
                break;
              }
            default:
                assert(false);
          }
        if (done)
            break;
      }
    *prefix_length = position;
    return current_node;
  }

template <class elem_t> void tree_string_index<elem_t>::
        internal_development_dump(char **buffer, size_t *buf_size,
        size_t position, tsi_node<elem_t> *base_node, ion *ip,
        void (*node_func)(elem_t data, ion *ip), bool show_internals)
  {
    if (base_node == 0)
        return;

    if (position >= *buf_size)
      {
        char *new_buffer = new char[*buf_size * 2];
        memcpy(new_buffer, *buffer, *buf_size);
        delete[] *buffer;
        *buffer = new_buffer;
        *buf_size *= 2;
      }

    if (show_internals)
      {
        ip->printf("%*s", (int)(8 + 2 * position), "");
        switch (base_node->kind())
          {
            case TSI_SUBSTRING:
              {
                tsi_substring<elem_t> *the_substring =
                        (tsi_substring<elem_t> *)base_node;
                ip->printf("sub <\"%s\">", the_substring->chars());
                break;
              }
            case TSI_CHAR_LIST:
              {
                tsi_char_list<elem_t> *the_char_list =
                        (tsi_char_list<elem_t> *)base_node;
                ip->printf("list[%lu]", the_char_list->count());
                break;
              }
            case TSI_CHAR_TABLE:
              {
                tsi_char_table<elem_t> *the_char_table =
                        (tsi_char_table<elem_t> *)base_node;
                ip->printf("table['%c'..'%c']", the_char_table->lower_char(),
                           the_char_table->upper_char());
                break;
              }
            case TSI_LEAF:
                ip->printf("leaf");
                break;
            default:
                assert(false);
          }
        ip->printf("\n");
      }

    tsi_entry<elem_t> *base_entry = base_node->base_entry();
    if (base_entry != 0)
      {
        (*buffer)[position] = 0;
        ip->printf("    (%p) \"%s\":", base_entry, *buffer);
        if (node_func != 0)
            (*node_func)(base_entry->_data, ip);
        else
            ip->printf(" [??]");
        ip->printf("\n");
      }

    switch (base_node->kind())
      {
        case TSI_SUBSTRING:
          {
            tsi_substring<elem_t> *the_tsi_substring =
                    (tsi_substring<elem_t> *)base_node;
            char *chars = the_tsi_substring->chars();
            size_t string_size = strlen(chars);
            if (position + string_size + 1 >= *buf_size)
              {
                char *new_buffer = 
                        new char[*buf_size + position + string_size + 1];
                memcpy(new_buffer, *buffer, *buf_size);
                delete[] *buffer;
                *buffer = new_buffer;
                *buf_size += position + string_size + 1;
              }
            strcpy(&((*buffer)[position]), chars);
            internal_development_dump(buffer, buf_size, position + string_size,
                                      the_tsi_substring->child(), ip,
                                      node_func, show_internals);
            break;
          }
        case TSI_CHAR_LIST:
          {
            tsi_char_list<elem_t> *the_tsi_char_list =
                    (tsi_char_list<elem_t> *)base_node;
            size_t count = the_tsi_char_list->count();
            for (size_t child_num = 0; child_num < count; ++child_num)
              {
                (*buffer)[position] = the_tsi_char_list->which_char(child_num);
                internal_development_dump(buffer, buf_size, position + 1,
                                          the_tsi_char_list->child(child_num),
                                          ip, node_func, show_internals);
              }
            break;
          }
        case TSI_CHAR_TABLE:
          {
            tsi_char_table<elem_t> *the_tsi_char_table =
                    (tsi_char_table<elem_t> *)base_node;
            char lower_char = the_tsi_char_table->lower_char();
            char upper_char = the_tsi_char_table->upper_char();
            for (char which = lower_char; which < upper_char; ++which)
              {
                (*buffer)[position] = which;
                internal_development_dump(buffer, buf_size, position + 1,
                                          the_tsi_char_table->child(which), ip,
                                          node_func, show_internals);
              }
            (*buffer)[position] = upper_char;
            internal_development_dump(buffer, buf_size, position + 1,
                                      the_tsi_char_table->child(upper_char),
                                      ip, node_func, show_internals);
            break;
          }
        case TSI_LEAF:
            break;
        default:
           assert(false);
      }
  }

template <class elem_t> void tree_string_index<elem_t>::get_development_stats(
        tsi_node<elem_t> *base_node, size_t *entry_count,
        size_t *substring_node_count, size_t *char_list_node_count,
        size_t *char_table_node_count, size_t *leaf_node_count,
        size_t *substring_char_count, size_t *char_list_place_count,
        size_t *char_list_used_count, size_t *char_table_place_count,
        size_t *char_table_used_count)
  {
    assert(base_node != 0);
    if (base_node->base_entry() != 0)
        ++*entry_count;

    switch (base_node->kind())
      {
        case TSI_SUBSTRING:
          {
            tsi_substring<elem_t> *the_tsi_substring =
                    (tsi_substring<elem_t> *)base_node;
            ++*substring_node_count;
            *substring_char_count += strlen(the_tsi_substring->c_str());
            get_development_stats(the_tsi_substring->child(), entry_count,
                                  substring_node_count, char_list_node_count,
                                  char_table_node_count, leaf_node_count,
                                  substring_char_count, char_list_place_count,
                                  char_list_used_count, char_table_place_count,
                                  char_table_used_count);
            break;
          }
        case TSI_CHAR_LIST:
          {
            tsi_char_list<elem_t> *the_tsi_char_list =
                    (tsi_char_list<elem_t> *)base_node;
            ++*char_list_node_count;
            size_t count = the_tsi_char_list->count();
            *char_list_place_count += _list_size;
            *char_list_used_count += count;
            for (size_t child_num = 0; child_num < count; ++child_num)
              {
                get_development_stats(the_tsi_char_list->child(child_num),
                                      entry_count, substring_node_count,
                                      char_list_node_count,
                                      char_table_node_count, leaf_node_count,
                                      substring_char_count,
                                      char_list_place_count,
                                      char_list_used_count,
                                      char_table_place_count,
                                      char_table_used_count);
              }
            break;
          }
        case TSI_CHAR_TABLE:
          {
            tsi_char_table<elem_t> *the_tsi_char_table =
                    (tsi_char_table<elem_t> *)base_node;
            ++*char_table_node_count;
            char lower_char = the_tsi_char_table->lower_char();
            char upper_char = the_tsi_char_table->upper_char();
            *char_table_place_count += 
                    ((size_t)(upper_char - lower_char)) + 1;
            for (char which = lower_char; which < upper_char; ++which)
              {
                tsi_node<elem_t> *this_child =
                        the_tsi_char_table->child(which);
                if (this_child != 0)
                  {
                    get_development_stats(this_child, entry_count,
                                          substring_node_count,
                                          char_list_node_count,
                                          char_table_node_count,
                                          leaf_node_count,
                                          substring_char_count,
                                          char_list_place_count,
                                          char_list_used_count,
                                          char_table_place_count,
                                          char_table_used_count);
                    ++*char_table_used_count;
                  }
              }
            tsi_node<elem_t> *this_child =
                    the_tsi_char_table->child(upper_char);
            if (this_child != 0)
              {
                get_development_stats(this_child, entry_count,
                                      substring_node_count,
                                      char_list_node_count,
                                      char_table_node_count, leaf_node_count,
                                      substring_char_count,
                                      char_list_place_count,
                                      char_list_used_count,
                                      char_table_place_count,
                                      char_table_used_count);
                ++*char_table_used_count;
              }
            break;
          }
        case TSI_LEAF:
            ++*leaf_node_count;
            break;
        default:
           assert(false);
      }
  }

template <class elem_t> tsi_entry<elem_t> *
        tree_string_index<elem_t>::lookup_entry(const char *the_key) const
  {
    assert(the_key != 0);
    size_t prefix_length;
    tsi_node<elem_t> *the_node = follow_prefix_match(the_key, &prefix_length);
    if ((the_node != 0) && (the_key[prefix_length] == 0))
        return the_node->base_entry();
    else
        return 0;
  }

template <class elem_t> elem_t tree_string_index<elem_t>::lookup_prefix(
        const char *the_key, const char **suffix) const
  {
    assert(the_key != 0);
    size_t prefix_length;
    tsi_node<elem_t> *the_node = follow_prefix_match(the_key, &prefix_length);
    if (the_node == 0)
        return zero((elem_t *)0);
    while (the_node->base_entry() == 0)
      {
        the_node = the_node->parent_node();
        if (the_node == 0)
            return zero((elem_t *)0);
        if (the_node->kind() == TSI_SUBSTRING)
          {
            tsi_substring<elem_t> *the_substring =
                    (tsi_substring<elem_t> *)the_node;
            assert(prefix_length > strlen(the_substring->c_str()));
            prefix_length -= strlen(the_substring->c_str());
          }
        else
          {
            assert(prefix_length > 1);
            --prefix_length;
          }
      }
    *suffix = &(the_key[prefix_length]);
    return the_node->base_entry()->_data;
  }

template <class elem_t> elem_t tree_string_index<elem_t>::lookup(
        const char *the_key) const
  {
    tsi_entry<elem_t> *the_entry = lookup_entry(the_key);
    if (the_entry != 0)
        return the_entry->_data;
    else
        return zero((elem_t *)0);
  }

template <class elem_t> bool tree_string_index<elem_t>::exists(
        const char *the_key) const
  {
    return (lookup_entry(the_key) != 0);
  }

template <class elem_t> index_handle<const char *, elem_t>
        tree_string_index<elem_t>::enter(const char *the_key, elem_t the_elem)
  {
    assert(the_key != 0);

    if (_root_node == 0)
      {
        tsi_entry<elem_t> *result = new tsi_entry<elem_t>(the_elem);
        tsi_leaf<elem_t> *new_leaf = new tsi_leaf<elem_t>(result);
        result->remove_ref();
        _root_node =
                new tsi_substring<elem_t>(strlen(the_key), the_key, new_leaf);
        return this->build_handle(result);
      }

    size_t prefix_length;
    tsi_node<elem_t> *the_node = follow_prefix_match(the_key, &prefix_length);
    assert(the_node != 0);
    if (the_key[prefix_length] == 0)
      {
        if (the_node->base_entry() != 0)
            return this->build_handle(0);
        tsi_entry<elem_t> *result = new tsi_entry<elem_t>(the_elem);
        the_node->replace_base_entry(result);
        result->remove_ref();
        return this->build_handle(result);
      }
    tsi_entry<elem_t> *result = new tsi_entry<elem_t>(the_elem);
    tsi_node<elem_t> *new_node;
    switch (the_node->kind())
      {
        case TSI_SUBSTRING:
          {
            tsi_substring<elem_t> *the_tsi_substring =
                    (tsi_substring<elem_t> *)the_node;
            char *old_chars = the_tsi_substring->chars();
            size_t match_length = 0;
            while (the_key[prefix_length + match_length] ==
                   old_chars[match_length])
              {
                assert(old_chars[match_length] != 0);
                ++match_length;
              }
            tsi_node<elem_t> *old_child = the_tsi_substring->child();
            the_tsi_substring->replace_child(0);
            size_t old_count = strlen(old_chars);
            assert(match_length < old_count);
            if (the_key[prefix_length + match_length] == 0)
              {
                tsi_substring<elem_t> *new_substring1 =
                        new tsi_substring<elem_t>(old_count - match_length,
                                          &(old_chars[match_length]),
                                          old_child);
                new_substring1->replace_base_entry(result);
                new_node =
                        new tsi_substring<elem_t>(match_length, old_chars,
                                          new_substring1);
                break;
              }
            if (match_length < (old_count - 1))
              {
                tsi_substring<elem_t> *new_substring =
                        new tsi_substring<elem_t>((old_count - 1) -
                                          match_length,
                                          &(old_chars[match_length + 1]),
                                          old_child);
                old_child = new_substring;
              }
            tsi_char_list<elem_t> *new_char_list =
                    new tsi_char_list<elem_t>(_list_size);
            tsi_node<elem_t> *new_child = new tsi_leaf<elem_t>(result);
            const char *suffix = &(the_key[prefix_length + match_length + 1]);
            if (suffix[0] != 0)
              {
                tsi_substring<elem_t> *new_substring =
                        new tsi_substring<elem_t>(strlen(suffix), suffix,
                                                  new_child);
                new_child = new_substring;
              }
            char old_char = old_chars[match_length];
            char new_char = the_key[prefix_length + match_length];
            if (old_char > new_char)
              {
                char temp_char = old_char;
                old_char = new_char;
                new_char = temp_char;
                tsi_node<elem_t> *temp_child = old_child;
                old_child = new_child;
                new_child = temp_child;
              }
            new_char_list->add_child(0, old_child, old_char);
            new_char_list->add_child(1, new_child, new_char);
            new_node = new_char_list;
            if (match_length > 0)
              {
                const char *prefix = &(the_key[prefix_length]);
                new_node = new tsi_substring<elem_t>(match_length, prefix,
                                                     new_node);
              }
            break;
          }
        case TSI_CHAR_LIST:
          {
            tsi_char_list<elem_t> *the_tsi_char_list =
                    (tsi_char_list<elem_t> *)the_node;
            size_t count = the_tsi_char_list->count();
            tsi_node<elem_t> *new_child = new tsi_leaf<elem_t>(result);
            const char *suffix = &(the_key[prefix_length + 1]);
            if (*suffix != 0)
              {
                new_child = new tsi_substring<elem_t>(strlen(suffix), suffix,
                                                      new_child);
              }
            if (count == _list_size)
              {
                tsi_char_table<elem_t> *new_table =
                        new tsi_char_table<elem_t>(_expected_lower,
                                                   _expected_upper);
                for (unsigned child_ind = 0; child_ind < count; ++child_ind)
                  {
                    size_t child_num = (count - child_ind) - 1;
                    tsi_node<elem_t> *this_child =
                            the_tsi_char_list->child(child_num);
                    char which_char = the_tsi_char_list->which_char(child_num);
                    the_tsi_char_list->remove_child(child_num);
                    new_table->replace_child(which_char, this_child);
                  }
                assert(the_tsi_char_list->count() == 0);
                new_table->replace_child(the_key[prefix_length], new_child);
                new_node = new_table;
                break;
              }
            else
              {
                char this_char = the_key[prefix_length];
                unsigned child_num;
                for (child_num = 0; child_num < count; ++child_num)
                  {
                    if (the_tsi_char_list->which_char(child_num) > this_char)
                        break;
                  }
                the_tsi_char_list->add_child(child_num, new_child, this_char);
                result->remove_ref();
                return this->build_handle(result);
              }
          }
        case TSI_CHAR_TABLE:
          {
            tsi_char_table<elem_t> *the_tsi_char_table =
                    (tsi_char_table<elem_t> *)the_node;
            assert(the_tsi_char_table->child(the_key[prefix_length]) == 0);
            tsi_node<elem_t> *new_child = new tsi_leaf<elem_t>(result);
            const char *suffix = &(the_key[prefix_length + 1]);
            if (*suffix != 0)
              {
                new_child = new tsi_substring<elem_t>(strlen(suffix), suffix,
                                                      new_child);
              }
            the_tsi_char_table->replace_child(the_key[prefix_length],
                                              new_child);
            result->remove_ref();
            return this->build_handle(result);
          }
        case TSI_LEAF:
          {
            const char *suffix = &(the_key[prefix_length]);
            tsi_leaf<elem_t> *new_leaf = new tsi_leaf<elem_t>(result);
            new_node = new tsi_substring<elem_t>(strlen(suffix), suffix,
                                                 new_leaf);
            break;
          }
        default:
            assert(false);
            return this->build_handle(0);
      }
    if (the_node == _root_node)
        _root_node = new_node;
    else
        the_node->replace(new_node);
    tsi_entry<elem_t> *old_base = the_node->base_entry();
    if (old_base != 0)
        old_base->add_ref();
    the_node->replace_base_entry(0);
    new_node->replace_base_entry(old_base);
    if (old_base != 0)
        old_base->remove_ref();
    delete the_node;
    result->remove_ref();
    return this->build_handle(result);
  }

template <class elem_t> void tree_string_index<elem_t>::remove(
        const char *the_key)
  {
    tsi_entry<elem_t> *the_entry = lookup_entry(the_key);
    assert(the_entry != 0);
    remove(this->build_handle(the_entry));
  }

template <class elem_t> index_handle<const char *, elem_t>
        tree_string_index<elem_t>::lookup_handle(const char *the_key) const
  {
    tsi_entry<elem_t> *the_entry = lookup_entry(the_key);
    if (the_entry != 0)
        return this->build_handle(the_entry);
    else
        return this->build_handle(0);
  }

template <class elem_t> elem_t tree_string_index<elem_t>::elem(
        index_handle<const char *, elem_t> handle) const
  {
    tsi_entry<elem_t> *the_entry = (tsi_entry<elem_t> *)(from_handle(handle));
    assert(the_entry != 0);
    return the_entry->_data;
  }

template <class elem_t> void tree_string_index<elem_t>::remove(
        index_handle<const char *, elem_t> handle)
  {
    tsi_entry<elem_t> *the_entry = (tsi_entry<elem_t> *)(from_handle(handle));
    assert(the_entry != 0);
    tsi_node<elem_t> *back_pointer = the_entry->back_pointer;
    assert(back_pointer != 0);
    assert(back_pointer->base_entry() == the_entry);
    back_pointer->replace_base_entry(0);
    if (back_pointer->kind() == TSI_LEAF)
      {
        tsi_node<elem_t> *branchpoint = back_pointer->find_branchpoint();
        tsi_entry<elem_t> *branchpoint_entry = branchpoint->base_entry();
        if ((branchpoint_entry != 0) && (branchpoint_entry != the_entry))
          {
            branchpoint_entry->add_ref();
            branchpoint->replace_base_entry(0);
            tsi_leaf<elem_t> *new_leaf =
                    new tsi_leaf<elem_t>(branchpoint_entry);
            branchpoint_entry->remove_ref();
            assert(!branchpoint_entry->delete_me());
            if (branchpoint == _root_node)
                _root_node = new_leaf;
            else
                branchpoint->replace(new_leaf);
          }
        else
          {
            if (branchpoint == _root_node)
                _root_node = 0;
            else
                branchpoint->remove();
          }
        delete branchpoint;
      }
  }

template <class elem_t> void tree_string_index<elem_t>::clear(void)
  {
    if (_root_node != 0)
        delete _root_node;
    _root_node = 0;
  }

template <class elem_t> void tree_string_index<elem_t>::development_dump(
        ion *ip, void (*node_func)(elem_t data, ion *ip))
  {
    if (_root_node == 0)
      {
        ip->printf("  <empty>\n");
      }
    else
      {
        size_t buf_size = 20;
        char *buffer = new char[buf_size];
        internal_development_dump(&buffer, &buf_size, 0, _root_node, ip,
                                  node_func, false);
        delete[] buffer;
      }
  }

template <class elem_t> void
        tree_string_index<elem_t>::development_internals_dump(ion *ip)
  {
    if (_root_node == 0)
      {
        ip->printf("  <empty>\n");
      }
    else
      {
        size_t buf_size = 20;
        char *buffer = new char[buf_size];
        internal_development_dump(&buffer, &buf_size, 0, _root_node, ip, 0,
                                  true);
        delete[] buffer;
      }
  }

template <class elem_t> void
        tree_string_index<elem_t>::development_stats_dump(ion *ip)
  {
    size_t entry_count = 0;

    size_t substring_node_count = 0;
    size_t char_list_node_count = 0;
    size_t char_table_node_count = 0;
    size_t leaf_node_count = 0;

    size_t substring_char_count = 0;
    size_t char_list_place_count = 0;
    size_t char_list_used_count = 0;
    size_t char_table_place_count = 0;
    size_t char_table_used_count = 0;

    if (_root_node != 0)
      {
        get_development_stats(_root_node, &entry_count, &substring_node_count,
                              &char_list_node_count, &char_table_node_count,
                              &leaf_node_count, &substring_char_count,
                              &char_list_place_count, &char_list_used_count,
                              &char_table_place_count, &char_table_used_count);
      }

    size_t total_node_count =
            substring_node_count + char_list_node_count +
            char_table_node_count + leaf_node_count;
    size_t total_memory = sizeof(tree_string_index<elem_t>) +
            entry_count * sizeof(tsi_entry<elem_t>) +
            substring_node_count * sizeof(tsi_substring<elem_t>) +
              (substring_char_count + substring_node_count) * sizeof(char) +
            char_list_node_count * sizeof(tsi_char_list<elem_t>) +
              char_list_place_count * (sizeof(tsi_node<elem_t> *) +
                                       sizeof(char)) +
            char_table_node_count * sizeof(tsi_char_table<elem_t>) +
              char_table_place_count * sizeof(tsi_node<elem_t> *) +
            leaf_node_count * sizeof(tsi_leaf<elem_t>);

    ip->printf(
"    Substring nodes:                                   %20lu\n"
"        Chars in substrings:          %20lu\n"
"    Char list nodes:                                   %20lu\n"
"        Char list spaces:             %20lu\n"
"        Char list spaces occupied:    %20lu (%5.1f%%)\n"
"    Char table nodes:                                  %20lu\n"
"        Char table spaces:            %20lu\n"
"        Char table spaces occupied:   %20lu (%5.1f%%)\n"
"    Leaf nodes:                                        %20lu\n"
"                                                       --------------------\n"
"  Total nodes:                                         %20lu\n"
"\n"
"  Total entries:                                       %20lu\n"
"\n"
"  Total direct memory requirement:                     %20lu\n",
        (unsigned long)substring_node_count,
        (unsigned long)substring_char_count,
        (unsigned long)char_list_node_count,
        (unsigned long)char_list_place_count,
        (unsigned long)char_list_used_count,
        (char_list_place_count == 0) ? 0.0 :
        ((((double)char_list_used_count) / ((double)char_list_place_count)) *
         100.0), (unsigned long)char_table_node_count,
        (unsigned long)char_table_place_count,
        (unsigned long)char_table_used_count,
        (char_table_place_count == 0) ? 0.0 :
        ((((double)char_table_used_count) / ((double)char_table_place_count)) *
         100.0), (unsigned long)leaf_node_count,
        (unsigned long)total_node_count, (unsigned long)entry_count,
        (unsigned long)total_memory);
  }

#endif /* ((!defined(INLINE_ALL_TEMPLATES)) ||
           defined(DOING_TEMPLATE_INLINING)) */
