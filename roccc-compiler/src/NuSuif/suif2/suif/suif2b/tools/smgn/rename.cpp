//	Rename old type names to new
#include <stdlib.h>
#include "common/text_util.h"
#include "common/simple_stack.h"
#include "common/MString.h"
#include "common/suif_hash_map.h"
#include <stddef.h>
#include <stdio.h>

static void usage(const char *text = NULL) {
  if (text) 
    fprintf(stderr,"error: %s\n",text);
  
  fprintf(stderr,"usage : rename <renames file> <file1 > <file 2> ....\n");
  exit(-1);
}// end usage

/*int hash(String x)*/
size_t hash(String x)  /*jul modif*/ {
  int len = x.length();
  int i = 0;
  for (int j = 0;j < len; j++)
    i = (i << 2) + x[j];
  return (size_t)i;
}//end hash

suif_hash_map<String ,String> renames;
 

static void read_renames(const char *filename, char *renames_defs)
    {
    const char *text = renames_defs;
    text = skip_space(text);
    while (*text) {
	if (!isalpha(*text)) {
	    send_error(filename, renames_defs,text,"expected identifier",false);
	    exit(-1);
	    }
	String old_name,new_name;
	int len = get_name(text,old_name);
	text = skip_space(text + len);
	if (*text != '=') {
	    send_error(filename, renames_defs,text,"expected =",false);
	    exit(-1);
	    }
	text = skip_space(text + 1);
	len = get_name(text,new_name);
        text = skip_space(text + len);
	renames.enter_value(old_name,new_name);
	}
    }

static void translate(char *filename) {
  int count = 0;
  
  char *text = get_file_text(filename);
  char *start = text;
  if (text == NULL) {
    fprintf(stderr,"file %s not found\n",filename);
    return;
  }
  printf("translating %s:",filename);
  String result(1000,1000);
  while (*text) {
    if (isalpha(*text)) {
      String name;
      int len = get_name(text,name);
      if (len <= 0) {
	send_error(filename, start,text,"invalid identifier found",false);
	exit(-1);
      }
      text += len;
      suif_hash_map<String,String>::iterator iter = renames.find(name);
      if (iter != renames.end()) {
	count ++;
	result += (*iter).second;
      }
      else
	result += name;
    }
    else {
      result.push(*text);
      text ++;
    }
  }
  write_text(filename,(const char *)result);
  printf("%d substitutions made\n",count);
}// end translate
	    
int main(int argc,char *argv[]) {
  if (argc < 3) 
    usage();
  
  char * renames = get_file_text(argv[1]);
  
  if (renames == NULL)
    usage("renames file missing\n");
  
  read_renames(argv[1], renames);
  
  delete renames;

  for (int i = 2;i < argc; i++) {
    translate(argv[i]);
  }
}//end main
