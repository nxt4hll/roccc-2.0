#!/usr/bin/env python

# This script compiles the chosen file(s) or directory 

import os
import re
import sys
import getopt

### Define constants ##########################################################

# Executables
GCC2SUIF = 'gcc2suif'
SUIFDRIVER = 'suifdriver'

# Environment variables
CONST_ENV_VAR_ROCCCHOME='ROCCC_HOME'
CONST_ENV_VAR_NCIHOME='NCIHOME'
CONST_ENV_VAR_MACHSUIFHOME='MACHSUIFHOME'

# String suffixes
CONST_LOOP_DPT_FILE='hi_cirrf.c'
CONST_C_SUFFIX='.c'

# Error return values, eventually used for debugging
# to map to string errors
RETURN_DONE=0
ERROR_NONE=0
ERROR_ENV_VAR_UNDEFINED=1
ERROR_PARSE_ARG=2
ERROR_INVALID_ARG_MIX=3
ERROR_NON_UNIQUE_FILE=5
ERROR_BAD_PASS_FILE=6
ERROR_GCC2SUIF=7
ERROR_SUIFDRIVER_EXECUTE=8
ERROR_NO_COMPILE_DONE=9

ERROR_SIGINT_KILL=99

# Stage levels
STAGE_UNKNOWN=0
STAGE_C_TO_HICIRRF=1

# Random string constants
NEWLINE = '\n'
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

### PASS CONSTANTS ############################################################

CONST_SUIFDRIVER_PASSES_LIST= ["""\
require basicnodes suifnodes cfenodes transforms control_flow_analysis ;
require jasonOutputPass libraryOutputPass global_transforms utility_transforms array_transforms ;
require bit_vector_dataflow_analysis gcc_preprocessing_transforms verifyRoccc ;
require preprocessing_transforms data_dependence_analysis ;
require fifoIdentification ;
load $1.suif ;

CleanRepositoryPass $3 ; AddModulePass $2 $3 ; DumpHeaderPass $3 ;
""" ]

### Define functions ##########################################################

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

def parse_args(local_argv=None):
  """Parse Arguments to find File Options and working directory"""

  if local_argv is None:
    local_argv = sys.argv

  # There are three arguments, the name of the c file, the module name,
  #  and the local directory where the repository is located.

  c_filename = local_argv[1] 
  moduleName = local_argv[2] 
  localDirectory = local_argv[3]

  try:
    c_filename
  except:
    print("No c file specified")

  try:
    moduleName
  except:
    print("No module specified")

  try:
    localDirectory
  except:
    print("No local directory specified") 

  return (c_filename, moduleName, localDirectory)

############################
# Process SUIFDRIVER string Passes and fill in with appropriate values
############################
def process_suifdriver_passes(c_filename_prefix, moduleName, localDirectory):
  """Read instructions listed in pass file and put them in a dictionary where they can be used later"""

  # replace all instances of $1 with the filename prefix
  pat_substitute_prefix = re.compile('[$]1')

  # replace all instances of $2 with the module name
  pat_substitute_moduleName = re.compile('[$]2') 

  # replace all instances of $3 with the local directory
  pat_substitute_localDirectory = re.compile('[$]3')
  
  #split_passes = CONST_SUIFDRIVER_PASSES_LIST.splitlines()

  total_new_passes = [ ]

  for suifdriver_list_item in CONST_SUIFDRIVER_PASSES_LIST :
   
    split_passes = suifdriver_list_item.splitlines()
    new_passes = [ ] 
    for the_split_input_line in split_passes:

      # any reference to $1 is change to <prefix>
      the_line = pat_substitute_prefix.sub(c_filename_prefix, the_split_input_line)
      # any reference to $2 is changed to <moduleName>
      the_line = pat_substitute_moduleName.sub(moduleName, the_line)
      
      # any reference to $3 is changed to <localDirectory>
      the_line = pat_substitute_localDirectory.sub(localDirectory, the_line)

      new_passes.append(the_line + NEWLINE)

    # concatonate the last set of passes
    total_new_passes.append( ''.join(new_passes))

  return total_new_passes


###########################################################
### Begin program #########################################
###########################################################
if __name__ == "__main__":

  FINAL_RETURN_VALUE = ERROR_NO_COMPILE_DONE  

  # ensure environment variables are set because we can call the compiled programs
  # without having to reference the entire path
  #check_env_vars()

  # parse from command line
  c_filename, moduleName, localDirectory = parse_args()

  (c_filename_prefix,ext) = os.path.splitext(c_filename)

  # process with pragma parsing support
  CONST_NEW_SUIFDRIVER_PASSES_LIST = process_suifdriver_passes(c_filename_prefix, moduleName, localDirectory)

  ##########################################
  ### c to hicirrf compilation #############
  ##########################################

  # Execute gcc2suif
  suif_filename = c_filename_prefix + '.suif'
  cmd_convert_c_to_suif = GCC2SUIF + SPACE + c_filename
  retval = os.system(cmd_convert_c_to_suif)

  for pass_list in CONST_NEW_SUIFDRIVER_PASSES_LIST:
    # Execute all optimization stages
    cmd_suifdriver_execute = SUIFDRIVER + ' -e ' + DOUBLEQUOTE + pass_list + DOUBLEQUOTE
    retval = os.system(cmd_suifdriver_execute)
