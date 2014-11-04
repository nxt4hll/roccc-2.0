#!/usr/bin/env python

# This script compiles set of test directories.  At $ROCCC_HOME/../code-repository/ 
# We Run the script on each of the directories listed in the COMPILE_DIR_LIST
# All errors and successes are reported

import os
import signal
import re
import sys
import getopt

import time
import datetime

### Define constants ##########################################################

# Random string constants
SPACE=' '
COMMA=','
COLON=':'
SEMICOLON=';'
QUOTE='"'
COMMENT='//'
CURR_DIR='./'
PREV_DIR='../'

### Global Variables ###
# verbose level
VERBOSE_LEVEL=0

# boolean do clean compile
DO_CLEAN_COMPILE=True

# Do Synthesis
DO_SYNTHESIS=False

# SIGNAL QUIT
DO_SIGNAL_QUIT=False

### END Global Variables ###


# Error return values, eventuall used for debugging
# to map to string errors
RETURN_DONE=0
ERROR_NONE=0
ERROR_ENV_VAR_UNDEFINED=1
ERROR_PARSE_ARG=2
ERROR_PARSE_DIR=3
ERROR_REMOVE_NEW_DIR=4
ERROR_COMPILE_DIR=5

ERROR_SIGINT_KILL=99

# Compile script
COMPILE_SCRIPT='compile_c2hicirrf2vhdl.py' 

# NEW DIRECTORY FOR COMPILES
NEW_DIR_PREFIX = '__test_compile_dir'

# STRING CONSTANTS
COMPILE_DIR_LIST_NAME = 'COMPILE_DIR_LIST'

CONST_C_SUFFIX='.c'
CONST_PASS_SUFFIX='.pass'


########################
# Check Environment Variables
#######################
#Check environment variables to make sure MACHSUIFHOME and NCIHOME are defined

# Environment variables
CONST_ENV_VAR_ROCCCHOME='ROCCC_HOME'
CONST_ENV_VAR_NCIHOME='NCIHOME'
CONST_ENV_VAR_MACHSUIFHOME='MACHSUIFHOME'

try:
  os.environ[CONST_ENV_VAR_ROCCCHOME]
  os.environ[CONST_ENV_VAR_NCIHOME]
  os.environ[CONST_ENV_VAR_MACHSUIFHOME]
except:
  print 'ERROR: Environment variable(s) undefined.  Source \'use_suif.sh\' from the ROCCC compiler directory'
  sys.exit(ERROR_ENV_VAR_UNDEFINED)
#######################
#######################


SVN_CODE_REPO_DIR = os.environ[CONST_ENV_VAR_ROCCCHOME] + '/../code-repository'
HOME_DIR = os.getcwd();

### Define functions ##########################################################

#######################
# ERROR Printing
#######################
def error_print(retval, the_string):
  """Effective method for printing an error message and having it return the correct error."""
    
  print 'DEBUG_COMPILE_TEST_DIR: ERROR(%d): '% retval,
  print the_string
  sys.stdout.flush()
  debug_print('########################################################')
  debug_print('<<<<<<<<<<<<<<<< COMPILE TEST DIR FAIL <<<<<<<<<<<<<<<<')
  debug_print('########################################################')
 
  sys.exit(retval)


#######################
# DEBUG Printing
#######################
def debug_print(the_string, level=1):
  """Based on the global variable VERBOSE_LEVEL it will print out the string
    if the VERBOSE_LEVEL exceeds or matches the input level. Useful for debug 
    printing."""

  global VERBOSE_LEVEL 

  if VERBOSE_LEVEL >= level:
    #print 'DEBUG_COMPILE_TEST_DIR: [DIR=' + os.getcwd() + ']' 
    print 'DEBUG_COMPILE_TEST_DIR: ',
    print the_string
    sys.stdout.flush()

#######################
# Argument Processing
#######################
def print_help():
  """Print Help for local code"""
  print "\nCompile directory's contents to do a test "
  print "\nUsage: " + sys.argv[0] + " [OPTIONS]"
  print """\
    This code will compile a set of directories located at
    $ROCCC_HOME/../../code-repository. In that directory lists
    the compile directories (local to the code repository directory)
    that must be compiled. 
   
    This script will copy all that code locally to $PWD/__test_compile_dir+$DATA
    and compile each one with the script compile_c2hicirrf2vhdl.py.  It will
    pass in the file arguments by parisng the names of the files looking for
    one *.c and one *.pass file. 

    This script will keep a tally of which ones failed and passed
    and print the output at the end of compilation.  The return value
    is the number of directories that failed. 

  [COMPILE OPTIONS]:

    -h,--help         
        Display this usage message
  
    -v, --verbose  
        Turn on verbose mode.  For more verbose add another -v. 
        (eg. -vv)  Max is 3.

    --do-not-remove
        Tells test script to not remove temporary compilation directory (for debugging
        purposes).

    --synthesis
        Tells it to run the addition syntheis option after vhdl generation.

  """


def parse_args(local_argv=None):
  """Parse Arguments to find File Options and working directory"""

  global VERBOSE_LEVEL 
  global DO_CLEAN_COMPILE
  global DO_SYNTHESIS

  if local_argv is None:
    local_argv = sys.argv
    
  # parse command line options
  try:
    opts, args = getopt.gnu_getopt(local_argv[1:], "hv", ["help", "verbose", "do-not-remove", "synthesis"])
  except getopt.error, msg:
    print msg
    print "For help use --help"
    sys.exit(ERROR_PARSE_ARG)

  # process options
  for o, a in opts:
    #print "OPTS: '%s' : '%s'" % (o, a)  

    if o in ("-h", "--help"):
      print_help()
      sys.exit(RETURN_DONE)
    
    elif o in ("-v", "--verbose"):
      VERBOSE_LEVEL += 1

    elif o == "--do-not-remove":
      DO_CLEAN_COMPILE=False

    elif o == "--synthesis":
      DO_SYNTHESIS = True

#######################
# Parse directory
#######################
def parse_local_dir(local_dir_path=None):
  """Parse directory, determine from filenames which files to pass to compiler"""
  
  assert os.path.isdir(local_dir_path), 'Cannot access directory ' + local_dir_path

  # Now that the we're in the right directory find all appropriate files
  c_file_count = 0
  c_file_list = [ ]
  pass_file_count = 0
  pass_file_list = [ ]

  files = os.listdir(local_dir_path);
  for file in files:
    (prefix, ext) = os.path.splitext(file)

    if ext ==  CONST_PASS_SUFFIX: 
      pass_file_count = pass_file_count + 1
      pass_file_tuple = (file, prefix, ext)
      pass_file_list.append(pass_file_tuple)

    elif ext == CONST_C_SUFFIX:
      c_file_count = c_file_count + 1
      c_file_tuple = (file, prefix, ext)
      c_file_list.append(c_file_tuple)

      #if c_file_count > 1:
      #  print "ERROR: more than one c file (non hicirrf) in directory" 
      #  sys.exit(ERROR_NON_UNIQUE_FILE)


  c_index = 0
  for c_file_tuple in c_file_list : 
    c_file = c_file_tuple[0]
    debug_print("parse-dir-c-file[%d] = '%s'" % (c_index, c_file) )
    c_index = c_index + 1
    if c_file != None and not os.path.isfile(c_file):
      error_print(ERROR_PARSE_DIR, 'Could not parse the c-file in dir: ' + local_dir_path)

  pass_index = 0
  for pass_file_tuple in pass_file_list : 
    pass_file = pass_file_tuple[0]
    debug_print("parse-dir-pass-file[%d] = '%s'" % (pass_index, pass_file) )
    pass_index = pass_index + 1
    if pass_file != None and not os.path.isfile(pass_file):
      error_print(ERROR_PARSE_DIR, 'Could not parse the pass-file in dir: ' + local_dir_path)

  # make sure that we follow the following restrictions
  # at minimum 1 c_file with zero or more pass files
  # if more than 1 cfile, then ONE ore more pass files, no-opt is not supported for only one file for now
  # so just use an empty file
  if len(c_file_list) == 0 : 
    error_print(ERROR_PARSE_DIR, 'At least one input c file must be located in ' + os.getcwd() + ', none found')
  elif len(c_file_list) > 1 and len(pass_file_list) == 0 : 
    error_print(ERROR_PARSE_DIR, 'For now, test script does not support compilation of many c-files without at least one pass file ')

  return c_file_list, pass_file_list

#######################
# Compile Dir
#######################
def compile_local_dir(c_file, pass_file, verbose_str):

  global DO_SYNTHESIS

  if not os.path.isfile(c_file):
    error_print(ERROR_COMPILE_DIR, 'Can not find c_file(%s) in local dir(%s)' % (c_file, os.getcwd()))
  if pass_file != None and ( not os.path.isfile(pass_file)):
    error_print(ERROR_COMPILE_DIR, 'Can not find pass_file(%s) in local dir(%s)' % (pass_file, os.getcwd()))

  if pass_file != None:
    PASS_FILE_OPTION_STR = SPACE + pass_file
  else:
    PASS_FILE_OPTION_STR = ''

  if DO_SYNTHESIS : 
    SYNTHESIS_OPTION_STR = '--synthesis' + SPACE
  else : 
    SYNTHESIS_OPTION_STR = ''
  
  cmd_call_compile = COMPILE_SCRIPT + SPACE + verbose_str + SPACE + \
                     SYNTHESIS_OPTION_STR + \
                     c_file + PASS_FILE_OPTION_STR

  debug_print(cmd_call_compile)
  retval = os.WEXITSTATUS(os.system(cmd_call_compile))
  if retval != 0 :
    if retval == ERROR_SIGINT_KILL : 
      error_print(ERROR_SIGINT_KILL, "USER ORDERED KILL (%d)" % retval );
    debug_print('Execution failed (retval=%d)\n\n' %  retval)

  return retval


###########################################################
### Begin program #########################################
###########################################################
if __name__ == "__main__":

  parse_args()

  verbose_str_list = [ ]
  for i in range(VERBOSE_LEVEL-1):
    if len(verbose_str_list) == 0 : 
      verbose_str_list.append('-v')
    else:
      verbose_str_list.append('v')
  verbose_str = ''.join(verbose_str_list)

  debug_print('#############################################')
  debug_print('VERBOSE level = %d' % VERBOSE_LEVEL)
  debug_print('VERBOSE string = %s' % verbose_str)
  debug_print("code-repo-dir = " +  SVN_CODE_REPO_DIR )
  debug_print("home-dir = " + HOME_DIR )


  ### Create new test dir
  today_str = datetime.datetime.now().strftime('_%Y%b%d%a%H%M%S')
  NEW_DIR_NAME = NEW_DIR_PREFIX + today_str
  NEW_CODE_REPO_COMPILE_DIR = HOME_DIR + '/' + NEW_DIR_NAME
  debug_print("NewDirName = " + NEW_CODE_REPO_COMPILE_DIR )

  if not os.path.isdir(NEW_CODE_REPO_COMPILE_DIR) :
    os.mkdir(NEW_CODE_REPO_COMPILE_DIR) 
    assert os.path.isdir(NEW_CODE_REPO_COMPILE_DIR)


  # Copy contents of SVN to new test dir
  cmd_cp_to_local = 'cp -rf ' + SVN_CODE_REPO_DIR + '/* ' + NEW_CODE_REPO_COMPILE_DIR
  debug_print(cmd_cp_to_local)
  os.system(cmd_cp_to_local)

  # READ the instructor which tells which directories to compile
  #cmd_cd_to_compile_dir = 'cd ' + NEW_CODE_REPO_COMPILE_DIR
  #debug_print(cmd_cd_to_compile_dir)
  #os.chdir(cmd_cp_to_compile_dir)

  the_dirs_process_list = [ ]
  job_list = []

  # Retrieve from the compile list, which run to test
  fin = open(NEW_CODE_REPO_COMPILE_DIR + '/' + COMPILE_DIR_LIST_NAME)
  for line_in in fin:
    if not re.match('^#', line_in ):
      dir_name = line_in.strip()
      if dir_name != '':
        debug_print('Appending test dir : ' + dir_name) 
        the_dirs_process_list.append( dir_name )
  fin.close()

  # process each job that was read from the dir list
  for dir_path in the_dirs_process_list : 
    full_dir_path = NEW_CODE_REPO_COMPILE_DIR + '/' + dir_path

    debug_print('PROCESS: ' + full_dir_path)
    os.chdir(full_dir_path)
    c_file_list, pass_file_list = parse_local_dir('.')

    # if has only one c file (and one or zero pass files) then compile without making
    # a new directory
    if len(c_file_list) == 1 and ( len(pass_file_list) == 1 or len(pass_file_list) == 0 ) :
      c_file = (c_file_list[0])[0]
      if len(pass_file_list) == 1: 
        pass_file = (pass_file_list[0])[0]
      else : 
        pass_file = None

      retval = compile_local_dir(c_file, pass_file, verbose_str)
      t = (dir_path, retval)
      job_list.append(t)

    # if has more than one file or more than one c_file then make directories
    # based on the prefixes of the filenames ( c_file_prefix-pass_file_prefix)
    else : 
      for c_file_tuple in c_file_list : 

        c_file = c_file_tuple[0]
        c_file_prefix = c_file_tuple[1]

        for pass_file_tuple in pass_file_list:
        
          pass_file = pass_file_tuple[0]
          pass_file_prefix = pass_file_tuple[1]

          new_dir_prefix = c_file_prefix + '-' + pass_file_prefix

          # create full pass prefix path
          full_dir_pass_prefix_path = full_dir_path + '/' + new_dir_prefix
          if not os.path.isdir(full_dir_pass_prefix_path ):
            debug_print('mkdir: ' + full_dir_pass_prefix_path)
            os.mkdir(full_dir_pass_prefix_path)
            assert os.path.isdir(full_dir_pass_prefix_path)

          # copy the c_file and pass_file to that new directory and compile
          cmd_cp = 'cp ' + c_file + SPACE + pass_file + SPACE +  full_dir_pass_prefix_path
          debug_print(cmd_cp)
          os.system(cmd_cp)
        
          last_dir = os.getcwd()
          debug_print('CHDIR to ' + full_dir_pass_prefix_path)
          os.chdir(full_dir_pass_prefix_path)

          retval = compile_local_dir(c_file, pass_file, verbose_str)

          t = (dir_path + '/' + new_dir_prefix, retval)
          job_list.append( t )

          os.chdir( last_dir )

    

  debug_print('CD to HOME: ' + HOME_DIR)
  os.chdir(HOME_DIR)

  if DO_CLEAN_COMPILE : 
    debug_print('Remove temp compile directory')
    cmd_rm_compile_dir = 'rm -rf ' + NEW_CODE_REPO_COMPILE_DIR
    debug_print(cmd_rm_compile_dir)
    retval = os.system(cmd_rm_compile_dir)
    if os.WEXITSTATUS(retval) != 0 : 
      error_print(ERROR_REMOVE_NEW_DIR, 'Remove of directory ' + NEW_CODE_REPO_COMPILE_DIR + ' failed')

  debug_print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')
  debug_print('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%')

  FAIL_CNT = 0
  SUCC_CNT = 0
  for _dir,_retval in job_list:
    if _retval == 0 : 
      debug_print('PASS : ' + _dir, 0)
      SUCC_CNT = SUCC_CNT + 1
    else:
      debug_print('FAIL : ' + _dir, 0)
      FAIL_CNT = FAIL_CNT + 1
 
  debug_print('Total compiled = %d' % len(job_list), 0)
  debug_print('Passed # = %d' % SUCC_CNT, 0)
  debug_print('Failed # = %d' % FAIL_CNT, 0)

 
  debug_print('#############################################')
  sys.exit(FAIL_CNT)
