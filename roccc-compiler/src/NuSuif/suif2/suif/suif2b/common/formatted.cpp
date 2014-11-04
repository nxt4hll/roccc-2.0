#include "system_specific.h"
#include "MString.h"
#include "formatted.h"
#include "suif_map.h"

#include <ctype.h>
#include <stdlib.h>
#include <stddef.h>

class LiteralPrint : public PrintAddress {
    public:
        String get_pointer_as_string(void *addr);
    };


class PointerPrint : public PrintAddress {
    public:
        PointerPrint();
        ~PointerPrint();

        String get_pointer_as_string(void *addr);
    private:
        int count;
        suif_map<void *, String> _sm;
    };

//	Here because not all machines have this
static bool mystrcasecmp(const char* s1,const char *s2) {

    if (!s1) {
      return (!s2);
      }

    if (!s2) {
      return false;
      }

    while ((toupper(*s1)) == (toupper(*s2)) && (*s1)) {
      s1++;
      s2++;
      }

    return (toupper(*s1)) == (toupper(*s2));
    }


static bool get_use_diff_mode() {
    static  const char* _getenv;
    static  bool _setenv = false;
    static  bool _result;

    if(!_setenv)
        {
        _setenv = true;
        _getenv = getenv("SUIFDIFF");
        if(_getenv != NULL)
            {
            _result = mystrcasecmp(_getenv, "true");
            if(!_result)
                _result = mystrcasecmp(_getenv, "yes");
            }
        else {
            _result = false;
            }
        }

    return _result;
    }






class TextBlock {
public:
  TextBlock *parent;
  String title;
  String value;
  simple_stack<TextBlock *>children;
  TextBlock(TextBlock *p);
  ~TextBlock();
private:
  TextBlock(const TextBlock &);
  TextBlock &operator=(const TextBlock &);
};


FormattedText::FormattedText(	int maxline,
			int the_indent,
			bool nolinemerge ) :
		max_line(maxline),
		indent(the_indent),
		no_line_merge(nolinemerge) ,
		max_header_len(0),
		root(0),
		current(0),
		_pa(NULL)
    {
    set_use_diff_mode(get_use_diff_mode());
    start_block(String(""));
    }

FormattedText::~FormattedText() {
  delete root;
}

TextBlock::~TextBlock() {
  // delete the children
  for (int i = 0;i < children.len(); i++)
    {
      delete children[i];
    }
}

TextBlock::TextBlock(TextBlock *p) :
		parent(p), title(), value(), children()
    { }

TextBlock::TextBlock(const TextBlock &p) :
  parent(0), title(0), value(0), children(0)
    { assert(0); }

TextBlock &TextBlock::operator=(const TextBlock &p) 
{
  assert(0); return(*this); 
}


void FormattedText::start_block(const String &title)
    {
    TextBlock *t = new TextBlock(current);
    if (current == NULL)
	{
	root = t;
	}
    else
	{
	current->children.push(t);
	}
    current = t;
    t->title = title;
    }

void FormattedText::set_value(const String &val)
    {
    current->value = val;
    }
void FormattedText::set_value(const LString &val)
    {
    current->value = String(val);
    }

void FormattedText::end_block()
    {
    current = current->parent;
    }

int FormattedText::total_len(TextBlock *block) const
    {
    int len = block->title.length();
    for (int i=0;i < block->children.len(); i++)
	len += total_len(block->children[i]);
    return len;
    }

int FormattedText::get_indent(int cur,TextBlock *block) const
    {
    if (!no_line_merge && (total_len(block) < max_line))
	{
	return cur;
	}
    if (block->title.length() > cur)
	cur = block->title.length();
    for (int i = 0;i < block->children.len(); i++)
	{
	cur = get_indent(cur,block->children[i]);
 	}
    return cur;
    }

String FormattedText::get_value() const
    {
    int ind;
    if (indent >= 0)
 	ind = indent;
    else
	ind = get_indent(0,root);
    String ind_text;
    int i;
    for (i = 0;i < ind; i++)
	ind_text += " ";
    return get_value(ind_text,ind_text,no_line_merge,root) + "\n";
    }

String FormattedText::get_value(String cur_pad,String indent,bool no_newlines,TextBlock *block) const
    {
    String val = block->title + "[";
    if (block->value.length() > 0)
	{
	val += block->value;
	return val + "]";
	}
    if (no_newlines || (!no_line_merge && (total_len(block) < max_line)))
	{
	for (int i =0;i < block->children.len(); i++)
	    val += get_value("","",true,block->children[i]);
	return val + "]";
	}
    val += "\n";
    String new_pad = cur_pad + indent;
    for (int i = 0; i < block->children.len(); i++)
	{
	val += cur_pad + get_value(new_pad,indent,false,block->children[i]);
	val +="\n";
	}
    return val + cur_pad + "]";
    }

void FormattedText::set_value(int i)
    {
    set_value(String(i));
    }

void FormattedText::set_value(const IInteger &v)
     {
       char *s = v.to_string();
       set_value(String(s));
       delete [] s;
     }

void FormattedText::set_value(bool b)
    {
    if (b)
	set_value(String("true"));
    else
	set_value(String("false"));
    }

void FormattedText::set_value(long l)
    {
    set_value(String(l));
    }

void FormattedText::set_value(double d)
    {
    set_value(String(d));
    }

void FormattedText::set_value(LString &l)
    {
    set_value(String(l));
    }

void FormattedText::set_value(const char *c)
    {
    set_value(String(c));
    }

void FormattedText::set_value(const void *p)
    {
    if (p == NULL)
	set_value("NULL");
    else
	{
	char text[12];
    //FIXME - 64bit problem
    	//sprintf(text,"0x%08X",(int)p);
    	sprintf(text,"0x%08X",(long int)p);
	set_value(text);
	}
    }


String FormattedText::pointer_header(String x,void *p)
    {
    if (p == NULL)
        return x + String("[NULL]");
    else
        {
	static String _leftB = "[";
	static String _rightB = "]->";
	String _pointer = _pa->get_pointer_as_string(p);
	return x + _leftB + _pointer + _rightB;
        }
    }

#include <stdio.h>

PointerPrint::PointerPrint() {
    count = 0;
    }

PointerPrint::~PointerPrint() {
    }


String LiteralPrint::get_pointer_as_string(void *addr) {
    if (addr == NULL)
    	return String("NULL");
    else {
    	char text[12];
        //FIXME - 64bit problem
    	//sprintf(text,"0x%08X",(int)addr);
    	sprintf(text,"0x%08X",(long int)addr);
    	return String(text);
  	}
    }

String PointerPrint::get_pointer_as_string(void *addr) {
    suif_map<void *, String>::iterator _iter = _sm.find(addr);

    if(_iter != _sm.end())
    	return (*_iter).second;

    count++;

    String result = String ("pointer")+String(count); 

    _sm.enter_value(addr, result); 
    return (result); 
    }

void FormattedText::set_use_diff_mode(bool diff) {
    if (diff)
	set_special_print_mode(new PointerPrint);
    else
	set_special_print_mode(new LiteralPrint);
    }

void FormattedText::set_special_print_mode(PrintAddress *p) {
    if (_pa)delete _pa;
    _pa = p;
    }

