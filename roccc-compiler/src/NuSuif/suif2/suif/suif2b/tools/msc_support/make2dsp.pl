#
# A script to convert a SUIF makefile to a Visual C++ 6.0 project files
# The script is looking for the following names in the gmake makefile :
#		SRCS
#		HEADERS
#       TARGET_LIB
#		PROGRAM, PROGRAM2, PROGRAM3
#		MAIN_OBJ, MAIN2_OBJ, MAIN3_OBJ
#

$tmake 			= 'tmake';
$common_pro 	= '%NCIHOME%\suif\suif2b\tools\msc_support\nci.pro ';
@program_list 	= qw(PROGRAM PROGRAM2 PROGRAM3);
$APP_POSTFIX    = '_app';

# process command line options
foreach $_ (@ARGV){
	if ( /^\-(.*)/ ) {
		s/\s+//g;
		$option = $1;
		if($option eq 'h' || $option eq '?' || $option eq '-help'){
            &usage();
		}

		if($option eq 'keep'){$keep = 1;}
		if($option eq 'quiet'){$quiet = 1;}
        if($option eq 'nodep'){$nodep = 1;}
		#if(/^\-out=(.+)/){$out = $1;}
		#if(/^\-is_exe=(.+)/){$is_exe = int($1);}
	}else{
		$file = $_;
	}
}

# use Makefile by default
unless($file){
	print STDERR "Using Makefile in the current directory...\n" unless $quiet;
	$file = "Makefile";
}

undef $/;
open IN, $file or die "Can't open the input file $file, $!";
$_ = <IN>;

# parse the file
&parse($_);
# make sure the right info is present
&check_consistency();
# combine the info
&extract_values();
# for each of the modules, i.e. executable programs or libraries, generate code
foreach $module (keys %MODULES){
	$_ = $MODULES{$module};
	&generate_code($_);
}
1;

############################## Support routines ###########################
sub parse(){
	$_ = shift;
    s/\\([\s\n]+)/ /mg;                # remove line breaks -- \
    s/^\w*$//mg;                        # remove empty lines

	# search for assignments
	while(
		/^
			(\S+)
			\s*
			\+?=
            [^\n]
            (.*)
        $/gmx)
	{
		($var, $value) = ($1, $2);
		next if $var =~ /^\#/;			# remove comments
		$value =~ s/\#.*/ /g;			# remove comments
		$value =~ s/\s+/ /g;			# remove extra spaces
        print "$var = $value \n";
		$VALUES{$var} .= " ".$value;
	}
}

# Make sure the right information is present
sub check_consistency(){
	if($quiet){
		($VALUES{TARGET_LIB} or  $VALUES{PROGRAM}) or exit(1);
        ($VALUES{HEADERS} or $VALUES{OBJ_FILES}) or exit(1);
	}else{
		($VALUES{TARGET_LIB} or  $VALUES{PROGRAM}) or
			die "At least one of TARGET_LIB and PROGRAM must be defined";

        ($VALUES{HEADERS} or $VALUES{OBJ_FILES}) or die "HEADERS or/and SRCS must be defined";
		$VALUES{LIBS} or warn "Warning: LIBS is not defined";
	}
}

# collect libraries into a list, create a list of modules
sub extract_values(){
    # Create a LIB string. This ignores SLIBs and all that for now
	foreach $_ (split /\s+/, $VALUES{LIBS}){
		if(/-l(\w+)/){
			push @LIBS, "$1.lib";
		}
	}
	$LIB_string = join(' ', @LIBS);

    # Deal with the gmake macros

    # Do substitution of HOOF -- (unconservatively) hoping that the one seen
    # locally it the actual one!!
    local($HOOF) = $VALUES{HOOF};
    if($HOOF){
        $VALUES{OBJ_FILES} =~ s/\$\(HOOF\)/$HOOF/g;
        $VALUES{HEADERS} =~ s/\$\(HOOF\)/$HOOF/g;
        $VALUES{SRCS} =~ s/\$\(HOOF\)/$HOOF/g;
    }

    # get rid of standard_main, it's not really needed -- a unix hack
    $VALUES{OBJ_FILES} =~ s/standard_main.o/ /ig;
    $VALUES{SRCS} =~ s/standard_main.cpp/ /ig;

    # Now process output module definitions and record them in %MODULES hash
    # Start with the library if there's one
	if($VALUES{TARGET_LIB}){
		$VALUES{TARGET_LIB} =~ s/\s+//g;
		!(exists $MODULES{$VALUES{TARGET_LIB}}) or die "Name conflict for a library";
		$VALUES{TARGET_LIB} or die;
		#print "Lib : $VALUES{TARGET_LIB} \n";

        # use LIB_OBJ if it's explicitly defined
        local($OBJS) = $VALUES{LIB_OBJ} ? $VALUES{LIB_OBJ} : $VALUES{OBJ_FILES};

        # TODO: this assumes that the source extension is .cpp!!
        local($SOURCES);
        if ($OBJS = /^\s*\$\(SRCS:\.cpp=\.o\)\s*$/) { # using substitution
            $OBJS = $SOURCES;
            $OBJS =~ s/\.o/\.cpp/g
        }else{
            $SOURCES = $VALUES{SRCS};
        }

        $SOURCES .=  " .\\Debug\\".$VALUES{TARGET_LIB}.".def ";
        $SOURCES .=  " $file ";

		$MODULES{$VALUES{TARGET_LIB}} = {
            'name'      => $VALUES{TARGET_LIB},
            'old_name'  => $VALUES{TARGET_LIB},
            'is_exe'    => 0,
            'srcs'      => $SOURCES
		};
	}

    # Now all the PROGRAMs
	foreach $_ ( @program_list ){
		/PROGRAM(\d*)/ or die "Doesn't match the pattern!";
		local($num) = $1; #$num = 1 unless $num;
		local($old_name);

        if($VALUES{$_}){                    # if the target is defined
			#print "Exe : $VALUES{$_} \n";
			local($MAIN_OBJ_NAME) = "MAIN${num}_OBJ";

            # make sure that we have the appropriate MAINx_OBJ defined
			if(!$VALUES{$MAIN_OBJ_NAME} && (length(keys %MODULES)>1)){
				die "Target $VALUES{$_} is defined, but $MAIN_OBJ_NAME is not, invalid makefile";
			}

			$VALUES{$_} =~ s/\s+//g;
			$old_name = $VALUES{$_}; $old_name or die "Name undefined";
            # Make sure that the target name is unique. Library is processed first, so it get a
            # priority. Executables with the same name will have $APP_POSTFIX appended to it.
			if(exists $MODULES{$VALUES{$_}}){
				if(!$quiet){
					print "Value of target $_, $VALUES{$_}, conflicts with another module, \n";
					print "using ${VALUES{$_}}$APP_POSTFIX as the output name instead \n";
				}
				$VALUES{$_} .= $APP_POSTFIX;
                die "Target name $VALUES{$_} is still not unique" if exists $MODULES{$VALUES{$_}};
			}
			$VALUES{$_} or die;

            local($OBJS) = $VALUES{OBJ_FILES};
            $OBJS .= $VALUES{$MAIN_OBJ_NAME};           # append additional obj files for this target
            local($SOURCES) = $OBJS;

            # TODO: this assumes that the source extension is .cpp!!
            local($SOURCES);
            if ($OBJS = /^\s*\$\(SRCS:\.cpp=\.o\)\s*$/) { # using substitution
                $OBJS = $SOURCES;
                $OBJS =~ s/\.o/\.cpp/g
            }else{
                $SOURCES = $VALUES{SRCS};
            }

            $SOURCES .=  " $file ";                       # Note: no .def file here

			$MODULES{$VALUES{$_}} = {
                'name'      => $VALUES{$_},
                'old_name'  => $old_name,
                'is_exe'    => 1,
                'srcs'      => $SOURCES
			};
		}
	}
}

#
# Generates a project file for a module
#
# Note that it is possible to generate multiple projects from the
# same makefile. The projects correspond to different targets. Currently,
# makefile targets that genrate new projects are TARGET_LIB, PROGRAM, PROGRAM2,
# and PROGRAM3. When more than one target is used, the object files are shared.
# If there's a name conflict between a TARGET_LIB and a PROGRAMx, the program
# target receives name $PROGRAMx${APP_POSTFIX}
# It is also necessary to use different .pro files, since the set of sources may
# turn out to be different for different targets. Currently, .dep files are also
# not shared
#
sub generate_code(){
	$_ = shift;
    local($MODULE, $is_exe) = ($_->{name}, $_->{is_exe});

    #print "\$MODULE=$MODULE, \$is_exe=$is_exe \n";

	$projfile = "${MODULE}.pro";
	open PROJFILE, ">$projfile" or die "Can't open $projfile, $!";

	if(!$quiet){
		print STDERR '-' x 80;
		print STDERR $is_exe ?
				"Generating a project file for an executable program $MODULE \n" :
				"Generating a project file for a library module $MODULE \n";
	}

	local($SOURCES) = $MODULES{$MODULE}->{srcs};
	local($TARGET_NAME) = $MODULES{$MODULE}->{old_name};

	print PROJFILE<<EndOfCommon;
	TARGET 	  = $TARGET_NAME
	SOURCES   = $SOURCES
	HEADERS   = $VALUES{HEADERS}
	LIBS 	  = $LIB_string
EndOfCommon

	local($out) = "${MODULE}.dsp";

	if($is_exe){
		$cmd = "$tmake -tvcapp -o$out $common_pro $projfile";

		print PROJFILE "\tCONFIG    = warn_on debug console \n";
		print PROJFILE "\twin32:DEFINES  += SUPPORTS_STANDALONE_MAIN \n";
	}else{
		$cmd = "$tmake -t vclib -o$out $common_pro $projfile";

		print PROJFILE "\tCONFIG    = warn_on debug dll \n";
		print PROJFILE "\tDEF_FILE  = .\\Debug\\$MODULE.def \n";
	}

	print PROJFILE "\tDESTDIR = $ENV{NCIHOME} \n";
	if($VALUES{INCLDIRS}){
        print "Parsing INCLDIRS : $VALUES{INCLDIRS} \n" unless $quiet;
		foreach $_ (split(/\s+/, $VALUES{INCLDIRS})){
			unless(/^\s*$/){			# skip whitespace

				if(/^(\-I)?
					\$
					\(
					([^\)]+)
					\)
				/x)
				{
					print PROJFILE "\tINCLUDEPATH += ".'$$'."$2 \n";
				}elsif(/^\-I(.+)$/){
					print PROJFILE "\tINCLUDEPATH += $1 \n";
				}else{
					print STDERR "Strange include directory: $_ \n" unless $quiet;
				}
			}
		}
	}
	close PROJFILE;

 	unless ($ENV{TMAKEPATH}){
		local($template_loc) = $ENV{NCIHOME}.'\suif\suif2b\tools\msc_support\win32-msvc-nci';
		$ENV{TMAKEPATH} = $template_loc;
		print "Setting environment variable TMAKEPATH to the default of \n$template_loc\n" unless $quiet;
	}

    #$cmd = "\@echo %NCIHOME%; $cmd";
    print STDERR "Running tmake: $cmd ...\n" unless $quiet;
	#print "\t", int(-e "d");
	!system($cmd) or die "Couldn't run tmake, $!";
	#print "\t", int(-e "d");

	print STDERR "Done.\n" unless $quiet;

	unless($keep){
		unlink "$projfile" or die "Can't remove $projfile, $!";
	}

    unless($nodep){
        local($depfile) = "${MODULE}.dep";
        print STDERR "Creating $depfile ... \n" unless $quiet;
        open DEPFILE, ">$depfile" or die "Can't open $depfile, $!";
        print DEPFILE $LIB_string;
        close DEPFILE;
    }
	print STDERR "Done.\n" unless $quiet;

    local(@hoofs) = glob("*.hoof");
    foreach $_ (@hoofs){
        print STDERR "Expanding hoof file $_ \n" unless $quiet;
        local($basename) = $_;
        $basename =~ s/\.hoof$//i;
        system "smgn_build.bat $basename";
    }
}

sub usage(){
    print STDERR<<EOM;
    Usage: make2dsp.pl [--help] [-keep] [-nodep][-quiet] [gmake-makefile-name]

    Generate a MSVC project file(s) .dsp from a SUIF makefile

    --help                  - prints this usage information
    -keep                   - controls whether temporary .pro files are preserved
    -nodep                  - if this option is specified, .dep files with the lists
                              of libraries are not generated
    -quiet                  - controls the amount of output
    -gmake-makefile-name    - name of the makefile, Makefile by default
EOM
    exit(1);
}