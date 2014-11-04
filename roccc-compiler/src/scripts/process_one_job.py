#!/usr/bin/env python

import os
import re
import sys
import smtplib
from email.MIMEMultipart import MIMEMultipart
from email.MIMEBase import MIMEBase
from email.MIMEText import MIMEText
from email.Utils import COMMASPACE, formatdate
from email import Encoders

### Define constants ##########################################################
SOURCE_EXTENSION = 'c'
PASS_EXTENSION = 'pass'

# Email addresses
FROM_EMAIL_ADDRESS = 'ROCCC Team <roccc@cs.ucr.edu>'

# Master directories
ROOT_DIRECTORY = '/home/www-data'

# Job directories
JOBS_DIRECTORY = ROOT_DIRECTORY + '/jobs'
COMPILE_DIRECTORY = ROOT_DIRECTORY + '/compile'

# Executables/scripts
SOURCE2SUIF = 'gcc2suif'
SOURCE2HICIRRF2VHDL = 'compile_c2hicirrf2vhdl.py -v --graph'

# Directory names
DOWNLOAD_DIRECTORY_NAME = 'download'

# Filenames
LOCK_FILENAME = 'COMPILE_LOCK'
USERINFO_FILENAME = 'ROCCC_USER.txt'
LOG_FILENAME = '.log'
ERRORLOG_FILENAME = 'COMPILE_LOG.txt'
FAILED_FILENAME = 'FAILED'
SUCCESS_FILENAME = 'SUCCESS'
HICIRRF_FILENAME = 'HICIRRF'
HEADER_FILENAME = 'roccc.h'
LOW_CIRRF_PS_FILENAME = 'low_cirrf_df.ps' # filename mentioned in trunk/roccc-compiler/bin/do_print_dot_graph
LOW_CIRRF_PDF_FILENAME = 'low_cirrf_df.pdf' # filename mentioned in trunk/roccc-compiler/bin/do_print_dot_graph

# Misc
APACHE_USERNAME = 'www-data'


SPACE=' '
COMMA=','
COLON=':'
SEMICOLON=';'
QUOTE='"'
COMMENT='//'
CURR_DIR='./'
PREV_DIR='../'

# verbosity
VERBOSE_LEVEL = 1

# Environment variables
CONST_ENV_VAR_ROCCCHOME='ROCCC_HOME'
CONST_ENV_VAR_NCIHOME='NCIHOME'
CONST_ENV_VAR_MACHSUIFHOME='MACHSUIFHOME'


### Define functions ##########################################################

#######################
# SEND MAIL
#######################
# send_mail - Sends an email with support for attachments
# Precondition:
#   send_from: an email address string
#   send_to: an array of one or more email addresses
#   subject: a string
#   text: string message
#   files: array of paths of files to attach
# Postcondition: An email is sent to send_to
#
def send_mail(send_from, send_to, subject, text, files=[], server="localhost"):
  assert type(send_to)==list
  assert type(files)==list

  # Compose headers
  msg = MIMEMultipart()
  msg['From'] = send_from
  msg['To'] = COMMASPACE.join(send_to)
  msg['Date'] = formatdate(localtime=True)
  msg['Subject'] = subject

  msg.attach( MIMEText(text) )

  # Attach files
  for file in files:
    try:
      part = MIMEBase('application', "octet-stream")
      part.set_payload( open(file,"rb").read() )
      Encoders.encode_base64(part)
      part.add_header('Content-Disposition',
                      'attachment; filename="%s"' % os.path.basename(file))
      msg.attach(part)
      print 'Attaching "%s"' % os.path.basename(file)
    except:
      print 'Skipping "%s" (cannot read)' % os.path.basename(file)

  # Send email
  smtp = smtplib.SMTP(server)
  smtp.sendmail(send_from, send_to, msg.as_string())
  smtp.close()


#######################
# Create Lock
#######################
# create_lock - Creates a lockfile
# Precondition: none
# Postcondition:
#
def create_lock():
  global JOBS_DIRECTORY, LOCK_FILENAME

  print 'Creating lock file..'
  os.system('touch "%s/%s"' % (JOBS_DIRECTORY, LOCK_FILENAME))

#######################
# Remove lock
#######################
# remove_lock - Removes a lockfile
# Precondition: none
# Postcondition: 
#
def remove_lock():
  global JOBS_DIRECTORY, LOCK_FILENAME
  
  if os.path.isfile(JOBS_DIRECTORY + '/' + LOCK_FILENAME):
    print 'Removing lock file..'
    retval = os.system('rm ' + JOBS_DIRECTORY + '/' + LOCK_FILENAME)
    if retval != 0:
      print 'Error: Could not remove lock file'
  else:
    print 'No lock file to remove'


#######################
# cleanup failed job
#######################
def cleanup_failed_job(parent_directory, files):
  global DOWNLOAD_DIRECTORY_NAME, APACHE_USERNAME, FAILED_FILENAME, COMPILE_DIRECTORY

  # Copy all relevant files to download
  if not os.path.isdir(DOWNLOAD_DIRECTORY_NAME):
    os.mkdir(DOWNLOAD_DIRECTORY_NAME)

  for file in files:
    os.system('cp ' + file + ' ' + DOWNLOAD_DIRECTORY_NAME)
  os.system('chown -hR ' + APACHE_USERNAME + ' ' + DOWNLOAD_DIRECTORY_NAME)

  # Create indicator and move job
  os.system('touch ' + FAILED_FILENAME)
  os.chdir('..')
  os.system('mv "' + job + '" "' + COMPILE_DIRECTORY + '"')

  send_email(False)


#######################
# Send email
#######################
def send_email(success):
  global USERINFO_FILENAME, COMPILE_DIRECTORY

  userinfo_file = COMPILE_DIRECTORY + '/' + job + '/' + USERINFO_FILENAME
  print 'Parsing "' + userinfo_file + '"..'

  f = open(userinfo_file)
  for line in f:
    line = line.rstrip()
    try:
      (variable, value) = line.split(': ')
    except:
      print 'Can\'t parse line "' + line + '"'
      
  dictionary[variable] = value
  f.close()

  # Set variables retrieved from user info file
  print 'Parsing job info..'
  try:
    job_id = dictionary['job_id']
  except:
    job_id = ''
  try:
    firstname = dictionary['firstname']
  except:
    firstname = 'ROCCC'
  try:
    lastname = dictionary['lastname']
  except:
    lastname = 'User'
  try:
    email = dictionary['email']
  except:
    email = ''

  
  # Create email message to send (if an email exists)
  if email != '':
    send_from = FROM_EMAIL_ADDRESS
    send_to = [firstname + ' ' + lastname + ' <' + email + '>']

    if job_id != '':
      subject = 'ROCCC Job #' + job_id + ': ' + source_filename
    else:
      subject = 'ROCCC Job: ' + source_filename

    body    = 'Dear ' + firstname + ' ' + lastname + '\n'
    if success:
      body += '  Here is your compiled VHDL code.\n\n'
      body += '-ROCCC Team\n'
    else:
      body += '  There were compile errors. The error log is attached.\n\n'
      body += '-ROCCC Team\n'


    # Add paths of all files in the download directory to the attachments array
    attachments = []
    download_directory = COMPILE_DIRECTORY + '/' + job + '/' + DOWNLOAD_DIRECTORY_NAME

    # This is if you want to attach ALL files in the downloads directory
    files = os.listdir(download_directory)
    for file in files:
      attachments.append('%s/%s' % (download_directory, file))

    # Send email to user
    print 'Sending email to user..'
    try:
      send_mail(send_from, send_to, subject, body, attachments)
    except:
      print '[Error] Could not send email. Make sure an smtp server is installed.'

  else:
    print 'No email address specified, not sending'

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
    sys.exit(1)


#######################
# ERROR Printing
#######################
def error_print(retval, the_string):
  """Effective method for printing an error message and having it return the correct error."""
    
  print 'ERROR(%d)[%s]: ' % (retval, os.getcwd() ),
  print the_string
  sys.stdout.flush()
  sys.exit(retval)


#######################
# DEBUG Printing
#######################
def debug_print(the_string, level=1):
  """Based on the global variable VERBOSE_LEVEL it will print out the string
    if the VERBOSE_LEVEL exceeds or matches the input level. Useful for debug 
    printing."""

  if VERBOSE_LEVEL >= level:
    print 'PROCESS_ONE_JOB: ',
    print the_string
    sys.stdout.flush()



### Begin program #############################################################

# make sure environment is set up
check_env_vars()

# Assert all globals
assert os.path.isdir(ROOT_DIRECTORY), 'Cannot access directory ' + ROOT_DIRECTORY
assert os.path.isdir(JOBS_DIRECTORY), 'Cannot access directory ' + JOBS_DIRECTORY
assert os.path.isdir(COMPILE_DIRECTORY), 'Cannot access directory ' + COMPILE_DIRECTORY


# List all job directories in /jobs
try:
  jobs = os.listdir(JOBS_DIRECTORY)
except:
  print "Error: Can't open '" + JOBS_DIRECTORY + "'"
  sys.exit(1)


# if directory is empty, exit
if len(jobs) == 0:
  print 'No jobs to process'
  sys.exit(0)


# if a lock exists, do not process
if os.path.isfile(JOBS_DIRECTORY + '/' + LOCK_FILENAME):
  print 'Compiler is busy. Please wait for previous job to finish'
  sys.exit(0)

# else create a lock and continue script
else:
  create_lock()



# Process the first job in the directory
jobs.sort()
job = jobs[0]
print 'Processing: ' + job


# Change into the directory
job_directory = JOBS_DIRECTORY + '/' + job
print 'Changing into', job_directory
try:
  os.chdir(job_directory)
except:
  print 'Error: could not change into', job_directory
  remove_lock()
  sys.exit(1)


# Find the first .c and .pass in the directory
dictionary = {} # contains user variables
files = os.listdir(job_directory)
for file in files:
  (prefix, ext) = os.path.splitext(file)

  if file == USERINFO_FILENAME:
    print 'Parsing "' + USERINFO_FILENAME + '"..'
    f = open(USERINFO_FILENAME)
    for line in f:
      line = line.rstrip()
      try:
        (variable, value) = line.split(': ')
      except:
        print 'Can\'t parse line "' + line + '"'

      dictionary[variable] = value
    f.close()
        
  elif ext == '.' + SOURCE_EXTENSION:
    source_filename = file
    basename = prefix

  elif ext == '.' + PASS_EXTENSION:
    passes_filename = file


# Verify source code exists
try:
  source_filename
except NameError:
  print 'Error: missing .c file'
  cleanup_failed_job('..', [])
  remove_lock()
  sys.exit(1)


# Verify pass file exists
assert not os.path.isfile(HICIRRF_FILENAME) # Disable acceptance of hicirrf files

try:
  passes_filename
except NameError:
  print '[Error] Missing .pass file'
  cleanup_failed_job('..', [source_filename])
  remove_lock()
  sys.exit(1)


# Set variables retrieved from user info file
print 'Parsing job info..'
try:
  job_id = dictionary['job_id']
except:
  job_id = ''
try:
  firstname = dictionary['firstname']
except:
  firstname = 'ROCCC'
try:
  lastname = dictionary['lastname']
except:
  lastname = 'User'
try:
    email = dictionary['email']
except:
  email = ''

print ' Job ID:', job_id
print ' First:', firstname
print ' Last:', lastname
print ' Email:', email


(hicirrf_prefix, hicirrf_ext) = os.path.splitext(source_filename)
hicirrf_filename = hicirrf_prefix + '-hicirrf.c'

# Process job
#cmd_execute_c2hicirrf2vhdl = SOURCE2HICIRRF2VHDL + SPACE + source_filename + SPACE +  passes_filename + ' >& ' + ERRORLOG_FILENAME
cmd_execute_c2hicirrf2vhdl = SOURCE2HICIRRF2VHDL + SPACE + '--job=' + job_id + SPACE + source_filename + SPACE +  passes_filename + ' >& ' + ERRORLOG_FILENAME

print cmd_execute_c2hicirrf2vhdl
retval = os.system(cmd_execute_c2hicirrf2vhdl)
if retval != 0 :
  success = False
  print '[Error] Error compiling source to vhdl'
  # remove tempory compile directories
  os.system('rm -vrf ./compile_hicirrf ./compile_lowcirrf')
  files = [source_filename, passes_filename, ERRORLOG_FILENAME]
  # Append hicirrf file if it got created
  if os.path.isfile(hicirrf_filename): 
    files.append(hicirrf_filename)

  cleanup_failed_job('..', files)
  remove_lock()
  sys.exit(1)

os.system('touch ' + SUCCESS_FILENAME)
success = True


# Create download directory if it doesn't already exist (the web server should
# automatically generate it)
if not os.path.isdir(DOWNLOAD_DIRECTORY_NAME):
  os.mkdir(DOWNLOAD_DIRECTORY_NAME)


# The return files
source_file = source_filename
passes_file = passes_filename
hicirrf_file = hicirrf_filename


cmd_cp_sources = 'cp -v *.vhd ' + source_file + SPACE + passes_file + SPACE + hicirrf_file + SPACE + \
  ERRORLOG_FILENAME + SPACE + LOW_CIRRF_PS_FILENAME + SPACE + LOW_CIRRF_PDF_FILENAME + SPACE + \
  HEADER_FILENAME + SPACE +  DOWNLOAD_DIRECTORY_NAME 

debug_print(cmd_cp_sources)
os.system(cmd_cp_sources)

utility_file = os.environ[CONST_ENV_VAR_ROCCCHOME] + '/src/roccc_lib/vhdl_lib/ROCCC_utility_lib.vhd'
control_file = os.environ[CONST_ENV_VAR_ROCCCHOME] + '/src/roccc_lib/vhdl_lib/ROCCC_control.vhd'

os.system('cp "' + utility_file + '" "' + DOWNLOAD_DIRECTORY_NAME + '/ROCCC_utility_lib.vhd"')
os.system('cp "' + control_file + '" "' + DOWNLOAD_DIRECTORY_NAME + '/ROCCC_control.vhd"')



# gzip all files in the directory
os.chdir(DOWNLOAD_DIRECTORY_NAME)
gzip_filename = 'job' + job_id + '.tar.gz'
os.system('tar cvf - * | gzip -9c > ../' + gzip_filename + '; mv ../' + gzip_filename + ' .')


# Change file permissions for all files in download directory so apache can read them
os.chdir('..')
os.system('chown -hR ' + APACHE_USERNAME + ' ' + DOWNLOAD_DIRECTORY_NAME)

# remove tempory compile directories
os.system('rm -vrf ./compile_hicirrf ./compile_lowcirrf')

# Move job to /compile
print 'Moving to /compile'
os.chdir('..')
cmd = 'mv "' + job + '" "' + COMPILE_DIRECTORY + '"'
os.system(cmd)


# Create email message to send (if an email exists)
if email != '':
  send_from = FROM_EMAIL_ADDRESS
  send_to = [firstname + ' ' + lastname + ' <' + email + '>']

  if job_id != '':
    subject = 'ROCCC Job #' + job_id + ': ' + source_filename
  else:
    subject = 'ROCCC Job: ' + source_filename

  body    = 'Dear ' + firstname + ' ' + lastname + '\n'
  if success:
    body += '  Here is your compiled VHDL code.\n\n'
    body += '-ROCCC Team\n'
  else:
    body += '  There were compile errors. The error log is attached.\n\n'
    body += '-ROCCC Team\n'


  # Add paths of all files in the download directory to the attachments array
  attachments = []
  download_directory = COMPILE_DIRECTORY + '/' + job + '/' + DOWNLOAD_DIRECTORY_NAME

  # This is if you want to attach ALL files in the downloads directory
#  files = os.listdir(download_directory)
#  for file in files:
#    attachments.append('%s/%s' % (download_directory, file))

  # This is if you only want to attach the .tar.gz file
  attachments.append(download_directory + '/' + gzip_filename)


  # Send email to user
  print 'Sending email to user..'
  try:
    send_mail(send_from, send_to, subject, body, attachments)
  except:
    print '[Error] Could not send email. Make sure an smtp server is installed.'

else:
  print 'No email address specified, not sending'

# Remove lock and finish
remove_lock()
print 'done'
