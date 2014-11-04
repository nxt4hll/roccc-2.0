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
CONST_HICIRRF_SUFFIX='-hicirrf.c'
CONST_LOOP_DPT_SUFFIX='-loop.dpt'
#CONST_LOOP_DPT_FILE='loop.dpt'
CONST_LOOP_DPT_FILE='hi_cirrf.c'
CONST_C_SUFFIX='.c'
CONST_PASS_SUFFIX='.pass'


# Error return values, eventuall used for debugging
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

### PASS CONSTANTS ############################################################

CONST_STR_PART_UNROLL_STMT = "PARTIALLY_UNROLL_STATEMENT"
CONST_STR_FULL_UNROLL_STMT = "FULLY_UNROLL_STATEMENT"
CONST_STR_GEN_TILE_STMT = "GENERATE_TILE_STATEMENT"
CONST_STR_GEN_SYS_ARRAY_STMT = "GENERATE_SYSTOLIC_ARRAY_STATEMENT"
CONST_STR_GEN_SYS_ARRAY_UNROLL_STMT = "GENERATE_SYSTOLIC_ARRAY_UNROLL_STATEMENT"
CONST_STR_GEN_SYS_ARRAY_INTERCHANGE_STMT = "GENERATE_SYSTOLIC_ARRAY_INTERCHANGE_STATEMENT"
CONST_STR_GEN_SYS_ARRAY_FEEDBACK_ELIMINATE_STMT = "GENERATE_SYSTOLIC_ARRAY_FEEDBACK_ELIMINATE_STATEMENT"

CONST_SUIFDRIVER_PASSES_LIST= ["""\
require basicnodes suifnodes cfenodes transforms control_flow_analysis ;
require jasonOutputPass global_transforms utility_transforms array_transforms loop_transforms ;
require bit_vector_dataflow_analysis gcc_preprocessing_transforms verifyRoccc ;
require preprocessing_transforms data_dependence_analysis optimizer_output ;
require fifoIdentification DetermineReusePair ;
load $1.suif ;
PreprocessPass ;
FlattenStatementListsPass ;
ControlFlowSolvePass ;
DataFlowSolvePass2 ;
UD_DU_ChainBuilderPass2 ;

ConstantPropagationAndFoldingPass ;
ConstantQualedArrayPropagationPass ;
ConstantPropagationAndFoldingPass ;
TemporalCSEPass ; 

FlattenStatementListsPass ;
ControlFlowSolvePass ;
DataFlowSolvePass2 ;
UD_DU_ChainBuilderPass2 ;
LoopInfoPass ; 
PreprocessingPass ;
CopyPropagationPass2 ;
ScalarReplacementPass ;
IfConversionPass ;
FifoIdentification ;
DetermineReusePairs ;

ControlFlowSolvePass ;
DataFlowSolvePass2 ;
UD_DU_ChainBuilderPass2 ;
OutputIdentificationPass ;

VerifyPass ;
OutputPass ;
StripAnnotesPass ;
""" ]

### Global Variables ##########################################################

# verbose level
VERBOSE_LEVEL=1


### Define functions ##########################################################

#######################
# ERROR Printing
#######################
def error_print(retval, the_string):
  """Effective method for printing an error message and having it return the correct error."""
    
  print 'DEBUG_COMPILE_C_TO_VHDL: ERROR(%d): '% retval,
  print the_string
  sys.stdout.flush()
  debug_print('########################################################') 
  debug_print('<<<<<< COMPILE LOCAL (C TO VHDL) JOB FAIL <<<<<<')
  debug_print('########################################################') 
 
  sys.exit(retval)


#######################
# DEBUG Printing
#######################
def debug_print(the_string, level=2):
  """Based on the global variable VERBOSE_LEVEL it will print out the string
    if the VERBOSE_LEVEL exceeds or matches the input level. Useful for debug 
    printing."""

  global VERBOSE_LEVEL 

  if VERBOSE_LEVEL >= level:
    print 'DEBUG_COMPILE_C_TO_VHDL: ' + the_string
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
# Argument Processing
#######################
def print_help():
  """Print Help for local code"""
  print "\nCompile local directory's contents"
  print "\nUsage: " + sys.argv[0] + "[OPTIONS] <c_filename>.c <Opt Pass file(optional)>.pass"
  print """\
  [COMPILE OPTIONS]:

    -h,--help         
        Display this usage message
  
    -v, --verbose  
        Turn on verbose mode.  For more verbose information, 
        add more -v switches to increase verbosity (eg: -vv).
        Max verbosity listed as 2 for now.
"""
#    -q, --quiet
#        Turns off default printing of information in on screen.  Good if running
#        a testbench to see if a compilation pass fails
#  """


def parse_args(local_argv=None):
  """Parse Arguments to find File Options and working directory"""

  global VERBOSE_LEVEL 

  if local_argv is None:
    local_argv = sys.argv
    
  # parse command line options
  try:
    opts, args = getopt.gnu_getopt(local_argv[1:], "hv", ["help", "verbose"])
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

  if len(args) > 2 : 
    error_print(ERROR_INVALID_ARG_MIX, "Must specify only one hicirrf with or without a pass file")

  for i in args:
    if re.match('.*[.]c$', i):
      c_filename = i
    if re.match('.*[.]pass$', i):
      pass_filename = i

  try:
    c_filename
  except:
    error_print(ERROR_PARSE_ARG, 'A *.c must be specified in the argument')

  try: 
    pass_filename
  except:
    pass_filename = None


  if not os.path.isfile(c_filename):
    error_print(ERROR_PARSE_ARG, 'File \"' + c_filename + '\" does not exist')

  if pass_filename != None and not os.path.isfile(pass_filename):
    error_print(ERROR_PARSE_ARG, 'File \"' + pass_filename + '\" does not exist')



  return (c_filename, pass_filename)

#######################
# Remove underscores function
#######################
def remove_underscores(in_filename, the_prefix):
  """Remove underscores from the input file in the form of '_prefix' -> 'prefix'"""
  # also fix some problems with init_inputscalar by commenting that line out

  if not os.path.isfile(in_filename):
    error_print(ERROR_PARSE_ARG, 'REMOVE_UNDERSCORES_FUNCTION: File \"' + in_filename + '\" does not exist')


  out_filename = the_prefix + CONST_HICIRRF_SUFFIX
  sliced_prefix = the_prefix[0:10]

  pat_underscore = re.compile('_' + sliced_prefix)  # understand '_$prefix'
  CONST_STR_COMMENT = '//'

  # init_c_scalar support as ROCCC_init_inputscalar
  str_init_c_scalar = 'ROCCC_init_C_scalar'
  str_output_c_scalar = 'ROCCC_output_C_scalar'

  pat_init_c_scalar = re.compile(str_init_c_scalar)
  pat_output_c_scalar = re.compile(str_output_c_scalar)

  debug_print('Removing underscores : infile(%s) -> match(%s) -> outfile(%s)' % (in_filename, sliced_prefix, out_filename))

  fout = open(out_filename, "w")
  fin = open(in_filename)
  for line_in in fin:
    line_out1 = pat_underscore.sub(sliced_prefix, line_in)
    #line_out2 = pat_init_c_scalar.sub(CONST_STR_COMMENT + str_init_c_scalar, line_out1)
    line_out3 = pat_output_c_scalar.sub(CONST_STR_COMMENT + str_output_c_scalar, line_out1)
    line_out = line_out3

    fout.write( line_out )
  fin.close()
  fout.close()
  return out_filename

############################
# Process pass_file function
############################
def process_pass_file(pass_filename):
  """Read instructions listed in pass file and put them in a dictionary where they can be used later"""
  global CONST_STR_PART_UNROLL_STMT
  global CONST_STR_FULL_UNROLL_STMT
  global CONST_STR_GEN_TILE_STMT
  global CONST_STR_GEN_SYS_ARRAY_STMT

  re_pattern_full_unroll    = re.compile('^[ \t]*fully[ \t]+unroll[ \t]+(?P<label>\w+)[ \t]*$')
  re_pattern_part_unroll    = re.compile('^[ \t]*partially[ \t]+unroll[ \t]+(?P<label>\w+)[ \t]+(?P<unroll_value>\d+)[ \t]*$')
  re_pattern_gen_tile       = re.compile('^[ \t]*generate[ \t]+tile[ \t]+(?P<label1>\w+)[ \t]+(?P<label2>\w+)[ \t]+(?P<tilelen1>\d+)[ \t]+(?P<tilelen2>\d+)[ \t]*$')
  re_pattern_gen_sys_array  = re.compile('^[ \t]*generate[ \t]+systolic[ \t]+array[ \t]+(?P<label1>\w+)[ \t]+(?P<label2>\w+)[ \t]+(?P<sysarraysize>\w+)[ \t]*$')


  PRINT_LEVEL = 3

  # big dictionary
  PASS_MAP = { }
  # each one is a list of maps
  PASS_MAP[CONST_STR_FULL_UNROLL_STMT] = [ ]
  PASS_MAP[CONST_STR_PART_UNROLL_STMT] = [ ]
  PASS_MAP[CONST_STR_GEN_TILE_STMT] = [ ]
  PASS_MAP[CONST_STR_GEN_SYS_ARRAY_STMT] = [ ]

  if pass_filename == None:
    debug_print('No pass file defined, default no-optimizations')
    # return empty pass map
    return PASS_MAP

  debug_print('PARSE PASS FILE: ' + pass_filename)
  fin = open(pass_filename)
  for line_in in fin:
    re_match_fu = re_pattern_full_unroll.match(line_in)
    re_match_pu = re_pattern_part_unroll.match(line_in)
    re_match_gen_tile = re_pattern_gen_tile.match(line_in)
    re_match_gen_sys_array = re_pattern_gen_sys_array.match(line_in)

    debug_print('Line = ' + line_in.strip(), PRINT_LEVEL )

    local_dict = { }

    ## MATCH AND PARSE: fully unroll <LABEL>
    if re_match_fu:
      debug_print('full unroll LABEL:' + re_match_fu.group('label'), PRINT_LEVEL)
      local_dict['type'] = CONST_STR_FULL_UNROLL_STMT
      local_dict['label'] = re_match_fu.group('label')

      PASS_MAP[CONST_STR_FULL_UNROLL_STMT].append(local_dict)

    ## MATCH AND PARSE: partially unroll <LABEL> <UNROLL_VALUE>
    elif re_match_pu:
      debug_print('partially unroll LABEL:' + re_match_pu.group('label') + ' unroll_factor:' + re_match_pu.group('unroll_value'), PRINT_LEVEL)
      local_dict['type'] = CONST_STR_PART_UNROLL_STMT
      local_dict['label'] = re_match_pu.group('label')
      local_dict['unroll_value'] = re_match_pu.group('unroll_value')

      PASS_MAP[CONST_STR_PART_UNROLL_STMT].append(local_dict)

    ## MATCH AND PARSE: generate tile <LABEL1> <LABEL2> <TILELEN1> <TILELEN2>
    elif re_match_gen_tile:
      debug_print('generate tile LABEL1:' + re_match_gen_tile.group('label1') + ' LABEL2:' + re_match_gen_tile.group('label2') + \
        ' TILELEN1:' + re_match_gen_tile.group('tilelen1') + ' TILELEN2:' + re_match_gen_tile.group('tilelen2'), PRINT_LEVEL)
      local_dict['type'] = CONST_STR_GEN_TILE_STMT
      local_dict['label1'] = re_match_gen_tile.group('label1')
      local_dict['label2'] = re_match_gen_tile.group('label2')
      local_dict['tilelen1'] = re_match_gen_tile.group('tilelen1')
      local_dict['tilelen2'] = re_match_gen_tile.group('tilelen2')

      PASS_MAP[CONST_STR_GEN_TILE_STMT].append(local_dict)

    ## MATCH AND PARSE: generate systolic array <LABEL1> <LABEL2> <SYSARRAYSIZE>
    elif re_match_gen_sys_array:
      #print re_match_gen_sys_array.groups()
      debug_print( 'generate systolic array LABEL1:' + re_match_gen_sys_array.group('label1') + ' LABEL2:' + re_match_gen_sys_array.group('label2') + \
        ' ArraySize:' + re_match_gen_sys_array.group('sysarraysize') , PRINT_LEVEL )

      local_dict['type'] = CONST_STR_GEN_SYS_ARRAY_STMT
      local_dict['label1'] = re_match_gen_sys_array.group('label1')
      local_dict['label2'] = re_match_gen_sys_array.group('label2')
      local_dict['sysarraysize'] = re_match_gen_sys_array.group('sysarraysize')

      PASS_MAP[CONST_STR_GEN_SYS_ARRAY_STMT].append(local_dict)

    # Match a commented line
    elif re.match('^#', line_in):
      debug_print('Commented line: ' + line_in.strip(), PRINT_LEVEL)

    # find a word type, but matching above didn't work so print out error
    elif re.match('\S+', line_in):
      error_print(ERROR_BAD_PASS_FILE, 'Unrecognizable sequence in pass file: ' + line_in.strip())

  fin.close()

  debug_print('FINISH PARSE PASS FILE')
  return PASS_MAP

############################
# Process SUIFDRIVER string Passes and fill in with appropriate values
############################
def process_suifdriver_passes(PASS_MAP, c_filename_prefix):
  """Read instructions listed in pass file and put them in a dictionary where they can be used later"""
  global CONST_STR_PART_UNROLL_STMT
  global CONST_STR_FULL_UNROLL_STMT
  global CONST_STR_GEN_TILE_STMT
  global CONST_STR_GEN_SYS_ARRAY_STMT
  global CONST_STR_GEN_SYS_ARRAY_UNROLL_STMT
  global CONST_STR_GEN_SYS_ARRAY_INTERCHANGE_STMT
  global CONST_STR_GEN_SYS_FEEDBACK_ELIMINATE_STMT


  pat_fu = re.compile('### INSERT ' + CONST_STR_FULL_UNROLL_STMT)
  pat_pu = re.compile('### INSERT ' + CONST_STR_PART_UNROLL_STMT)
  pat_gt = re.compile('### INSERT ' + CONST_STR_GEN_TILE_STMT)
  pat_gsau = re.compile('### INSERT ' + CONST_STR_GEN_SYS_ARRAY_UNROLL_STMT)
  pat_gsai = re.compile('### INSERT ' + CONST_STR_GEN_SYS_ARRAY_INTERCHANGE_STMT)
  pat_gsafe = re.compile('### INSERT ' + CONST_STR_GEN_SYS_ARRAY_FEEDBACK_ELIMINATE_STMT)

  # replace all instances of $1 with the filename prefix
  pat_substitute_prefix = re.compile('[$]1')
  
  #split_passes = CONST_SUIFDRIVER_PASSES_LIST.splitlines()

  total_new_passes = [ ]

  for suifdriver_list_item in CONST_SUIFDRIVER_PASSES_LIST :
   
    split_passes = suifdriver_list_item.splitlines()
    new_passes = [ ] 
    for the_split_input_line in split_passes:

      # any reference to $1 is change to <prefix>
      the_line = pat_substitute_prefix.sub(c_filename_prefix, the_split_input_line)


      # FOLLOWING ARE PRAGMA INSERTIONS (stuff found in pass file, insert where specified)


      #################
      ### FULLY UNROLL : 
      ### ACTION: fully unroll <LABEL> => UnrollConstantBoundsPass <LABEL>
      #################
      if pat_fu.search(the_line):
        if len( PASS_MAP[CONST_STR_FULL_UNROLL_STMT] ) > 0 : 
          new_passes.append(the_line + NEWLINE)
          for p in PASS_MAP[CONST_STR_FULL_UNROLL_STMT]:
            temp_str = 'UnrollConstantBoundsPass ' + p['label'] + ' 0 ' + SEMICOLON + NEWLINE
            new_passes.append(temp_str)
          new_passes.append('FlattenStatementListsPass' + SPACE + SEMICOLON + NEWLINE)
          new_passes.append('FusePass' + SPACE + SEMICOLON + NEWLINE)

      #################
      ### PARTIALLY UNROLL 
      ### ACTION: partially unroll <LABEL> <VALUE> => UnrollConstantBoundsPass <LABEL> <VALUE>
      #################
      elif pat_pu.search(the_line):
        if len( PASS_MAP[CONST_STR_PART_UNROLL_STMT] ) > 0 : 
          new_passes.append(the_line + NEWLINE)
          for p in PASS_MAP[CONST_STR_PART_UNROLL_STMT]:
            temp_str = 'UnrollPass ' + p['label'] + SPACE + p['unroll_value'] + SPACE + SEMICOLON + NEWLINE
            new_passes.append(temp_str)
          new_passes.append('FlattenStatementListsPass' + SPACE + SEMICOLON + NEWLINE)
          new_passes.append('FusePass' + SPACE + SEMICOLON + NEWLINE)

      #################
      ### GENERATE TILE
      ### ACTION: generate tile <LABEL1> <LABEL2> <VALUE1> <VALUE2>  => 
      ###             TilePass <LABEL1> <LABEL2> <VALUE1> <VALUE2>
      ###             UnrollConstantBoundsPass <LABEL1>
      ###             UnrollConstantBoundsPass <LABEL2>
      #################
      elif pat_gt.search(the_line):
        if len( PASS_MAP[CONST_STR_GEN_TILE_STMT] ) > 0 : 
          new_passes.append(the_line + NEWLINE)
          for p in PASS_MAP[CONST_STR_GEN_TILE_STMT]:
            temp_str1 = 'TilePass ' + p['label1'] + SPACE + p['label2'] + SPACE + p['tilelen1'] + SPACE + p['tilelen2'] + SPACE + SEMICOLON + NEWLINE
            temp_str2 = 'UnrollConstantBoundsPass ' + p['tilelen1'] + SPACE + SEMICOLON + NEWLINE
            temp_str3 = 'UnrollConstantBoundsPass ' + p['tilelen2'] + SPACE + SEMICOLON + NEWLINE
            new_passes.append(temp_str1)
            new_passes.append(temp_str2)
            new_passes.append(temp_str3)
          new_passes.append('FusePass' + SPACE + SEMICOLON + NEWLINE)

      #################
      ### GENERATE SYSTOLIC ARRAY (unroll pass)
      ### ACTION1: generate systolic array <LABEL1> <LABEL2> <SYSARRAYSIZE>
      ###             UnrollPass <LABEL2> <SYSARRAYSIZE>
      #################
      elif pat_gsau.search(the_line):
        if len( PASS_MAP[CONST_STR_GEN_SYS_ARRAY_STMT] ) > 0 : 
          new_passes.append(the_line + NEWLINE)
          for p in PASS_MAP[CONST_STR_GEN_SYS_ARRAY_STMT]:
            temp_str = 'UnrollPass ' + p['label2'] + SPACE + p['sysarraysize'] + SPACE + SEMICOLON + NEWLINE
            new_passes.append(temp_str)

      #################
      ### GENERATE SYSTOLIC ARRAY (interchange pass)
      ### ACTION2: generate systolic array <LABEL1> <LABEL2> <SYSARRAYSIZE>
      ###             InterchangePass <LABEL2> <LABEL1> 
      #################
      elif pat_gsai.search(the_line):
        if len( PASS_MAP[CONST_STR_GEN_SYS_ARRAY_STMT] ) > 0 : 
          new_passes.append(the_line + NEWLINE)
          for p in PASS_MAP[CONST_STR_GEN_SYS_ARRAY_STMT]:
            temp_str = 'InterchangePass ' + p['label2'] + SPACE + p['label1'] + SPACE + SEMICOLON + NEWLINE
            new_passes.append(temp_str)
          new_passes.append('FusePass' + SPACE + SEMICOLON + NEWLINE)

      #################
      ### GENERATE SYSTOLIC ARRAY (array feedback elimination pass)
      ### ACTION3: generate systolic array <LABEL1> <LABEL2> <SYSARRAYSIZE>
      ###             FeedbackEliminationPass 1 ;
      #################
      elif pat_gsafe.search(the_line):
        if len( PASS_MAP[CONST_STR_GEN_SYS_ARRAY_STMT] ) > 0 : 
          new_passes.append(the_line + NEWLINE)
          for p in PASS_MAP[CONST_STR_GEN_SYS_ARRAY_STMT]:
            temp_str = 'SystolicArrayGenerationPass 0 ;' + NEWLINE
            new_passes.append(temp_str)

      ##############################
      # ELSE just copy the original pass
      ##############################
      else:
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
  c_filename, pass_filename = parse_args()

  debug_print('########################################################') 
  debug_print('>>>>>> COMPILE LOCAL (C TO VHDL) JOB START >>>>>>')
  debug_print('########################################################') 
  debug_print('VERBOSE level = %d' % VERBOSE_LEVEL)
  debug_print("PROCESSING DIR = '%s'" % os.getcwd() )


  (c_filename_prefix,ext) = os.path.splitext(c_filename)

  debug_print('c_filename = %s' % c_filename)
  debug_print('c_filename_prefix = %s' % c_filename_prefix)
  debug_print('pass_filename = %s' % pass_filename)

  # Process passes from inside file
  PASS_MAP = process_pass_file(pass_filename)

  # process with pragma parsing support
  CONST_NEW_SUIFDRIVER_PASSES_LIST = process_suifdriver_passes(PASS_MAP, c_filename_prefix)

  ##########################################
  ### c to hicirrf compilation #############
  ##########################################

  debug_print('#################################')
  debug_print('Performing C to VHDL stage...')
  debug_print('#################################')


  # Execute gcc2suif
  suif_filename = c_filename_prefix + '.suif'
  cmd_convert_c_to_suif = GCC2SUIF + SPACE + c_filename
  debug_print('EXECUTE: ' + cmd_convert_c_to_suif)
  retval = os.system(cmd_convert_c_to_suif)

  if os.WEXITSTATUS(retval) != 0 :
    error_print(ERROR_GCC2SUIF, 'GCC2SUIF COMPILATION ERROR(retval=%d)'% os.WEXITSTATUS(retval) )
  elif os.WIFSIGNALED(retval) :
    # USER SIGINT SIGNALED
    if os.WTERMSIG(retval) == 2 :  
      error_print(ERROR_SIGINT_KILL, "TERMINATED BY USER (SIGNAL = %d) = %s" % (os.WTERMSIG(retval), sig_dict[os.WTERMSIG(retval)] ) )
    # possible segfault
    else : 
      error_print(ERROR_GCC2SUIF, "TERMINATED BY SIGNAL(%d) = %s" % (os.WTERMSIG(retval), sig_dict[os.WTERMSIG(retval)] ) )


  for pass_list in CONST_NEW_SUIFDRIVER_PASSES_LIST:
    # Execute all optimization stages
    cmd_suifdriver_execute = SUIFDRIVER + ' -e ' + DOUBLEQUOTE + pass_list + DOUBLEQUOTE
    debug_print('EXECUTE: suifdriver')
    debug_print(cmd_suifdriver_execute, 3)
    retval = os.system(cmd_suifdriver_execute)

    if os.WEXITSTATUS(retval) != 0 :
      error_print(ERROR_SUIFDRIVER_EXECUTE, 'SUIFDRIVER EXECUTION ERROR (retval=%d)'% os.WEXITSTATUS(retval) )
    elif os.WIFSIGNALED(retval) :
      # USER SIGINT SIGNALED
      if os.WTERMSIG(retval) == 2 :  
        error_print(ERROR_SIGINT_KILL, "TERMINATED BY USER (SIGNAL = %d) = %s" % (os.WTERMSIG(retval), sig_dict[os.WTERMSIG(retval)] ) )
      # possible segfault
      else : 
        error_print(ERROR_SUIFDRIVER_EXECUTE, "TERMINATED BY SIGNAL(%d) = %s" % (os.WTERMSIG(retval), sig_dict[os.WTERMSIG(retval)] ) )

  # We no longer need to remove underscores or create another file,
  #  so we can comment this out.
  # out_filename = remove_underscores(CONST_LOOP_DPT_FILE, c_filename_prefix)
  # debug_print('OUTPUT FILENAME = ' + out_filename)

  
  ##############################################
  ### end c to hicirrf compilation #############
  ##############################################

  debug_print('########################################################') 
  debug_print('<<<<<< COMPILE LOCAL (C TO VHDL) JOB END <<<<<<')
  debug_print('########################################################') 
