#!/usr/bin/env python

# This script compiles the chosen file(s) or directory 

import os
import re
import sys
import getopt

### Define constants ##########################################################
SOURCE_EXTENSION = 'c'
PASS_EXTENSION = 'pass'


# Directory names
LOW_CIRRF_DIRECTORY_NAME = 'low_cirrf_outputs'
CUSTOM_PASS_DIRECTORY_NAME = 'pass_spec_db'

# Filename prefixes and suffixes
CUSTOM_PASS_PREFIX = 'custom_'

##########
# Newer constants for compile_local_job.py
##########

# Executables
PASS_DRIVER = 'pass_driver'
GCC2SUIF = 'gcc2suif'
CMD_COMPILE_C2HICIRRF='compile_c2hicirrf.py'
CMD_COMPILE_HICIRRF2VHDL='compile_hicirrf2vhdl.py'
DIFF_CMD = 'diff'

# Job directories
COMPILE_DIRECTORY_PREFIX = './compile'
COMPILE_DIRECTORY_HICIRRF_SUFFIX = '_hicirrf'
COMPILE_DIRECTORY_LOWCIRRF_SUFFIX = '_lowcirrf'
COMPILE_DIRECTORY_TEST = './test'

# String constants
RUNLOG_FILENAME = 'runlog.log'
ERRORLOG_FILENAME = 'error.log'
TEST_VHDL_FILENAME = 'test.vhd'


LOW_CIRRF_PS_FILENAME = 'low_cirrf_df.ps' # filename mentioned in trunk/roccc-compiler/bin/do_print_dot_graph
LOW_CIRRF_PDF_FILENAME = 'low_cirrf_df.pdf' # filename mentioned in trunk/roccc-compiler/bin/do_print_dot_graph

# Environment variables
CONST_ENV_VAR_ROCCCHOME='ROCCC_HOME'
CONST_ENV_VAR_NCIHOME='NCIHOME'
CONST_ENV_VAR_MACHSUIFHOME='MACHSUIFHOME'

# String constants
CONST_ROCCC_HEADER_FILENAME = 'roccc.h'
CONST_HICIRRF_SUFFIX='-hicirrf.c'
CONST_LOOP_DPT_SUFFIX='.dpt'
CONST_C_SUFFIX='.c'
CONST_PASS_SUFFIX='.pass'


# Error return values, eventuall used for debugging
# to map to string errors
RETURN_DONE=0
ERROR_NONE=0
ERROR_ENV_VAR_UNDEFINED=1
ERROR_PARSE_ARG=2
ERROR_INVALID_ARG_MIX=3
ERROR_CHANGE_DIR=4
ERROR_NON_UNIQUE_FILE=5
ERROR_LOWCIRRF_COMPILE=6
ERROR_HICIRRF_COMPILE=7
ERROR_TEST_NO_DIR=8
ERROR_TEST_NO_FILE=9
ERROR_TEST_VHDL_DIFF_FAIL=10
ERROR_NO_COMPILE_DONE=11

ERROR_SIGINT_KILL=99

# Stage levels
STAGE_UNKNOWN=0
STAGE_C_TO_HICIRRF=1
STAGE_HICIRRF_TO_VHDL=STAGE_C_TO_HICIRRF + 1
STATE_DONE = STAGE_HICIRRF_TO_VHDL + 1

# Random string constants
SPACE=' '
COMMA=','
COLON=':'
SEMICOLON=';'
QUOTE='"'
COMMENT='//'
CURR_DIR='./'
PREV_DIR='../'

# verbose level
VERBOSE_LEVEL=0

# Do graph printing
DO_PRINT_GRAPH=False

# Do synthesis after vhdl generation
DO_SYNTHESIS=False

# Do hicirrf 2 vhdl only
DO_HICIRRF2VHDL_ONLY=False

# HOME_DIR CONSTANT
HOME_DIR=os.getcwd()

### Define functions ##########################################################

#######################
# ERROR Printing
#######################
def error_print(retval, the_string):
  """Effective method for printing an error message and having it return the correct error."""
    
  print 'DEBUG_COMPILE_LOCAL: ERROR(%d): '% retval,
  print the_string
  sys.stdout.flush()
  debug_print('########################################################')
  debug_print('<<<<<<<<<<<<<<<< COMPILE LOCAL JOB FAIL <<<<<<<<<<<<<<<<')
  debug_print('########################################################')
 
  sys.exit(create_success_fail_file(retval))


#######################
# DEBUG Printing
#######################
def debug_print(the_string, level=1):
  """Based on the global variable VERBOSE_LEVEL it will print out the string
    if the VERBOSE_LEVEL exceeds or matches the input level. Useful for debug 
    printing."""

  global VERBOSE_LEVEL 

  if VERBOSE_LEVEL >= level:
    print 'DEBUG_COMPILE_LOCAL: ',
    print the_string
    sys.stdout.flush()


#######################
# Check Environment Variables
#######################
def check_env_vars():
  """Check environment variables to make sure MACHSUIFHOME and NCIHOME are defined"""

  try:
    os.environ[CONST_ENV_VAR_ROCCCHOME]
    os.environ[CONST_ENV_VAR_NCIHOME]
    os.environ[CONST_ENV_VAR_MACHSUIFHOME]
  except:
    print 'ERROR: Environment variable(s) undefined.  Source \'use_suif.sh\' from the ROCCC compiler directory'
    sys.exit(ERROR_ENV_VAR_UNDEFINED)


#######################
# Print help
#######################
def print_help():
  """Print Help for local code"""
  print "\nCompile local directory's contents"
  print "\nUsage: " + sys.argv[0] + "[OPTIONS] [INPUT FILES]"
  print """\
  [COMPILE OPTIONS]:

    -h,--help         
        Display this usage message
  
    -v, --verbose  
        Turn on verbose mode.  For more verbose information, 
        add more -v switches to increase verbosity (eg: -vv).
        Max verbosity listed as 2 for now.

    -g, --graph
        Turn on graph printing from stage before vhdl generation.
        Output should be called low_cirrf.pdf and low_cirrf.ps.

    --job
        Job Id.  Used for debugging for process_one_job.py script. Prints current
        job number to screen. 

    --synthesis
        Enable synthesis after vhdl generation.  Useful for error checking.

  [INPUT FILES and Options]
   
    Compilation requires the input *.c file and an option *.pass file.  The input
    is compiled down to *-hicirrf.c (with compile_c2hicirrf.py) after that a lowering
    set of passes takes it down to vhdl.

    --hicirrf
      If this option is specified then the c2hicirrf compilation is skipped and 
      compilation starts at the hicirrf2vhdl level where the argument is assumed
      to be the *-hicirrf.c input file. 

  """

#######################
# Argument Processing
#######################
def parse_args(local_argv=None):
  """Parse Arguments to find File Options and working directory"""

  global VERBOSE_LEVEL 
  global DO_PRINT_GRAPH
  global DO_HICIRRF2VHDL_ONLY
  global DO_SYNTHESIS

  if local_argv is None:
    local_argv = sys.argv
    
  # parse command line options
  try:
    opts, args = getopt.gnu_getopt(local_argv[1:], "hvg", ["help", "verbose", "graph", "hicirrf", "job=", "synthesis"])
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

    elif o in ("-g", "--graph"):
      DO_PRINT_GRAPH = True

    elif o == "--hicirrf":
      DO_HICIRRF2VHDL_ONLY = True

    elif o == "--job": 
      debug_print('Job # ' + a)

    elif o == "--synthesis":
      DO_SYNTHESIS = True

    
  if len(args) < 1 or len(args) > 2: 
    error_print('A correct input file argument must be defined.  Try --help')

  if DO_HICIRRF2VHDL_ONLY:
    hicirrf_filename = args[0]
    if not( len(args) == 1 and re.match('.*[.]c$', hicirrf_filename) ):
      error_print(ERROR_PARSE_ARG, 'For hicirrf option, 1 argument must be specified and must have an extension of *.c')

    c_filename = None
    pass_filename = None
  else:
    hicirrf_filename = None
    for i in args:
      if re.match('.*[.]c$', i):
        c_filename = i 
      if re.match('.*[.]pass$', i):
        pass_filename = i

    try:
      c_filename
    except:
      c_filename = None
    try:
      pass_filename
    except:
      pass_filename = None

    # both c_file and pass_file must be defined for now, will fix in future
    if c_filename == None : #or pass_filename == None:
      error_print(ERROR_PARSE_ARG, 'A *.c (and maybe a *.pass) file must be defined as an argument')

  dir_path = os.getcwd()

  ######
  # Error checking to make sure the options are valid
  ######

  # verify all paths are correct
  assert os.path.isdir(dir_path), 'Cannot access directory ' + dir_path

  if c_filename != None:
    assert os.path.isfile(c_filename), 'Cannot access file ' + c_filename

  if pass_filename != None:
    assert os.path.isfile(pass_filename), 'Cannot access file ' + pass_filename

  if hicirrf_filename != None:
    assert os.path.isfile(hicirrf_filename), 'Cannot access file ' + hicirrf_filename


  return (c_filename, pass_filename, hicirrf_filename, dir_path)

#######################
# Create compile directories
#######################
def create_compile_directory(compile_stage):
  """Create compile directories.  If they don't exist then create them, otherwise leave alone"""

  dir_compile_hicirrf  = COMPILE_DIRECTORY_PREFIX + COMPILE_DIRECTORY_HICIRRF_SUFFIX
  dir_compile_lowcirrf = COMPILE_DIRECTORY_PREFIX + COMPILE_DIRECTORY_LOWCIRRF_SUFFIX

  if compile_stage == STAGE_C_TO_HICIRRF : 
    if not os.path.isdir(dir_compile_hicirrf):
      os.mkdir(dir_compile_hicirrf)
    output_dir = dir_compile_hicirrf

  elif compile_stage == STAGE_HICIRRF_TO_VHDL :
    if not os.path.isdir(dir_compile_lowcirrf):
      os.mkdir(dir_compile_lowcirrf)
    output_dir = dir_compile_lowcirrf

  else:
    output_dir = None

  debug_print('Create compile directory: ' + output_dir)

  return output_dir

#######################
# Create SUCESS/FAIL file
#######################
def create_success_fail_file(retval):
  """Creates success or failure file with final return value"""

  last_dir = os.getcwd()
  os.chdir(HOME_DIR)

  SUCCESS_FILENAME = 'SUCCESS'
  FAILURE_FILENAME = 'FAILED'
  cmd_create_prefix = 'echo %d > ' % retval
  if retval == 0:
    cmd_create = cmd_create_prefix + SUCCESS_FILENAME
  else :
    cmd_create = cmd_create_prefix + FAILURE_FILENAME
  debug_print(cmd_create)
  retval_os =  os.WEXITSTATUS( os.system(cmd_create) )

  if retval_os != 0 :
    debug_print('Error creating success/failure filename for retval(%d)' % retval_os)
  

  os.chdir(last_dir)

  return retval

###########################################################
### Begin program #########################################
###########################################################
if __name__ == "__main__":

  FINAL_RETURN_VALUE = ERROR_NO_COMPILE_DONE  

  # ensure environment variables are set because we can call the compiled programs
  # without having to reference the entire path
  check_env_vars()

  # parse from command line
  pa_c_filename, pa_pass_filename, pa_hicirrf_filename, dir_path = parse_args()

  # Set a verbose string for passing to lower scripts 
  verbose_str_list = [ ]
  for i in range(VERBOSE_LEVEL-1):
    if len(verbose_str_list) == 0 : 
      verbose_str_list.append('-v')
    else:
      verbose_str_list.append('v')
  verbose_str = ''.join(verbose_str_list)



  debug_print('########################################################') 
  debug_print('>>>>>>>>>>>>>>>> COMPILE LOCAL JOB START >>>>>>>>>>>>>>>>')
  debug_print('########################################################') 
  debug_print('VERBOSE level = %d : ' % (VERBOSE_LEVEL) + verbose_str )
  debug_print('DO GRAPH OUTPUT = %d' % DO_PRINT_GRAPH)
  debug_print('DO SYNTHESIS = %d' % DO_SYNTHESIS)
  debug_print("dirpath = ", dir_path)

  # determine compile stage and set filename if appropriate
  compile_stage = STAGE_C_TO_HICIRRF
  if( DO_HICIRRF2VHDL_ONLY ):
    compile_stage = STAGE_HICIRRF_TO_VHDL
    hicirrf_filename = pa_hicirrf_filename

  debug_print("c_filename = '%s'" % pa_c_filename )
  debug_print("pass_filename = '%s'" % pa_pass_filename )
  debug_print("hicirrf-filename = '%s'" % pa_hicirrf_filename )
  debug_print("dir-path = '%s'" % dir_path )
  debug_print("compile stage = %d" % compile_stage )

  # At this point in this file, we know 3 things
  # 1) We are in the working directory
  # 2) We know what stage of compilation (c2hicirrf or hicirrf2vhdl)
  # 3) Given the appropriate stage we know the files that are
  #    to be processed in each stage

  ######################################
  ### c to hicirrf compilation stage ###
  ######################################
  if compile_stage == STAGE_C_TO_HICIRRF :

    debug_print('############################################')
    debug_print('Performing C to HiCirrf Optimization Passes:')
    debug_print('############################################')

    # create compile directory for this stage
    hicirrf_compile_dir = create_compile_directory(compile_stage)


    c_filename = pa_c_filename
    pass_filename = pa_pass_filename

    # c_filename and pass_filename are defined
    c_filename_prefix = os.path.splitext(c_filename)[0]


    # copy files to compile directory for hicirrf, and compile code
    cmd_cp_c_file = 'cp "' + c_filename + '" ' + hicirrf_compile_dir
    debug_print(cmd_cp_c_file)
    os.system(cmd_cp_c_file)
    
    if pass_filename != None:
      cmd_cp_pass_file = 'cp "' + pass_filename + '" ' + hicirrf_compile_dir
      debug_print(cmd_cp_pass_file)
      os.system(cmd_cp_pass_file)

    # CD into that compile directory and start execution
    debug_print('Change directory into ' + hicirrf_compile_dir)
    os.chdir(hicirrf_compile_dir);

    # Assert that files exist
    assert os.path.isfile(c_filename), 'Cannot access file ' + c_filename
    if pass_filename != None:
      assert os.path.isfile(pass_filename), 'Cannot access file ' + pass_filename

    # Execute
    if pass_filename != None: 
      cmd_execute_c2hicirrf = CMD_COMPILE_C2HICIRRF + SPACE + verbose_str + SPACE + c_filename + SPACE + pass_filename
    else:
      cmd_execute_c2hicirrf = CMD_COMPILE_C2HICIRRF + SPACE + verbose_str + SPACE + c_filename

    debug_print('EXECUTE: ' + cmd_execute_c2hicirrf)
    retval = os.WEXITSTATUS( os.system(cmd_execute_c2hicirrf) )

    if retval != 0 : 
      if retval == ERROR_SIGINT_KILL : 
        error_print(ERROR_SIGINT_KILL, "USER ORDERED KILL (%d)" % retval );
      error_print(ERROR_HICIRRF_COMPILE, 'Error compiling c to hicirrf (retval = %d)' % retval )

    # copy back hicirrf file
    output_hicirrf_filename = c_filename_prefix + CONST_HICIRRF_SUFFIX
    output_header_filename = CONST_ROCCC_HEADER_FILENAME

    if not os.path.isfile(output_hicirrf_filename):
      error_print(ERROR_HICIRRF_COMPILE, 'ERROR: ' + hicirrf_filename + ' was not produced in compilation')

    cmd_cp = 'cp' + SPACE + output_hicirrf_filename + SPACE + '../' 
    debug_print(cmd_cp)
    os.system(cmd_cp)

    if os.path.isfile(output_header_filename):
      cmd_cp = 'cp' + SPACE + output_header_filename + SPACE + '../'
      os.system(cmd_cp)

    # go back to rood directory for compilation
    os.chdir('..')

    # last thing to do is set the predicator to next stage
    compile_stage += 1

    hicirrf_filename = output_hicirrf_filename

  # End C_TO_HICIRRF COMPILE STAGE
  ######################################


  ##########################################
  ### lowcirrf to vhdl compilation stage ###
  ##########################################

  if compile_stage == STAGE_HICIRRF_TO_VHDL :

    debug_print('###################################')
    debug_print('Performing HICIRRF to VHDL stage...')
    debug_print('###################################')

    # create compile directory for this stage
    lowcirrf_compile_dir = create_compile_directory(compile_stage)

    # hicirrf filename already defined
    hicirrf_prefix = os.path.splitext(hicirrf_filename)[0]

    # copy file to compile directory for lowcirrf
    cmd_cp_hicirrf_file = 'cp' + SPACE + hicirrf_filename + SPACE + lowcirrf_compile_dir
    debug_print(cmd_cp_hicirrf_file)
    os.system(cmd_cp_hicirrf_file)

    # if exists, then copy over
    if os.path.isfile(CONST_ROCCC_HEADER_FILENAME):
      cmd_cp_roccc_file = 'cp' + SPACE + CONST_ROCCC_HEADER_FILENAME + SPACE + lowcirrf_compile_dir
      debug_print(cmd_cp_roccc_file)
      os.system(cmd_cp_roccc_file)

    # CD into that compile directory and start execution
    debug_print('Change directory into ' + lowcirrf_compile_dir)
    os.chdir(lowcirrf_compile_dir);

    # Assert that files exist
    assert os.path.isfile(hicirrf_filename), 'Cannot access file ' + hicirrf_filename

    # compile code from hicirrf downto vhdl
    if DO_PRINT_GRAPH : 
      PRINT_DOT_GRAPH_OPTIONS_STR = '--graph' + SPACE
    else: 
      PRINT_DOT_GRAPH_OPTIONS_STR = ''

    if DO_SYNTHESIS : 
      SYNTHESIS_OPTIONS_STR = '--synthesis' + SPACE
    else:
      SYNTHESIS_OPTIONS_STR = ''

    #cmd_execute_hicirrf2vhdl = CMD_COMPILE_HICIRRF2VHDL + SPACE + verbose_str + SPACE + '--graph' + SPACE + hicirrf_filename
    cmd_execute_hicirrf2vhdl = CMD_COMPILE_HICIRRF2VHDL + SPACE + verbose_str + SPACE + \
                               PRINT_DOT_GRAPH_OPTIONS_STR + SYNTHESIS_OPTIONS_STR + \
                               hicirrf_filename

    debug_print('EXECUTE: ' + cmd_execute_hicirrf2vhdl)
    retval = os.WEXITSTATUS( os.system(cmd_execute_hicirrf2vhdl) )

    if retval != 0 :
      if retval == ERROR_SIGINT_KILL : 
        error_print(ERROR_SIGINT_KILL, "USER ORDERED KILL (%d)" % retval );
      error_print(ERROR_LOWCIRRF_COMPILE, "Can't compile %s to vhdl" % hicirrf_filename)

    # copy back the vhdl file to root directory.  It's the compiled code
    cmd_copy_vhdl_to_root = 'cp -v *.vhd ../'
    debug_print(cmd_copy_vhdl_to_root)
    os.system(cmd_copy_vhdl_to_root)

    if DO_PRINT_GRAPH:
      cmd_copy_graphs = 'cp -v ' + LOW_CIRRF_PS_FILENAME + SPACE + LOW_CIRRF_PDF_FILENAME + SPACE + '../'
      debug_print(cmd_copy_graphs)
      os.system(cmd_copy_graphs)
      
 
    # go back to rood directory for compilation
    os.chdir('..')

    FINAL_RETURN_VALUE = ERROR_NONE

  # End LOWCIRRF_TO_VHDL COMPILE STAGE
  ######################################

  debug_print('########################################################') 
  debug_print('<<<<<<<<<<<<<<<< COMPILE LOCAL JOB DONE <<<<<<<<<<<<<<<<')
  debug_print('########################################################') 

  sys.exit(create_success_fail_file(FINAL_RETURN_VALUE))
