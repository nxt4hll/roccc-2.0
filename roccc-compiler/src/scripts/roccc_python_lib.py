#!/usr/bin/env python

# This script compiles the chosen file(s) or directory 

import os
import re
import sys
import getopt

### Define constants ##########################################################


# Environment variables
CONST_ENV_VAR_ROCCC_HOME='ROCCC_HOME'
CONST_ENV_VAR_XILINX='XILINX'
CONST_ENV_VAR_RASC='RASC'


# Error return values, eventuall used for debugging
# to map to string errors
RETURN_DONE=0
ERROR_NONE=0
ERROR_ENV_VAR_UNDEFINED=1
ERROR_PARSE_ARG=2
ERROR_SYNTHESIS=3
ERROR_FILE_DOES_NOT_EXIST=4
ERROR_BAD=5

ERROR_SIGINT_KILL=99

# Random string constants
SPACE=' '
COMMA=','
DOT='.'
PERIOD='.'
COLON=':'
SEMICOLON=';'
QUOTE='"'
DOUBLEQUOTE='"'
COMMENT='//'
CURR_DIR='./'
PREV_DIR='../'

######
# SIGNAL NAMES, more to add
sig_dict = { }
sig_dict[2] = "SIGINT"
sig_dict[3] = "SIGQUIT"
sig_dict[4] = "SIGILL"
sig_dict[6] = "SIGABRT"
sig_dict[8] = "SIGFPE"
sig_dict[9] = "SIGKILL"
sig_dict[11] = "SIGSEGV"
sig_dict[15] = "SIGTERM"



### Global Variables ##########################################################
# verbose level
VERBOSE_LEVEL=2
SCRIPT_NAME = None

### Define functions ##########################################################

#######################
# CHECK and set default script name
#######################
def check_script_name() : 
  global SCRIPT_NAME

  if SCRIPT_NAME == None : 
    SCRIPT_NAME = 'NO-SCRIPT-NAME'



#######################
# ERROR Printing
#######################
def error_print(retval, the_string):
  """Effective method for printing an error message and having it return the correct error."""

  check_script_name()

  print '%s: ERROR(%d): '% (SCRIPT_NAME, retval),
  print the_string
  sys.stdout.flush()
  debug_print('########################################################') 
  debug_print('<<<<<< (%s) SCRIPT FAIL <<<<<<' % SCRIPT_NAME )
  debug_print('########################################################') 

  sys.exit(retval)


#######################
# DEBUG Printing
#######################
def debug_print(the_string, level=2):
  """Based on the global variable VERBOSE_LEVEL it will print out the string
    if the VERBOSE_LEVEL exceeds or matches the input level. Useful for debug 
    printing."""

  check_script_name()

  if VERBOSE_LEVEL >= level:
    print SCRIPT_NAME + ': ' + the_string
    sys.stdout.flush()


#######################
# PRINT RETURN VALUE ERROR (useful for when calling os.system() and checking return value)
#######################
def print_system_return_error_if_exists(the_retval, the_error_msg = None) : 
  """Print error values from either user terminated fault or from nonzero return value of code"""

  if the_error_msg == None : 
    the_error_msg = 'ERROR'

  if os.WEXITSTATUS(the_retval) != 0:
    error_print(ERROR_SYNTHESIS, "%s(%d)" % ( the_error_msg, os.WEXITSTATUS(the_retval)) )

  elif os.WIFSIGNALED(the_retval) :
    # USER SIGINT SIGNALED
    if os.WTERMSIG(the_retval) == 2 :  
      error_print(ERROR_SIGINT_KILL, "TERMINATED BY USER (SIGNAL = %d) = %s" % (os.WTERMSIG(the_retval), sig_dict[os.WTERMSIG(the_retval)] ) )
    # possible segfault
    else : 
      error_print(ERROR_SIGINT_KILL, "TERMINATED BY SIGNAL(%d) = %s" % (os.WTERMSIG(the_retval), sig_dict[os.WTERMSIG(the_retval)] ) )


#######################
# Execute shell command
#######################
def execute_shell_cmd( the_cmd ):
  """Execute shell command and use the error check to validate and return the return value"""

  debug_print('EXECUTE: ' + DOUBLEQUOTE + the_cmd + DOUBLEQUOTE )
  retval = os.system(the_cmd)
  print_system_return_error_if_exists(retval)

  return retval
  

#######################
# Check Environment Variable (singular)
#######################
def check_env_var( env_var ):
  """Check environment variable to make sure it is set"""
  try:
    os.environ[ env_var]
  except:
    error_print(ERROR_ENV_VAR_UNDEFINED, 'ERROR: Environment variable(s) undefined: "%s"'% env_var)

#######################
# Check Environment Variables (list)
#######################
def check_env_vars( the_string_list=[] ):
  """Check environment variables to make sure all set"""

  for env_var in the_string_list : 
    check_env_var(env_var)

#######################
# verify file exists
#######################
def verify_file_exists(filepath):
  """Check to make sure file exist"""

  if not os.path.isfile(filepath):
    error_print(ERROR_FILE_DOES_NOT_EXIST, 'File \"' + filepath + '\" does not exist')


#######################
# Copy file to new file path
#######################
def copy_filepath_to_filepath(filepath_in, filepath_out):
  """Copy individual file to new filepath"""

  verify_file_exists(filepath_in)

  cmd_cp_file = 'cp' + SPACE + filepath_in + SPACE + filepath_out
  debug_print('COPY: ' + filepath_in + ' -> ' + filepath_out )
  os.system(cmd_cp_file)

  verify_file_exists(filepath_out)


#######################
# Copy files in list to directory
#######################
def copy_files_to_dir(filelist_in, dirpath) : 
  """Copy list of files to new directory.  If does not exist, create the directory"""

  if not os.path.isdir(dirpath):
    os.mkdir(dirpath)
  
  # copy all files in list
  for the_file in filelist_in : 
    verify_file_exists(the_file)
    cmd_cp_file  = 'cp' + SPACE + the_file + SPACE + dirpath
    debug_print('COPY: ' + the_file + ' -> ' + dirpath )
    os.system(cmd_cp_file)


#######################
# Replace all patterns in file and copy to new filepath
#######################
def pattern_replace_and_copy_to_new_filepath(filepath_in, filepath_out, pat_match_and_replace_str_list ) : 
  """Replace all patterns in file and copy to new filepath"""
  
  verify_file_exists(filepath_in)
  pat_list = []

  # format of input data: 
  # pat_match_and_replace_list = [ [ pat0, replace0 ], [ pat1, replace1] ... ]

  # create list of re's of patterns, pre-compiled, also verify each element in the outer list has 2 inner elements list
  # since the first [0] is the pattern to find, and the second [1] is the pattern to replace it with
  for i in range(0, len(pat_match_and_replace_str_list)): 
    curr_pat_and_replace_list  = pat_match_and_replace_str_list[i]
    if len(curr_pat_and_replace_list) != 2 : 
      error_print('Element in list elements must be of size 2 [pat_to_match, str_to_replace_with]')
    the_pat_str = curr_pat_and_replace_list[0]
    pat_list.append(  re.compile(the_pat_str)  )

  # open the input file and write to output file
  fin = open(filepath_in)
  fout = open(filepath_out, "w")
  temp_list = []
  temp_list.append( '' )
  for line_in in fin : 
    temp_list[0] = line_in 

    for k in range(0, len(pat_match_and_replace_str_list)) : 
      the_replace_str = pat_match_and_replace_str_list[k][1]
      temp_list[0] = pat_list[k].sub( the_replace_str , temp_list[0] )

    line_out = temp_list[0]
    fout.write( line_out ) 

  fin.close()
  fout.close()

  verify_file_exists(filepath_out)
  debug_print("COPY and PATTERN replace : " + filepath_in  + " ->[PAT SUB]-> " + filepath_out )


# #######################
# # Remove underscores function
# #######################
# def remove_underscores(in_filename, the_prefix):
#   """Remove underscores from the input file in the form of '_prefix' -> 'prefix'"""
#   # also fix some problems with init_inputscalar by commenting that line out
# 
#   p1 = re.compile('_' + the_prefix)  # understand '_$prefix'
#   str_init_input_scalar = 'ROCCC_init_inputscalar'
#   p2 = re.compile(str_init_input_scalar)  # for commenting out 'ROCCC_init_inputscalar, because has some problems
#   
#   out_filename = in_filename + '.remove_underscores'
#   fout = open(out_filename, "w")
#   fin = open(in_filename)
#   for line_in in fin:
#     line_out = p2.sub(COMMENT+str_init_input_scalar,  p1.sub(the_prefix, line_in))
#     fout.write( line_out )
#   fin.close()
#   fout.close()
#   return out_filename




#######################
#######################
#######################
