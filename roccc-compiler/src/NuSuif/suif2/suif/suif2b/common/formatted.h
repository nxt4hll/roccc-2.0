#ifndef __FORMAT_H__
#define __FORMAT_H__

#include "MString.h"
#include "simple_stack.h"
#include "i_integer.h"

/**	@file
 *	A helper class for printing (Suif) trees
 */

/**	\class PrintAddress formatted.h common/formatted.h
 *	allows you to control
 *	how addresses are to be output. This is useful
 *	to print out addresses as identifiers rather
 *	than actual addresses, to make comparisons
 *	easier 
 */
class PrintAddress { 
    public:
	/**	Given an address, return a printable String
	 *	version of it
	 */
  	virtual String get_pointer_as_string(void *addr) = 0;
	virtual ~PrintAddress() {}
    };

//	This class supports output of formatted text in a simple
//	minded way.
//
//	Typical entries consist of the form
//
//		StartBlock
//		Text with Separator calls
//		EndBlock
//
// A typical use of this might be
//   SuifObject *something;
//   FormattedText txt;
//   something->print(txt);
//   String s = txt.get_value();
//

class TextBlock;

/**	\class FormattedText formatted.h common/formatted.h
 *	Control structure for outputting a tree of data
 *	Handles indentation automatically
 */

class FormattedText
    {
	int max_line;
	int indent;
	bool no_line_merge;

	int max_header_len;
	TextBlock *root;
	TextBlock *current;

    public:

	/**	Create a formatted text object
	 *	@param maxline {longest line to output (default 80)}
	 *	@param the_indent {indent for nested fields (default 4)}
	 *	@param nolinemerge {merge short lines 	(default false)}
	 */
	FormattedText(	int maxline = 80,
			int the_indent = 4,
			bool nolinemerge=false);

	/**	Output a header for a pointer. 
	 *	Output looks like par1[pointer]=>
	 *	@param {identifying string}
	 *	@param {the pointer}
	 *	@see PrintAddress
	 */
	String pointer_header(class String, void *);

	/**	Set (or reset) use of diff mode for pointers
	 *	diff mode is useful when you want to diff
	 *	output from different runs
	 */
	void set_use_diff_mode(bool diff);  // set to use diff mode

	/**	Use your own class for printing pointers */
	void set_special_print_mode(PrintAddress *p); // or your own pointer print 

	~FormattedText();

	/**	Start and indented block with the given title*/
	void start_block(const String &title);

	/**	set_value - set the value associated with an entry
	 *	entries can have values or children, not both */
	void set_value(const String &val);
	void set_value(const LString &val);
	void set_value(int i);
	void set_value(bool b);
	void set_value(long l);
	void set_value(double d);
	void set_value(LString &l);
	void set_value(const char *c);
	void set_value(const IInteger &v);
	void set_value(const void *p);

	/**	EndBlock - return to previous level*/
	void end_block();

	/**	get indentation required for given text */
	int get_indent(int cur,TextBlock *block) const;
	int total_len(TextBlock *block) const;
	/**	get value of text block as a String*/
	String get_value() const;

	String get_value(const String cur_pad,const String indent,bool no_newlines,TextBlock *block) const;
      private:

	PrintAddress* _pa;
        FormattedText(const FormattedText &);
        FormattedText &operator=(const FormattedText &);
    };


#endif



















