#!/usr/bin/env python

# This script compiles the chosen file(s) or directory 

import os
import re
import sys
import getopt

### Define constants ##########################################################


# Environment variables
CONST_ENV_VAR_ROCCCHOME='XILINX'


# Error return values, eventuall used for debugging
# to map to string errors
RETURN_DONE=0
ERROR_NONE=0
ERROR_ENV_VAR_UNDEFINED=1
ERROR_PARSE_ARG=2
ERROR_SYNTHESIS=3

ERROR_SIGINT_KILL=99

# Stage levels
STAGE_UNKNOWN=0
STAGE_HICIRRF_TO_VHDL=1

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

STR_SYNTHESIS_DIRECTIVES_START  ='SYNTHESIS_DIRECTIVES_START'
STR_SYNTHESIS_DIRECTIVES_END    ='SYNTHESIS_DIRECTIVES_END'

STR_SYNTHESIS_OPTIONS_START ='SYNTHESIS_OPTIONS_START'
STR_SYNTHESIS_OPTIONS_END   ='SYNTHESIS_OPTIONS_END'

STR_TOP_ENTITY              = 'TOP_ENTITY'
STR_FILE_VHDL_SYNTHESIZE    = 'FILE_VHDL_SYNTHESIZE'
STR_FILE_VHDL_EXTRA         = 'FILE_VHDL_EXTRA'
STR_FILE_SW                 = 'FILE_SW'
STR_COMMENT                 = '#'

######
#PIPE2STDOUT = SPACE + '2>&1'

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
VHDL_FILES        = 'VHDL_FILES'
SYNTHESIS_OPTIONS = 'SYNTHESIS_OPTIONS'
TOP_ENTITY        = 'TOP_ENTITY'

SYNTHESIS_DIR     = './SYNTHESIS_DIR'

# verbose level
VERBOSE_LEVEL=2

# Do graph printing
DO_SYNTHESIS_ONLY=False



### Define functions ##########################################################

#######################
# ERROR Printing
#######################
def error_print(retval, the_string):
  """Effective method for printing an error message and having it return the correct error."""
    
  print 'DEBUG_COMPILE_VHDL_TO_SYNTHESIS: ERROR(%d): '% retval,
  print the_string
  sys.stdout.flush()
  debug_print('########################################################') 
  debug_print('<<<<<< COMPILE LOCAL (HICIRRF TO VHDL) JOB FAIL <<<<<<')
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
    print 'DEBUG_COMPILE_VHDL_TO_SYNTHESIS: ' + the_string
    sys.stdout.flush()


#######################
# Check Environment Variables
#######################
def check_env_vars():
  """Check environment variables to make sure MACHSUIFHOME and NCIHOME are defined"""

  try:
    os.environ[CONST_ENV_VAR_ROCCCHOME]
  except:
    error_print(ERROR_ENV_VAR_UNDEFINED, 'ERROR: Environment variable(s) undefined.  Source XILINX settings file')


#######################
# Argument Processing
#######################
def print_help():
  """Print Help for local code"""
  print "\nCompile local directory's contents"
  print "\nUsage: " + sys.argv[0] + "[OPTIONS] [ARG]"
  print """\
  [COMPILE OPTIONS]:

    -h,--help         
        Display this usage message
  
    -v, --verbose  
        Turn on verbose mode.  For more verbose information, 
        add more -v switches to increase verbosity (eg: -vv).
        Max verbosity listed as 2 for now.

    -q, --quiet
        Turn off verbose printing.  This option has precidence over the 
        verbosity (-v) option

    --synthesis-only
        Do only the 'xst' portion of synthesis, do not MAP or PLACE & ROUTE.
        Good if you only want to check syntax and the like, but also make
        sure the structure of the vhdl is sound (but don't want timing).

  [ARG]: 
    You may specifiy the directives file or not specify one at all 
    (default = 'DIRECTIVES.dat')


    The directives files has the information of what needs to be compiled 
    within the current directory, the top level entity, and the necessary
    options to run during synthesis.  A format example is: 

    START
    # comments
    <FILETYPE> <filename>
    ...
    TOP <top_level_entity_name>
    ...
    SYNTHESIS_OPTIONS_START
    <all synthesis options verbatime, including top level >
    ...
    SYNTHESIS_OPTIONS_END

    END

    This script will take that above commands to execute the appropriate
    xilinx compilation 


  """


def parse_args(local_argv=None):
  """Parse Arguments to find File Options and working directory"""

  global VERBOSE_LEVEL 
  global DO_SYNTHESIS_ONLY

  if local_argv is None:
    local_argv = sys.argv
    
  # parse command line options
  try:
    opts, args = getopt.gnu_getopt(local_argv[1:], "hvq", ["help", "verbose", "synthesis-only", "quiet"])
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
      # allows the quiet option to overwrite this
      if VERBOSE_LEVEL > 0 : 
        VERBOSE_LEVEL += 1

    elif o in ("-q", "--quiet"): 
      VERBOSE_LEVEL = 0

    elif o == "--synthesis-only":
      DO_SYNTHESIS_ONLY = True


  if len(args) == 0 : 
    full_filename = 'DIRECTIVES.dat'
     
  elif len(args) != 1 : 
    error_print(ERROR_PARSE_ARG, "Must specify only 1 input directives file")

  else : 
    full_filename = args[0]

  ######
  # Error checking to make sure the options are valid
  ######


  if not os.path.isfile(full_filename):
    error_print(ERROR_PARSE_ARG, 'File \"' + full_filename + '\" does not exist')


  return full_filename


#######################
# Read directives script
#######################
def read_directives(in_filename):
  """Read synthesis directives from file"""

  directives_map = { }
  directives_map[VHDL_FILES]        = [ ]
  directives_map[SYNTHESIS_OPTIONS] = [ ]

  #pat_directives_start    = re.compile(STR_SYNTHESIS_DIRECTIVES_START)
  #pat_directives_end      = re.compile(STR_SYNTHESIS_DIRECTIVES_END)

  pat_synth_option_start  = re.compile(STR_SYNTHESIS_OPTIONS_START)
  pat_synth_option_end    = re.compile(STR_SYNTHESIS_OPTIONS_END)

  pat_top_entity          = re.compile(STR_TOP_ENTITY)
  pat_vhdl_synth_file     = re.compile(STR_FILE_VHDL_SYNTHESIZE)
  #pat_vhdl_extra          = re.compile(STR_FILE_VHDL_EXTRA)
  #pat_file_sw             = re.compile(STR_VHDL_SYNTHESIZE)
  pat_comment             = re.compile(STR_COMMENT)

  state_other = 1
  state_get_options = 2

  curr_state = state_other

  fin = open(in_filename)
  for line_in in fin: 
    line_in_split = line_in.split()

    if not pat_comment.search( line_in ) : 
      if curr_state == 1 : 
        if pat_top_entity.search(line_in) : 
          directives_map[TOP_ENTITY] =  line_in_split[1]

        elif pat_vhdl_synth_file.search(line_in) : 
          directives_map[VHDL_FILES].append( line_in_split[1] )

        elif pat_synth_option_start.search(line_in) : 
          curr_state = state_get_options

      elif curr_state == 2: 
        # if we reach the ned then just go to new state
        if pat_synth_option_end.search(line_in): 
          curr_state = state_other

        # otherwise just assume this is the options line
        else : 
          directives_map[SYNTHESIS_OPTIONS].append( line_in )

  fin.close()

  return directives_map


###########################################################
### Begin program #########################################
###########################################################
if __name__ == "__main__":

  # ensure environment variables are set because we can call the compiled programs
  # without having to reference the entire path
  check_env_vars()

  directives_filename = parse_args()
  directives_map = read_directives(directives_filename)

  last_dir = os.getcwd();


  top_entity_name = directives_map[TOP_ENTITY]


  if not os.path.isdir(SYNTHESIS_DIR):
    os.mkdir(SYNTHESIS_DIR)

  os.chdir(SYNTHESIS_DIR)
 

  # write the project files
  fout_prj = open( top_entity_name + '.prj', 'w' )
  for val in directives_map[VHDL_FILES]: 
    fout_prj.write( 'vhdl work "../' + val + '"\n' )
  fout_prj.close();

  fout_scr = open( top_entity_name + '.scr', 'w' )
  for val in directives_map[SYNTHESIS_OPTIONS]: 
    fout_scr.write( val )
  fout_scr.close();

  ##############################
  # run synthesis in this directory
  ##############################

  # will only use this for XST since it can be used for testing
  if( VERBOSE_LEVEL < 2 ) : 
    cmd_pipe_to_file = ' >& SYNTHESIS_REPORT.LOG'
  else:
    cmd_pipe_to_file = ''

  ##### XST COMPILATION ####
  # output is top_entity_name.ngc
  cmd_xst = 'xst -ifn' + SPACE + top_entity_name + '.scr' + SPACE + \
                   '-ofn' + SPACE + top_entity_name + '.syr'
  
  debug_print('#######')
  debug_print('EXECUTE : ' + cmd_xst)
  debug_print('#######')
  retval = os.system(cmd_xst + cmd_pipe_to_file)

  if os.WEXITSTATUS(retval) != 0:
    error_print(ERROR_SYNTHESIS, "SYNTHESIS ERROR(%d)" % os.WEXITSTATUS(retval) );
  elif os.WIFSIGNALED(retval) :
    # USER SIGINT SIGNALED
    if os.WTERMSIG(retval) == 2 :  
      error_print(ERROR_SIGINT_KILL, "TERMINATED BY USER (SIGNAL = %d) = %s" % (os.WTERMSIG(retval), sig_dict[os.WTERMSIG(retval)] ) )
    # possible segfault
    else : 
      error_print(ERROR_SIGINT_KILL, "TERMINATED BY SIGNAL(%d) = %s" % (os.WTERMSIG(retval), sig_dict[os.WTERMSIG(retval)] ) )

  # Exit if only synthesis
  if DO_SYNTHESIS_ONLY == True:
    debug_print('STOP AT SYNTHESIS')
    sys.exit(ERROR_NONE)

  ##### NGDBUILD COMPILATION ####
  # output is top_entity_name.ngd
  cmd_ngdbuild = 'ngdbuild ' + SPACE + top_entity_name + '.ngc'

  debug_print('#######')
  debug_print('EXECUTE : ' + cmd_ngdbuild)
  debug_print('#######')
 
  retval = os.system(cmd_ngdbuild)

  if os.WEXITSTATUS(retval) != 0:
    error_print(ERROR_SYNTHESIS, "SYNTHESIS ERROR(%d)" % os.WEXITSTATUS(retval) );
  elif os.WIFSIGNALED(retval) :
    # USER SIGINT SIGNALED
    if os.WTERMSIG(retval) == 2 :  
      error_print(ERROR_SIGINT_KILL, "TERMINATED BY USER (SIGNAL = %d) = %s" % (os.WTERMSIG(retval), sig_dict[os.WTERMSIG(retval)] ) )
    # possible segfault
    else : 
      error_print(ERROR_SIGINT_KILL, "TERMINATED BY SIGNAL(%d) = %s" % (os.WTERMSIG(retval), sig_dict[os.WTERMSIG(retval)] ) )

  ##### MAP COMPILATION ####
  # output is top_entity_name.ncd, pcf ( pcf looks like a log file)
  cmd_map = 'map -p xc4vlx200-ff1513-10 -cm area -pr b -k 4 -c 100 ' + \
            '-o' + SPACE + top_entity_name + '.ncd' + SPACE + \
            top_entity_name + '.ngd' + SPACE + \
            top_entity_name + '.pcf'

  debug_print('#######')
  debug_print('EXECUTE : ' + cmd_map)
  debug_print('#######')
   
  retval = os.system(cmd_map)

  if os.WEXITSTATUS(retval) != 0:
    error_print(ERROR_SYNTHESIS, "SYNTHESIS ERROR(%d)" % os.WEXITSTATUS(retval) );
  elif os.WIFSIGNALED(retval) :
    # USER SIGINT SIGNALED
    if os.WTERMSIG(retval) == 2 :  
      error_print(ERROR_SIGINT_KILL, "TERMINATED BY USER (SIGNAL = %d) = %s" % (os.WTERMSIG(retval), sig_dict[os.WTERMSIG(retval)] ) )
    # possible segfault
    else : 
      error_print(ERROR_SIGINT_KILL, "TERMINATED BY SIGNAL(%d) = %s" % (os.WTERMSIG(retval), sig_dict[os.WTERMSIG(retval)] ) )


  ##### PAR COMPILATION ####
  # -w means overwrite, should change
  cmd_par = 'par -w -ol std -t 1' + SPACE + \
            top_entity_name + '.ncd' + SPACE + \
            top_entity_name + '.ncd' + SPACE + \
            top_entity_name + '.pcf'

  debug_print('#######')
  debug_print('EXECUTE : ' + cmd_par)
  debug_print('#######')
  
  retval = os.system(cmd_par)

  if os.WEXITSTATUS(retval) != 0:
    error_print(ERROR_SYNTHESIS, "SYNTHESIS ERROR(%d)" % os.WEXITSTATUS(retval) );
  elif os.WIFSIGNALED(retval) :
    # USER SIGINT SIGNALED
    if os.WTERMSIG(retval) == 2 :  
      error_print(ERROR_SIGINT_KILL, "TERMINATED BY USER (SIGNAL = %d) = %s" % (os.WTERMSIG(retval), sig_dict[os.WTERMSIG(retval)] ) )
    # possible segfault
    else : 
      error_print(ERROR_SIGINT_KILL, "TERMINATED BY SIGNAL(%d) = %s" % (os.WTERMSIG(retval), sig_dict[os.WTERMSIG(retval)] ) )


  ##### TRCE COMPILATION ####
  # -xml option needs top name, adds extension .twx automatically
  cmd_trce = 'trce -e 3 -s 10' + SPACE + \
            '-xml' + SPACE + top_entity_name + SPACE + \
            top_entity_name + '.ncd' + SPACE + \
            '-o' + SPACE + top_entity_name + '.twr' + SPACE + \
            top_entity_name + '.pcf'

  debug_print('#######')
  debug_print('EXECUTE : ' + cmd_trce)
  debug_print('#######')
  retval = os.system(cmd_trce)

  if os.WEXITSTATUS(retval) != 0:
    error_print(ERROR_SYNTHESIS, "SYNTHESIS ERROR(%d)" % os.WEXITSTATUS(retval) );
  elif os.WIFSIGNALED(retval) :
    # USER SIGINT SIGNALED
    if os.WTERMSIG(retval) == 2 :  
      error_print(ERROR_SIGINT_KILL, "TERMINATED BY USER (SIGNAL = %d) = %s" % (os.WTERMSIG(retval), sig_dict[os.WTERMSIG(retval)] ) )
    # possible segfault
    else : 
      error_print(ERROR_SIGINT_KILL, "TERMINATED BY SIGNAL(%d) = %s" % (os.WTERMSIG(retval), sig_dict[os.WTERMSIG(retval)] ) )





  os.chdir(last_dir)

  FINAL_RETURN_VALUE = ERROR_NONE

  ######################################
  # End LOWCIRRF_TO_VHDL COMPILE STAGE
  ######################################

  debug_print('########################################################') 
  debug_print('<<<<<< COMPILE LOCAL (HICIRRF TO VHDL) JOB END <<<<<<')
  debug_print('########################################################') 
