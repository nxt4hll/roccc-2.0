#!/usr/bin/env python

# This script compiles the chosen file(s) or directory 

import os
import re
import sys
import getopt

# this roccc_python_lib.py file was copied to the bin lib and can be used as a library file
import roccc_python_lib as ROCCC_PY_LIB 

### Define constants ##########################################################


# Error return values, eventuall used for debugging
# to map to string errors
RETURN_DONE=0
ERROR_NONE=0
ERROR_ENV_VAR_UNDEFINED=1
ERROR_PARSE_ARG=2
ERROR_SYNTHESIS=3
ERROR_FILE_DOES_NOT_EXIST=4
ERROR_BAD=5

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

STR_DEFAULT_SYNTHESIS_OPTIONS_START ='^DEFAULT_SYNTHESIS_OPTIONS_START'
STR_DEFAULT_SYNTHESIS_OPTIONS_END   ='^DEFAULT_SYNTHESIS_OPTIONS_END'

STR_RASC_XST_PROJECT_OPTIONS_START ='^RASC_XST_PROJECT_OPTIONS_START'
STR_RASC_XST_PROJECT_OPTIONS_END   ='^RASC_XST_PROJECT_OPTIONS_END'

STR_RASC_XST_SYNTHESIS_SCRIPT_OPTIONS_START ='^RASC_XST_SYNTHESIS_SCRIPT_OPTIONS_START'
STR_RASC_XST_SYNTHESIS_SCRIPT_OPTIONS_END   ='^RASC_XST_SYNTHESIS_SCRIPT_OPTIONS_END'

STR_RASC_SYNPLIFY_PRO_PROJECT_OPTIONS_START ='^RASC_SYNPLIFY_PRO_PROJECT_OPTIONS_START'
STR_RASC_SYNPLIFY_PRO_PROJECT_OPTIONS_END   ='^RASC_SYNPLIFY_PRO_PROJECT_OPTIONS_END'


STR_TOP_ENTITY              = '^TOP_ENTITY'
STR_FILE_VHDL_SYNTHESIZE    = '^FILE_VHDL_SYNTHESIZE'
STR_FILE_VHDL_EXTRA         = '^FILE_VHDL_EXTRA'
STR_FILE_SW                 = '^FILE_SW'
STR_COMMENT                 = '^#'

######


### Global Variables ##########################################################
TOP_ENTITY          = 'TOP_ENTITY'
SYNTHESIS_OPTIONS   = 'SYNTHESIS_OPTIONS'
PROJECT_OPTIONS     = 'PROJECT_OPTIONS'
VHDL_FILES_DEFAULT  = 'VHDL_FILES_DEFAULT'
VHDL_FILES_EXTRA    = 'VHDL_FILES_EXTRA'
HOST_C_SW           = 'HOST_C_SW'

# DEFAULT DIRECTORIES
SYNTHESIS_DIR       = './SYNTHESIS_DIR'
SOURCE_DIR          = './src'
RASC_EXPORT_DIR     = '../RASC_EXPORT'

# synthesis option
DO_DEFAULT_SYNTHESIS=False
DO_DEFAULT_SYNTHESIS_XST_ONLY=False

# RASC Mode
DO_RASC=False
DO_RASC_XST=False
DO_RASC_SYNPLIFY_PRO=False
DO_STOP_BEFORE_MAKE=False


### Define functions ##########################################################

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

    --default-synthesis
        Compile files (with XILINX tools), letting the ROCCC generated code 
        to be considered the top level.

    --default-synthesis-xst-only
        Same as "--default-synthesis", but do only the 'xst' portion of 
        synthesis, do not do MAP, PAR, nor TRCE.  This is strickly 'xst'.
        Good if you only want to check syntax and the like, but also make
        sure the structure of the vhdl is sound (but don't want timing).

    --RASC-xst
        Compile files for RASC architecture as top level (with XILINX tools)

    --RASC-synplify-pro
        Compile files for RASC architecture as top level (with Symplicity tools
        for synthesis, and then XILINX for rest of code)

    --stop-before-make
        If used in conjunction with --RASX-xst or --RASC-synplify-pro, it will
        allow the production of the directory ready for a command to "make all", 
        but will stop short of that allowing user to see the setup of the directory
        contents at "./SYNTHESIS_DIR" and possibly modify.

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

  global DO_DEFAULT_SYNTHESIS
  global DO_DEFAULT_SYNTHESIS_XST_ONLY
  global DO_RASC
  global DO_RASC_XST
  global DO_RASC_SYNPLIFY_PRO
  global DO_STOP_BEFORE_MAKE

  # import default setting
  VERBOSE_LEVEL = ROCCC_PY_LIB.VERBOSE_LEVEL

  if local_argv is None:
    local_argv = sys.argv
    
  # parse command line options
  try:
    opts, args = getopt.gnu_getopt(local_argv[1:], "hvq", ["help", "verbose", "quiet", "default-synthesis", "default-synthesis-xst-only", "RASC-xst", "RASC-synplify-pro", "stop-before-make"])
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

    elif o in ("--default-synthesis", "--default-synthesis-xst-only") : 
      DO_DEFAULT_SYNTHESIS = True
      if o == "--default-synthesis-xst-only" : 
        DO_DEFAULT_SYNTHESIS_XST_ONLY = True


    elif o in ("--RASC-xst", "--RASC-synplify-pro") : 
      DO_RASC = True
      if o == "--RASC-xst" : 
        DO_RASC_XST = True
      elif o == "--RASC-synplify-pro" : 
        DO_RASC_SYNPLIFY_PRO = True

    elif o == "--stop-before-make" : 
      DO_STOP_BEFORE_MAKE = True
        

  # save back to settings file
  ROCCC_PY_LIB.VERBOSE_LEVEL = VERBOSE_LEVEL


  if len(args) == 0 : 
    full_filename = 'DIRECTIVES.dat'
     
  elif len(args) != 1 : 
    ROCCC_PY_LIB.error_print(ERROR_PARSE_ARG, "Must specify only 1 input directives file")

  else : 
    full_filename = args[0]

  ######
  # Error checking to make sure the options are valid
  ######


  if not os.path.isfile(full_filename):
    ROCCC_PY_LIB.error_print(ERROR_PARSE_ARG, 'File \"' + full_filename + '\" does not exist')


  return full_filename


#######################
# Read directives script
#######################
def read_directives(in_filename):
  """Read synthesis directives from file"""

  global DO_RASC

  directives_map = { }
  directives_map[VHDL_FILES_DEFAULT] = [ ]
  directives_map[VHDL_FILES_EXTRA]   = [ ]
  directives_map[SYNTHESIS_OPTIONS]  = [ ]
  directives_map[PROJECT_OPTIONS]  = [ ]


  pat_DEFAULT_synth_option_start  = re.compile(STR_DEFAULT_SYNTHESIS_OPTIONS_START)
  pat_DEFAULT_synth_option_end    = re.compile(STR_DEFAULT_SYNTHESIS_OPTIONS_END)

  pat_RASC_xst_synth_script_option_start  = re.compile(STR_RASC_XST_SYNTHESIS_SCRIPT_OPTIONS_START)
  pat_RASC_xst_synth_script_option_end    = re.compile(STR_RASC_XST_SYNTHESIS_SCRIPT_OPTIONS_END)

  pat_RASC_xst_prj_option_start  = re.compile(STR_RASC_XST_PROJECT_OPTIONS_START)
  pat_RASC_xst_prj_option_end    = re.compile(STR_RASC_XST_PROJECT_OPTIONS_END)

  pat_RASC_synplify_pro_prj_option_start  = re.compile(STR_RASC_SYNPLIFY_PRO_PROJECT_OPTIONS_START)
  pat_RASC_synplify_pro_prj_option_end    = re.compile(STR_RASC_SYNPLIFY_PRO_PROJECT_OPTIONS_END)


  pat_top_entity          = re.compile(STR_TOP_ENTITY)
  pat_vhdl_synth_file     = re.compile(STR_FILE_VHDL_SYNTHESIZE)
  pat_vhdl_extra_file     = re.compile(STR_FILE_VHDL_EXTRA)
  pat_file_sw             = re.compile(STR_FILE_SW)
  pat_comment             = re.compile(STR_COMMENT)

  # possible states
  STATE_OTHER = 1
  STATE_GET_XST_OPTIONS = 2
  STATE_GET_PRJ_OPTIONS = 3


  # Read in 1 line at a time and process it based on state machine
  cs = STATE_OTHER
  fin = open(in_filename)
  for line_in in fin: 
    line_in_split = line_in.split()

    # if not a comment line
    if not pat_comment.search( line_in ) : 

      if cs == STATE_OTHER : 
        # top entity name
        if pat_top_entity.search(line_in) : 
          directives_map[TOP_ENTITY] =  line_in_split[1]
        
        # host sw
        elif pat_file_sw.search(line_in) : 
          directives_map[HOST_C_SW] = line_in_split[1]

        # append to DEFAULT files list
        elif pat_vhdl_synth_file.search(line_in) : 
          directives_map[VHDL_FILES_DEFAULT].append( line_in_split[1] )

        # append to EXTRA files list
        elif pat_vhdl_extra_file.search(line_in) : 
          directives_map[VHDL_FILES_EXTRA].append( line_in_split[1] )

        # if non-rasc mode, then get default synthesis options
        elif ( not DO_RASC ) and pat_DEFAULT_synth_option_start.search(line_in) : 
          cs = STATE_GET_XST_OPTIONS

        # if rasc mode, then get rasc default synthesis options as specified
        # from rasc sample code
        elif DO_RASC :  
          if DO_RASC_XST and pat_RASC_xst_synth_script_option_start.search(line_in) : 
            cs = STATE_GET_XST_OPTIONS
          elif DO_RASC_XST and pat_RASC_xst_prj_option_start.search(line_in) : 
            cs = STATE_GET_PRJ_OPTIONS
          elif DO_RASC_SYNPLIFY_PRO and pat_RASC_synplify_pro_prj_option_start.search(line_in) : 
            cs = STATE_GET_PRJ_OPTIONS

      # currently reading synthesis options
      elif cs == STATE_GET_XST_OPTIONS: 
        # if we reach the ned then just go to new state
        if pat_DEFAULT_synth_option_end.search(line_in) or pat_RASC_xst_synth_script_option_end.search(line_in): 
          cs = STATE_OTHER

        # otherwise just assume this is the options line
        else : 
          directives_map[SYNTHESIS_OPTIONS].append( line_in )

      # currently reading project options
      elif cs == STATE_GET_PRJ_OPTIONS: 
        # if we reach the ned then just go to new state
        if pat_RASC_xst_prj_option_end.search(line_in) or pat_RASC_synplify_pro_prj_option_end.search(line_in) : 
          cs = STATE_OTHER

        # otherwise just assume this is the options line
        else : 
          directives_map[PROJECT_OPTIONS].append( line_in )


  fin.close()

  return directives_map


###########################################################
# Compile synthesis manually, old compile_vhdl2synthesis
###########################################################
def compile_synthesis_manually(directives_map):
  """Compile manually the local VHDL files"""

  # ensure environment variables are set because we can call the compiled programs
  # without having to reference the entire path
  ROCCC_PY_LIB.check_env_vars( [ROCCC_PY_LIB.CONST_ENV_VAR_XILINX] )


  last_dir = os.getcwd();

  if not os.path.isdir(SYNTHESIS_DIR):
    os.mkdir(SYNTHESIS_DIR)
  os.chdir(SYNTHESIS_DIR)
 
  top_entity_name = directives_map[TOP_ENTITY]

  # write the project files
  fout_prj = open( top_entity_name + '.prj', 'w' )
  for val in directives_map[VHDL_FILES_DEFAULT]: 
    fout_prj.write( 'vhdl work "../' + val + '"\n' )
  fout_prj.close();

  fout_scr = open( top_entity_name + '.scr', 'w' )
  for val in directives_map[SYNTHESIS_OPTIONS]: 
    fout_scr.write( val )
  fout_scr.close();

  ##############################
  # run synthesis in this directory
  ##############################

  ##### XST COMPILATION ####
  # output is top_entity_name.ngc
  cmd_xst = 'xst -ifn' + SPACE + top_entity_name + '.scr' + SPACE + \
                   '-ofn' + SPACE + top_entity_name + '.syr'
  
  ROCCC_PY_LIB.execute_shell_cmd( cmd_xst )

  # Exit if only synthesis
  if DO_DEFAULT_SYNTHESIS_XST_ONLY == True:
    ROCCC_PY_LIB.debug_print('STOP AT XST SYNTHESIS')
    sys.exit(ERROR_NONE)

  ##### NGDBUILD COMPILATION ####
  # output is top_entity_name.ngd
  cmd_ngdbuild = 'ngdbuild ' + SPACE + top_entity_name + '.ngc'

  ROCCC_PY_LIB.execute_shell_cmd( cmd_ngdbuild )
 
  ##### MAP COMPILATION ####
  # output is top_entity_name.ncd, pcf ( pcf looks like a log file)
  cmd_map = 'map -p xc4vlx200-ff1513-10 -cm area -pr b -k 4 -c 100 ' + \
            '-o' + SPACE + top_entity_name + '.ncd' + SPACE + \
            top_entity_name + '.ngd' + SPACE + \
            top_entity_name + '.pcf'

  ROCCC_PY_LIB.execute_shell_cmd( cmd_map )
   

  ##### PAR COMPILATION ####
  # -w means overwrite, should change
  cmd_par = 'par -w -ol std -t 1' + SPACE + \
            top_entity_name + '.ncd' + SPACE + \
            top_entity_name + '.ncd' + SPACE + \
            top_entity_name + '.pcf'

  ROCCC_PY_LIB.execute_shell_cmd( cmd_par )
  

  ##### TRCE COMPILATION ####
  # -xml option needs top name, adds extension .twx automatically
  cmd_trce = 'trce -e 3 -s 10' + SPACE + \
            '-xml' + SPACE + top_entity_name + SPACE + \
            top_entity_name + '.ncd' + SPACE + \
            '-o' + SPACE + top_entity_name + '.twr' + SPACE + \
            top_entity_name + '.pcf'

  ROCCC_PY_LIB.execute_shell_cmd( cmd_trce )

  os.chdir(last_dir)


###########################################################
###########################################################
# Compile synthesis through RASC and hopefully produce
# a bin and two cfg files that can be uploaded to RASC blade
###########################################################
###########################################################
def compile_synthesis_for_RASC(directives_map):
  """Compile the local VHDL files and extra RASC files to produce RASC bit file"""

  # ensure environment variables are set because we can call the compiled programs
  # without having to reference the entire path
  ROCCC_PY_LIB.check_env_vars( [ROCCC_PY_LIB.CONST_ENV_VAR_ROCCC_HOME, 
      ROCCC_PY_LIB.CONST_ENV_VAR_XILINX, 
      ROCCC_PY_LIB.CONST_ENV_VAR_RASC ] )

  last_dir = os.getcwd();

  # make ./synthesis_DIR
  if not os.path.isdir(SYNTHESIS_DIR):
    os.mkdir(SYNTHESIS_DIR)
  os.chdir(SYNTHESIS_DIR)
  ROCCC_PY_LIB.debug_print('CD to ' + SYNTHESIS_DIR)


  # make ./src directory
  if not os.path.isdir(SOURCE_DIR):
    os.mkdir(SOURCE_DIR)
  ROCCC_PY_LIB.debug_print('MKDIR: ' + SOURCE_DIR)
 
  top_entity_name = directives_map[TOP_ENTITY]


  ###################
  # Copy compilation files to local directory for easier compilation. 
  #  Create : 
  #     - ${top_entity_name}.prj (project files listing for XST)
  #     - ${top_entity_name}.scr (script that list synthesis options for XST)
  #     - Makefile, Makefile.local, alg.h, alg_block_top.v to local directory
  ###################

  # write the project files
  fout_prj = open( top_entity_name + '.prj', 'w' )

  ###################
  if DO_RASC_XST : 
    ####
    # the default RASC project options that were imported
    # change all references of $RASC to actual evironment variable
    pat_env_var_RASC=re.compile("\$RASC")
    for val in directives_map[PROJECT_OPTIONS]: 
      val_out = pat_env_var_RASC.sub( os.environ[ROCCC_PY_LIB.CONST_ENV_VAR_RASC], val)
      fout_prj.write(val_out)
    ####

    # write alg_block_top.v
    fout_prj.write('verilog work ./src/alg_block_top.v\n')

    # by default alg-block.vhd and rc100.vhd
    for val in directives_map[VHDL_FILES_EXTRA]: 
      fout_prj.write( 'vhdl work ./src/' + val + '\n' )
      copy_files_to_dir( [ '../' + val ], SOURCE_DIR )

    # the dp, buffer, and ROCCC_control.vhd and ROCCC_utility_lib
    for val in directives_map[VHDL_FILES_DEFAULT]: 
      fout_prj.write( 'vhdl work ./src/' + val + '\n' )
      ROCCC_PY_LIB.copy_files_to_dir( [ '../' + val ], SOURCE_DIR )


    # WRITE XST script file to top_entity_name.scr
    fout_scr = open( top_entity_name + '.scr', 'w' )
    for val in directives_map[SYNTHESIS_OPTIONS]: 
      fout_scr.write( val )
    fout_scr.close();

  ###################
  elif DO_RASC_SYNPLIFY_PRO : 
    
    fout_prj.write('set RASC [get_env RASC]\n')
    fout_prj.write('set THE_PWD [get_env PWD]\n')
    # FIXME, met it relative and see if works, otherwise absolute will have to do
    #fout_prj.write('set ALG_DIR ' + os.environ['PWD'] + '/SYNTHESIS_DIR/src\n')
    fout_prj.write('set ALG_DIR $THE_PWD/src\n')
    fout_prj.write('add_file -verilog "$RASC/design/alg_core/templates/user_space_wrapper.v"\n')
    fout_prj.write('add_file -verilog "$RASC/design/alg_core/templates/acs_adr.v"\n')
    fout_prj.write('add_file -verilog "$RASC/design/alg_core/templates/acs_debug_reg.v"\n')
    fout_prj.write('add_file -verilog "$ALG_DIR/alg_block_top.v"\n')

    for val in directives_map[VHDL_FILES_EXTRA]: 
      fout_prj.write( 'add_file -vhdl "$ALG_DIR/' + val + '"\n' )
      ROCCC_PY_LIB.copy_files_to_dir( [ '../' + val ], SOURCE_DIR )

    # the dp, buffer, and ROCCC_control.vhd and ROCCC_utility_lib
    for val in directives_map[VHDL_FILES_DEFAULT]: 
      fout_prj.write( 'add_file -vhdl "$ALG_DIR/' + val + '"\n' )
      ROCCC_PY_LIB.copy_files_to_dir( [ '../' + val ], SOURCE_DIR )


    for val in directives_map[PROJECT_OPTIONS]: 
      val_out = val
      #pat_env_var_RASC.sub( os.environ[ROCCC_PY_LIB.CONST_ENV_VAR_RASC], val)
      fout_prj.write(val_out)
 

  ###################
  fout_prj.close()



  #####
  # copy RASC default makefile, by parsing it and changing several options to good one: 
  # make ucf -> make ucf_new (fix problem with reference to $RASC/implementations
  makefile_IN_filename = os.environ[ROCCC_PY_LIB.CONST_ENV_VAR_RASC] + '/implementations/templates/Makefile'
  makefile_OUT_filename = './Makefile'

  ROCCC_PY_LIB.pattern_replace_and_copy_to_new_filepath(
      makefile_IN_filename, makefile_OUT_filename, 
      [ 
        [ '\${RASC}/implementations/\${SYNTHESIS_PROJ}/\${SYNTHESIS_PROJ}.ucf' , './${SYNTHESIS_PROJ}.ucf' ], 
        [ 'python2.4', 'python' ]
      ] 
    )
  #####

  # Set to EXPORT directory because I want user to be able to modify it for RASC
  #RASC_CUSTOM_LIB_PATH = os.environ[ROCCC_PY_LIB.CONST_ENV_VAR_ROCCC_HOME] + '/src/roccc_lib/vhdl_lib/RASC-specific'
  RASC_CUSTOM_LIB_PATH = '..'

  #####
  # copy RASC makefile.local by parsing it and changing only the alg name
  makefile_local_IN_filename  = RASC_CUSTOM_LIB_PATH + '/Makefile.local.RASC-SYNTHESIS'
  makefile_local_OUT_filename = './Makefile.local'

  if DO_RASC_XST : 
    SYNTHESIS_RESULT_EXT = 'ngc'
    SYNTHESIS_TOOL = 'ise_xst'
  elif DO_RASC_SYNPLIFY_PRO : 
    SYNTHESIS_RESULT_EXT = 'edf'
    SYNTHESIS_TOOL = 'synplify_pro'
 
  ROCCC_PY_LIB.pattern_replace_and_copy_to_new_filepath(
      makefile_local_IN_filename, makefile_local_OUT_filename, 
      [  
        [ 'THE_DEFAULT_ALGORITHM_NAME', top_entity_name ], 
        [ 'THE_DEFAULT_SOURCE_DIR' , SOURCE_DIR ],
        [ 'THE_DEFAULT_SYNTHESIS_TOOL' , SYNTHESIS_TOOL ], 
        [ 'THE_DEFAULT_SYNTHESIS_RESULT_EXT' , SYNTHESIS_RESULT_EXT ] 
      ] 
    )
  #####


  # copy over the extra files needed: alg.h, alg_block_top.v
  ROCCC_PY_LIB.copy_filepath_to_filepath( 
      RASC_CUSTOM_LIB_PATH + '/alg_block_top.v' , 
      SOURCE_DIR + '/alg_block_top.v' )

  ROCCC_PY_LIB.copy_filepath_to_filepath( 
      RASC_CUSTOM_LIB_PATH + '/alg.h' , 
      SOURCE_DIR + '/alg.h' )


  # don't do make all and copy files if not wanted
  if not DO_STOP_BEFORE_MAKE : 
    ###################
    # EXECUTION 
    ###################

    ROCCC_PY_LIB.execute_shell_cmd( 'make all' )

    ###################
    # COPY RESULTS OUT
    ###################
    # copy out results (the RASC ./rev_1/${ENTITY_NAME}.bin, ./user_space.cfg, ./core_services.cfg)
    # to the ../RASC_EXPORT directory

    # append username for uniqueness of filenames, FIXME to add date+timestamp as well
    try:
      os.environ['USER']
      THE_USERNAME = '_' + os.environ['USER']
    except:
      THE_USERNAME = ''

    TOP_LEVEL_ENTITY_NAME = top_entity_name + THE_USERNAME

    ROCCC_PY_LIB.copy_files_to_dir( [ './user_space.cfg', './core_services.cfg' ], RASC_EXPORT_DIR )
    ROCCC_PY_LIB.copy_filepath_to_filepath('./rev_1/' + top_entity_name + '.bin', RASC_EXPORT_DIR + '/' + TOP_LEVEL_ENTITY_NAME + '.bin' )

    # copy Makefile for sgi-2 
    ROCCC_PY_LIB.pattern_replace_and_copy_to_new_filepath(
        RASC_CUSTOM_LIB_PATH + '/Makefile.C-COMPILE-SGI-2', 
        RASC_EXPORT_DIR + '/Makefile', 
        [  
          [ 'DEFAULT_ALGORITHM_NAME', TOP_LEVEL_ENTITY_NAME ]
        ] 
      )

    # copy ../$HOST_CODE to $RASC_EXPORT_DIR/$HOST_CODE
    ROCCC_PY_LIB.pattern_replace_and_copy_to_new_filepath(
        RASC_CUSTOM_LIB_PATH + '/' + directives_map[HOST_C_SW], 
        RASC_EXPORT_DIR + '/' + directives_map[HOST_C_SW], 
        [  
          [ 'DEFAULT_ALGORITHM_NAME', TOP_LEVEL_ENTITY_NAME ]
        ] 
      )
    # finished running make and copied files

  #### 
    



###########################################################
### Begin program #########################################
###########################################################
if __name__ == "__main__":

  # set our script name
  ROCCC_PY_LIB.SCRIPT_NAME = 'VHDL2FGPA'

  directives_filename = parse_args()
  directives_map = read_directives(directives_filename)

  if DO_RASC : 
    compile_synthesis_for_RASC(directives_map)
  else: 
    compile_synthesis_manually(directives_map)
    

  ROCCC_PY_LIB.debug_print('########################################################') 
  ROCCC_PY_LIB.debug_print('<<<<<< COMPILE LOCAL (VHDL TO FPGA) JOB END <<<<<<')
  ROCCC_PY_LIB.debug_print('########################################################') 
