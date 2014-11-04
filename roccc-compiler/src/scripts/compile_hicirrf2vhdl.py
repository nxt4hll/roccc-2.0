#!/usr/bin/env python

# This script compiles the chosen file(s) or directory 

import os
import re
import sys
import getopt

# from roccc_python_lib.py
import roccc_python_lib as ROCCC_PY_LIB 

### Define constants ##########################################################

# Executables
GCC2SUIF = 'gcc2suif'

# String suffixes
CONST_HICIRRF_SUFFIX='-hicirrf.c'
CONST_LOOP_DPT_SUFFIX='-loop.dpt'
CONST_C_SUFFIX='.c'


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
ERROR_LOWCIRRF_GRAPH=7
ERROR_NO_COMPILE_DONE=8
ERROR_INVALID_RANGE=9

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

### PASS CONSTANTS ############################################################

CONST_PASS_DO_START   = 1
CONST_PASS_DO_GCC2SUIF= 1 + CONST_PASS_DO_START
CONST_PASS_DO_LOWER   = 1 + CONST_PASS_DO_GCC2SUIF
CONST_PASS_DO_S2M     = 1 + CONST_PASS_DO_LOWER
CONST_PASS_DO_IL2CFG  = 1 + CONST_PASS_DO_S2M
CONST_PASS_DO_CFG2SSA = 1 + CONST_PASS_DO_IL2CFG
CONST_PASS_DO_SSA2CFG = 1 + CONST_PASS_DO_CFG2SSA
CONST_PASS_DO_DCESSA  = 1 + CONST_PASS_DO_SSA2CFG
CONST_PASS_DO_PEEP    = 1 + CONST_PASS_DO_DCESSA
# LOW CIRRF PASSES
CONST_PASS_DO_PREPROCESS      = 1 + CONST_PASS_DO_PEEP
CONST_PASS_DO_FINAL_CHECK_1   = 0 #1 + CONST_PASS_DO_PREPROCESS
#CONST_PASS_DO_KERNELCFG2DF    = 1 + CONST_PASS_DO_FINAL_CHECK_1
CONST_PASS_DO_KERNELCFG2DF    = 1 + CONST_PASS_DO_PREPROCESS
CONST_PASS_DO_EXPORTBITWIDTH  = 1 + CONST_PASS_DO_KERNELCFG2DF
CONST_PASS_DO_BITRESIZING     = 1 + CONST_PASS_DO_EXPORTBITWIDTH
CONST_PASS_DO_PLD             = 1 + CONST_PASS_DO_BITRESIZING
CONST_PASS_DO_PLD_GIPCORE     = 1 + CONST_PASS_DO_PLD
#CONST_PASS_DO_RCE             = 1 + CONST_PASS_DO_PLD_GIPCORE
#CONST_PASS_DO_CFS             = 1 + CONST_PASS_DO_RCE
CONST_PASS_DO_CFS             =  0 #1 + CONST_PASS_DO_PLD_GIPCORE
CONST_PASS_DO_FINAL_CHECK_2   = 0 #1 + CONST_PASS_DO_CFS
#CONST_PASS_DO_HDLGEN         = 1 + CONST_PASS_DO_FINAL_CHECK_2
CONST_PASS_DO_HDLGEN         = 1 + CONST_PASS_DO_PLD_GIPCORE
CONST_PASS_DO_TEXTIRGEN       = 0 #1 + CONST_PASS_DO_PLD_GIPCORE


# Range of passes to execute
START_RANGE = CONST_PASS_DO_GCC2SUIF
END_RANGE   = CONST_PASS_DO_HDLGEN




pass_dict = { }
pass_dict[CONST_PASS_DO_START]           = "EMPTY"
pass_dict[CONST_PASS_DO_GCC2SUIF]        = "gcc2suif"
pass_dict[CONST_PASS_DO_LOWER]           = "do_lower"
pass_dict[CONST_PASS_DO_S2M]             = "do_s2m"
pass_dict[CONST_PASS_DO_IL2CFG]          = "do_il2cfg"
pass_dict[CONST_PASS_DO_CFG2SSA]         = "do_cfg2ssa -build_minimal_form"
pass_dict[CONST_PASS_DO_SSA2CFG]         = "do_ssa2cfg"
pass_dict[CONST_PASS_DO_DCESSA]          = "do_dcessa"
pass_dict[CONST_PASS_DO_PEEP]            = "do_peep"
pass_dict[CONST_PASS_DO_PREPROCESS]      = "do_preprocess"
pass_dict[CONST_PASS_DO_KERNELCFG2DF]    = "do_kernelcfg2df"
pass_dict[CONST_PASS_DO_EXPORTBITWIDTH]  = "do_export_bitwidth"
pass_dict[CONST_PASS_DO_BITRESIZING]     = "do_bitresizing"
pass_dict[CONST_PASS_DO_PLD]             = "do_pld"
pass_dict[CONST_PASS_DO_PLD_GIPCORE]     = "do_pld_gipcore"
#pass_dict[CONST_PASS_DO_RCE]             = "do_rce"
pass_dict[CONST_PASS_DO_CFS]             = "do_cfs"
pass_dict[CONST_PASS_DO_FINAL_CHECK_1]   = "do_final_check"
pass_dict[CONST_PASS_DO_FINAL_CHECK_2]   = "do_final_check"
pass_dict[CONST_PASS_DO_TEXTIRGEN]       = "do_textIRgen"
pass_dict[CONST_PASS_DO_HDLGEN]         = "do_hdlgen"

pass_suffix_dict = { }
pass_suffix_dict[CONST_PASS_DO_START]           = "c"
pass_suffix_dict[CONST_PASS_DO_GCC2SUIF]        = "suif"
pass_suffix_dict[CONST_PASS_DO_LOWER]           = "lsf"
pass_suffix_dict[CONST_PASS_DO_S2M]             = "svm"
pass_suffix_dict[CONST_PASS_DO_IL2CFG]          = "cfg"
pass_suffix_dict[CONST_PASS_DO_CFG2SSA]         = "ssa"
pass_suffix_dict[CONST_PASS_DO_SSA2CFG]         = "ssacfg"
pass_suffix_dict[CONST_PASS_DO_DCESSA]          = "dcessa"
pass_suffix_dict[CONST_PASS_DO_PEEP]            = "peep"
pass_suffix_dict[CONST_PASS_DO_PREPROCESS]      = "ppr"
pass_suffix_dict[CONST_PASS_DO_KERNELCFG2DF]    = "df"
pass_suffix_dict[CONST_PASS_DO_EXPORTBITWIDTH]  = "xbw"
pass_suffix_dict[CONST_PASS_DO_BITRESIZING]     = "brs"
pass_suffix_dict[CONST_PASS_DO_PLD]             = "pld"
pass_suffix_dict[CONST_PASS_DO_PLD_GIPCORE]     = "pld_gipcore"
#pass_suffix_dict[CONST_PASS_DO_RCE]             = "pld_rce"
pass_suffix_dict[CONST_PASS_DO_CFS]             = "cfs"
pass_suffix_dict[CONST_PASS_DO_FINAL_CHECK_1]   = "fck1"
pass_suffix_dict[CONST_PASS_DO_FINAL_CHECK_2]   = "fck2"
pass_suffix_dict[CONST_PASS_DO_TEXTIRGEN]       = "tir"
pass_suffix_dict[CONST_PASS_DO_HDLGEN]         = "hdl"

pass_prefix_dict = { }
pass_prefix_dict[CONST_PASS_DO_START]           = "XXXXXXX"
pass_prefix_dict[CONST_PASS_DO_GCC2SUIF]        = "COMPILE"
pass_prefix_dict[CONST_PASS_DO_LOWER]           = "PASS   "
pass_prefix_dict[CONST_PASS_DO_S2M]             = "PASS   "
pass_prefix_dict[CONST_PASS_DO_IL2CFG]          = "PASS   "
pass_prefix_dict[CONST_PASS_DO_CFG2SSA]         = "PASS   "
pass_prefix_dict[CONST_PASS_DO_SSA2CFG]         = "PASS   "
pass_prefix_dict[CONST_PASS_DO_DCESSA]          = "PASS   "
pass_prefix_dict[CONST_PASS_DO_PEEP]            = "PASS   "
pass_prefix_dict[CONST_PASS_DO_PREPROCESS]      = "PASS-LC"
pass_prefix_dict[CONST_PASS_DO_KERNELCFG2DF]    = "PASS-LC"
pass_prefix_dict[CONST_PASS_DO_EXPORTBITWIDTH]  = "PASS-LC"
pass_prefix_dict[CONST_PASS_DO_BITRESIZING]     = "PASS-LC"
pass_prefix_dict[CONST_PASS_DO_PLD]             = "PASS-LC"
pass_prefix_dict[CONST_PASS_DO_PLD_GIPCORE]     = "PASS-LC"
#pass_prefix_dict[CONST_PASS_DO_RCE]             = "PASS-LC"
pass_prefix_dict[CONST_PASS_DO_CFS]             = "PASS-LC"
pass_prefix_dict[CONST_PASS_DO_FINAL_CHECK_1]   = "PASS-LC"
pass_prefix_dict[CONST_PASS_DO_FINAL_CHECK_2]   = "PASS-LC"
pass_prefix_dict[CONST_PASS_DO_TEXTIRGEN]       = "PASS-LC"
pass_prefix_dict[CONST_PASS_DO_HDLGEN]         = "PASS-LC"


pass_name_to_int_dict = { }
pass_name_to_int_dict["C"           ] = CONST_PASS_DO_START
pass_name_to_int_dict["SUIF"        ] = CONST_PASS_DO_GCC2SUIF
pass_name_to_int_dict["LSF"         ] = CONST_PASS_DO_LOWER
pass_name_to_int_dict["SVM"         ] = CONST_PASS_DO_S2M
pass_name_to_int_dict["CFG"         ] = CONST_PASS_DO_IL2CFG
pass_name_to_int_dict["SSA"         ] = CONST_PASS_DO_CFG2SSA
pass_name_to_int_dict["SSACFG"      ] = CONST_PASS_DO_SSA2CFG
pass_name_to_int_dict["DCESSA"      ] = CONST_PASS_DO_DCESSA
pass_name_to_int_dict["PEEP"        ] = CONST_PASS_DO_PEEP
pass_name_to_int_dict["PPR"         ] = CONST_PASS_DO_PREPROCESS
pass_name_to_int_dict["DF"          ] = CONST_PASS_DO_KERNELCFG2DF
pass_name_to_int_dict["XBW"         ] = CONST_PASS_DO_EXPORTBITWIDTH
pass_name_to_int_dict["BRS"         ] = CONST_PASS_DO_BITRESIZING
pass_name_to_int_dict["PLD"         ] = CONST_PASS_DO_PLD
pass_name_to_int_dict["PLD_GIPCORE" ] = CONST_PASS_DO_PLD_GIPCORE
#pass_name_to_int_dict["PLD_RCE"     ] = CONST_PASS_DO_RCE
pass_name_to_int_dict["CFS"         ] = CONST_PASS_DO_CFS
pass_name_to_int_dict["FCK1"        ] = CONST_PASS_DO_FINAL_CHECK_1
pass_name_to_int_dict["FCK2"        ] = CONST_PASS_DO_FINAL_CHECK_2
pass_name_to_int_dict["TEXTIR"      ] = CONST_PASS_DO_TEXTIRGEN
pass_name_to_int_dict["HDL"         ] = CONST_PASS_DO_HDLGEN




### Global Variables ##########################################################

# Do graph printing
DO_PRINT_GRAPH=False

# Do default synthesis
DO_DEFAULT_SYNTHESIS=False
DO_DEFAULT_SYNTHESIS_XST_ONLY=False

# Do RASC synthesis
DO_RASC_SYNTHESIS=False




### Define functions ##########################################################

#######################
# Argument Processing
#######################
def print_help():
  """Print Help for local code"""
  print "\nCompile local directory's contents"
  print "\nUsage: " + sys.argv[0] + "[OPTIONS] <FileNamePrefix>.[EXTENSION_OPTIONS]"
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
        Output should be called low_cirrf_df.pdf(ps) and low_cirrf_cfg.pdf(ps)

    -q, --quiet
        Turns off default printing of information in on screen.  Good if running
        a testbench to see if a compilation pass fails.

    --default-synthesis
        Enable run of default xilinx synthesis all the way from SYR to TRCE.
        (can not be used with --RASC option)

    --default-synthesis-xst-only
        Same as above (--default-synthesis) but stop after SYR.

    --RASC
        Do the same as the (--default-synthesis) except make the RASC library the 
        top level and compile all the way down to 3 files.  The bit file to upload
        to the board and the 2 configuration files for dev_mgr.  These 3 files will
        be located at ./EXPORT/RASC_EXPORT
        (can not be used with --default-synthesis option)


  [EXTENSION OPTIONS]
        Form of START_EXTENSION:END_EXTENSION

        Runs all necessary passes to produce files with listed extensions. 
        Example file-hicirrf.peep:df will run preprocess and then kernelcfg2df phases.

        Extensions can be one of the following, in lowercase (They are listed in compile order) :
        C,SUIF,LSF,CFG,SSA,SSACFG,DCESSA,PEEP,
        PPR,DF,XBW,PLD,PLD_GIPCORE,PLD_RCE,CFS,HDL
  """


def parse_args(local_argv=None):
  """Parse Arguments to find File Options and working directory"""

  global DO_PRINT_GRAPH
  global DO_DEFAULT_SYNTHESIS
  global DO_DEFAULT_SYNTHESIS_XST_ONLY
  global DO_RASC_SYNTHESIS

  # do not use default level 
  VERBOSE_LEVEL = 1 #ROCCC_PY_LIB.VERBOSE_LEVEL


  rangeL_int = -1
  rangeR_int = -1

  if local_argv is None:
    local_argv = sys.argv
    
  # parse command line options
  try:
    opts, args = getopt.gnu_getopt(local_argv[1:], "hvgq", ["quiet", "help", "verbose", "graph", "default-synthesis", "default-synthesis-xst-only", "RASC"])
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

    if o in ("-q", "--quiet"):
      VERBOSE_LEVEL = 0
    
    elif o in ("-v", "--verbose"):
      VERBOSE_LEVEL += 1

    elif o in ("-g", "--graph"):
      DO_PRINT_GRAPH = True

    elif o in ("--default-synthesis", "--default-synthesis-xst-only") : 
      DO_DEFAULT_SYNTHESIS = True 
      if o == "--default-synthesis-xst-only" : 
        DO_DEFAULT_SYNTHESIS_XST_ONLY = True

    elif o == "--RASC":
      DO_RASC_SYNTHESIS = True 


  # set verbosity
  ROCCC_PY_LIB.VERBOSE_LEVEL = VERBOSE_LEVEL 


  if DO_DEFAULT_SYNTHESIS and DO_RASC_SYNTHESIS : 
    ROCCC_PY_LIB.error_print(ERROR_INVALID_ARG_MIX, "Can not use --RASC and --default-synthesis options together")
     
  if len(args) != 1 : 
    ROCCC_PY_LIB.error_print(ERROR_INVALID_ARG_MIX, "Must specify only one hicirrf input file")
  
  full_filename = args[0]

  # Take out prefix and split extension
  ext_temp = re.split('[.]',full_filename)
  hicirrf_filename_prefix = ext_temp[0]

  # split extension to determine ranges
  ext_range_temp = re.split(':', ext_temp[1])

  if len(ext_range_temp) == 1:
    if ext_temp[1].upper() != 'C' : 
      ROCCC_PY_LIB.error_print(ERROR_INVALID_RANGE, 'Invalid extension format, must be a *.c file if not defining a range')

    hicirrf_filename = full_filename
    ROCCC_PY_LIB.debug_print('ext = .c, hicirrf_filename = ' + hicirrf_filename)

    rangeL_int = START_RANGE
    rangeR_int = END_RANGE 

  elif len(ext_range_temp) == 2: 

    rangeL_str = ext_range_temp[0].upper()
    rangeR_str = ext_range_temp[1].upper()

    if pass_name_to_int_dict.has_key(rangeL_str):
      rangeL_int = pass_name_to_int_dict[rangeL_str] + 1

    if pass_name_to_int_dict.has_key(rangeR_str):
      rangeR_int = pass_name_to_int_dict[rangeR_str]

    if rangeR_int < rangeL_int :
      ROCCC_PY_LIB.error_print(ERROR_INVALID_RANGE, "Range must be in linear monotonic order(%d:%d)"%(rangeL_int, rangeR_int))

    hicirrf_filename = hicirrf_filename_prefix + DOT + ext_range_temp[0]

  else:
    ROCCC_PY_LIB.error_print(ERROR_INVALID_RANGE, 'Inavlid extension format. See help file with -h option')


  #ROCCC_PY_LIB.debug_print('hicirrf input file arg = \"' + hicirrf_filename + '\"')
  #ROCCC_PY_LIB.debug_print('compile_range = %d : %d' % (rangeL_int, rangeR_int) )

  ######
  # Error checking to make sure the options are valid
  ######
  # hicirrf_file option can not be used with other two


  if not os.path.isfile(hicirrf_filename):
    ROCCC_PY_LIB.error_print(ERROR_PARSE_ARG, 'File \"' + hicirrf_filename + '\" does not exist')

  return (hicirrf_filename, rangeL_int, rangeR_int)


###########################################################
### Begin program #########################################
###########################################################
if __name__ == "__main__":

  FINAL_RETURN_VALUE = ERROR_NO_COMPILE_DONE  
  ROCCC_PY_LIB.SCRIPT_NAME = 'HICIRRF2VHDL'

  # ensure environment variables are set because we can call the compiled programs
  # without having to reference the entire path
  ROCCC_PY_LIB.check_env_var( ROCCC_PY_LIB.CONST_ENV_VAR_ROCCC_HOME )

  # parse from command line
  hicirrf_file, PassRangeL, PassRangeR = parse_args()

  ROCCC_PY_LIB.debug_print('########################################################') 
  ROCCC_PY_LIB.debug_print('>>>>>> COMPILE LOCAL (HICIRRF TO VHDL) JOB START >>>>>>')
  ROCCC_PY_LIB.debug_print('########################################################') 
  ROCCC_PY_LIB.debug_print('VERBOSE level = %d' % ROCCC_PY_LIB.VERBOSE_LEVEL)
  ROCCC_PY_LIB.debug_print('DO GRAPH OUTPUT = %d' % DO_PRINT_GRAPH)
  ROCCC_PY_LIB.debug_print('DO DEFAULT SYNTHESIS= %d' % DO_DEFAULT_SYNTHESIS)
  ROCCC_PY_LIB.debug_print('DO DEFAULT SYNTHESIS XST ONLY= %d' % DO_DEFAULT_SYNTHESIS_XST_ONLY)
  ROCCC_PY_LIB.debug_print('DO RASC SYNTHESIS= %d' % DO_RASC_SYNTHESIS)
  ROCCC_PY_LIB.debug_print("PROCESSING DIR = '%s'" % os.getcwd() )

  verbose_str_list = [ ]
  for i in range(ROCCC_PY_LIB.VERBOSE_LEVEL-1):
    if len(verbose_str_list) == 0 : 
      verbose_str_list.append('-v')
    else:
      verbose_str_list.append('v')
  verbose_str = ''.join(verbose_str_list)


  if PassRangeL == -1 or PassRangeR == -1:
    PassRangeL = START_RANGE
    PassRangeR = END_RANGE


  # determine compile stage
  compile_stage = STAGE_HICIRRF_TO_VHDL

  ROCCC_PY_LIB.debug_print("HICIRRF_FILE = '%s'" % hicirrf_file )
  ROCCC_PY_LIB.debug_print('COMPILE_RANGE : ' + pass_suffix_dict[PassRangeL-1].upper() + 
    '(%d)'% (PassRangeL-1) + ' => ' +  pass_suffix_dict[PassRangeR].upper() + '(%d)'%PassRangeR )



  # At this point in this file, we know 3 things
  # 1) We are in the working directory
  # 2) What set of compilations to do (rangeL and rangeR are defined)

  
  ##########################################
  ### lowcirrf to vhdl compilation stage ###
  ##########################################

  ROCCC_PY_LIB.debug_print('#################################')
  ROCCC_PY_LIB.debug_print('Performing Hi-Cirrf to VHDL stage...')
  ROCCC_PY_LIB.debug_print('#################################')

  hicirrf_filename = hicirrf_file 
  hicirrf_prefix = os.path.splitext(hicirrf_filename)[0]

  ROCCC_PY_LIB.debug_print("hicirrf_filename = " + hicirrf_filename);
  ROCCC_PY_LIB.debug_print("hicirrf_prefix = " + hicirrf_prefix);
        
  # Assert that files exist
  assert os.path.isfile(hicirrf_filename), 'Cannot access file ' + hicirrf_filename

  # Set verbosity debug level for commands
  # If low verbose print less info, if more verbose then make
  # more stuff present on console

  for i in range( PassRangeL , PassRangeR + 1 ):

    if i == CONST_PASS_DO_GCC2SUIF :
      cmd_pass_process = pass_dict[i] + SPACE + hicirrf_prefix + DOT + pass_suffix_dict[i-1]
    else : 
      cmd_pass_process = pass_dict[i] + SPACE + hicirrf_prefix + DOT + pass_suffix_dict[i-1] + SPACE + hicirrf_prefix + DOT + pass_suffix_dict[i]
  
    if ROCCC_PY_LIB.VERBOSE_LEVEL == 1:
      os.system( "echo " + DOUBLEQUOTE + pass_prefix_dict[i] + SPACE + COLON + SPACE + cmd_pass_process + DOUBLEQUOTE )

    ROCCC_PY_LIB.execute_shell_cmd( cmd_pass_process )

    if DO_PRINT_GRAPH == True and i == CONST_PASS_DO_CFS :
      ROCCC_PY_LIB.execute_shell_cmd( "do_print_dot_graph" + SPACE + hicirrf_prefix + DOT + pass_suffix_dict[i] )


  if DO_PRINT_GRAPH == True: 
    if not os.path.isfile('low_cirrf_df.pdf'):
      ROCCC_PY_LIB.error_print(ERROR_LOWCIRRF_GRAPH)

#   if PassRangeR  == CONST_PASS_DO_HDLGEN: 
#     # copy all VHDL LIB FILES LOCALLY so that we can export them
#     EXPORT_DIR = './EXPORT';
#     if not os.path.isdir(EXPORT_DIR):
#       os.mkdir(EXPORT_DIR);
# 
#     VHDL_LIB_FILEPATH_PREFIX  = os.environ[ROCCC_PY_LIB.CONST_ENV_VAR_ROCCC_HOME] + '/src/roccc_lib/vhdl_lib'
#     SCRIPT_FILEPATH_PREFIX    = os.environ[ROCCC_PY_LIB.CONST_ENV_VAR_ROCCC_HOME] + '/src/scripts'
#     RASC_CUSTOM_LIB_PATH=  VHDL_LIB_FILEPATH_PREFIX + '/RASC-specific'
# 
#     ROCCC_PY_LIB.copy_files_to_dir( [ 
#         VHDL_LIB_FILEPATH_PREFIX + '/ROCCC_control.vhd', 
#         VHDL_LIB_FILEPATH_PREFIX + '/ROCCC_utility_lib.vhd', 
#         VHDL_LIB_FILEPATH_PREFIX + '/rc100.vhd', 
#         './compute_sw.c', 
#         './DIRECTIVES.dat', 
#         RASC_CUSTOM_LIB_PATH + '/Makefile.local.RASC-SYNTHESIS' , 
#         RASC_CUSTOM_LIB_PATH + '/Makefile.C-COMPILE-SGI-2' ], 
#       EXPORT_DIR )
# 
# 
#     # copy the vhdl files to EXPORT dir
#     ROCCC_PY_LIB.execute_shell_cmd( 'cp ' + './*.vhd' + SPACE + EXPORT_DIR )
# 
#     # copy over the extra files needed: alg.h, alg_block_top.v
#     ROCCC_PY_LIB.copy_filepath_to_filepath( 
#       RASC_CUSTOM_LIB_PATH + '/alg_block_top.v.RASC-SYNTHESIS' , 
#       EXPORT_DIR + '/alg_block_top.v' )
# 
#     ROCCC_PY_LIB.copy_filepath_to_filepath( 
#       RASC_CUSTOM_LIB_PATH + '/alg.h.RASC-SYNTHESIS' , 
#       EXPORT_DIR + '/alg.h' )


    # Run synthesis if told to do so
    if DO_DEFAULT_SYNTHESIS == True or DO_RASC_SYNTHESIS == True : 
      ROCCC_PY_LIB.debug_print('CHDIR to ' + EXPORT_DIR)
      os.chdir(EXPORT_DIR);

      if DO_DEFAULT_SYNTHESIS : 
        if DO_DEFAULT_SYNTHESIS_XST_ONLY : 
          cmd_do_synthesis = 'compile_vhdl2fpga.py --default-synthesis-xst-only' + SPACE + verbose_str 
        else : 
          cmd_do_synthesis = 'compile_vhdl2fpga.py --default-synthesis' + SPACE + verbose_str 

      elif DO_RASC_SYNTHESIS : 
        cmd_do_synthesis = 'compile_vhdl2fpga.py --RASC' + SPACE + verbose_str

      ROCCC_PY_LIB.execute_shell_cmd( cmd_do_synthesis ) 

      # go back to main directory
      os.chdir('../')

    # End running synthesis
    
  FINAL_RETURN_VALUE = ERROR_NONE

  ######################################
  # End LOWCIRRF_TO_VHDL COMPILE STAGE
  ######################################

  ROCCC_PY_LIB.debug_print('########################################################') 
  ROCCC_PY_LIB.debug_print('<<<<<< COMPILE LOCAL (HICIRRF TO VHDL) JOB END <<<<<<')
  ROCCC_PY_LIB.debug_print('########################################################') 
