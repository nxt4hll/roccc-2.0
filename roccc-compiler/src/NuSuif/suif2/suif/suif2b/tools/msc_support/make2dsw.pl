# Recursively create a workspace from makefiles inside the current directory
use File::Find;
use File::DosGlob 'glob';
use Cwd;
use File::Basename;

################################### Defaults ##################################
# Generate .dsp files. If not, then just a workspace file with dependencies gets regenerated
$make_dsp = 0;
# Output a workspace file. If not, you can just use the script
# to (re-)create a bunch of .dsp files
$make_dsw = 1;
# Verbosensess
$quiet = 0;
# Controls whether 1) .dep files are kept around 2) passed into
# make2dsp.pl to control whether .pro files are preserved
$keep = 0;
$out = basename cwd().'.dsw';

############################ Process parameters ##############################
local($excl) = 0;

foreach $_ (@ARGV){
	s/\s+//g;

	if( /^\-(.+)/ ){
		#print "Processing command line switch: $_ \n" unless $quiet;

		if(/^\-(.+)\s*=\s*(.+)/){
			$option = $1;

            if($option eq 'make_dsp') {$make_dsp = int($2);}
            if($option eq 'make_dsw') {$make_dsw = int($2);}
            if($option eq 'out'){$out = $2;}
            else{
                &usage();
            }
		}else{
			$option = $1;

            if($option eq 'quiet') {$quiet = 1;}
            if($option eq 'keep') {$keep = 1;}
            if($option eq 'x') {$excl = 1;}
            else{
                &usage();
            }# if $option eq '-help' or $option eq 'help';;
		}
    }elsif(!$excl){                     # push on to_build list
		push @dirs, $_;
    }else{                              # push on the excl_dirs list
		$excl_dirs{$_} = 1;
	}
}

unless($quiet){
	print "Running with make_dsp=$make_dsp, make_dsw=$make_dsw, keep=$keep, quiet=$quiet, out=$out \n";

	if($excl){
		print "Excluding: ";
		foreach $_ (keys %excl_dirs){print "$_ ";}
		print "\n";
	}
}

push @dirs, '.' unless @dirs;                   # by default, start in the common directory

if($make_dsw){
	open DSW, ">$out" or die "Can't open $out, $!";
	print DSW<<EndOfHeader;
Microsoft Developer Studio Workspace File, Format Version 6.00
# WARNING: DO NOT EDIT OR DELETE THIS WORKSPACE FILE!

EndOfHeader
}

find( {wanted => \&process, no_chdir => 0}, @dirs );

if($make_dsw){
	print DSW<<EndOfFooter;
###############################################################################

Global:

Package=<5>
{{{
}}}

Package=<3>
{{{
}}}

###############################################################################

EndOfFooter

	close DSW;
}

sub process(){
	return unless $_ eq 'Makefile';
	local($base_dir) = basename cwd();
	return if $excl_dirs{$base_dir};				# skip if in one of these directories
	if( &produces_a_target($File::Find::dir) ){
		#print "$File::Find::name + \n";

		if($make_dsp){
			$cmd = "make2dsp.pl Makefile";
			$cmd .= " -quiet " if $quiet;
			$cmd .= " -keep " if $keep;
			print "Running 'make2dsp.pl Makefile' in $File::Find::dir \n" unless $quiet;
			system $cmd;
		}

		if($make_dsw){
			$dos_dir = ${File::Find::dir};
			$dos_dir =~ s#/#\\#g;
			$depfile = "$base_dir.dep";

			print STDERR "Trying to open $depfile \n" unless $quiet;

			unless(open DEPFILE, $depfile){
				print STDERR "Can't open $depfile, $!\n" unless $quiet;
				return;
			}

			$_ = <DEPFILE>;
			close DEPFILE;
			unless($keep){
				unlink $depfile or warn "Cound't erase $depfile, $!";
			}
			@LIBS = split /\s+/, $_;
			#print "Libraries  $_ \n";
			print STDERR "Done reading dependencies \n" unless $quiet;

print DSW<<EndOfProject1;
###############################################################################

Project: "$base_dir"=$dos_dir\\$base_dir.dsp - Package Owner=<4>

Package=<5>
{{{
}}}

Package=<4>
{{{
EndOfProject1

foreach $lib (@LIBS){
	$lib = $1 if $lib =~ /(.+)\.lib$/;
	print DSW<<EndOfProject2;
    Begin Project Dependency
    Project_Dep_Name $lib
    End Project Dependency
EndOfProject2
}

print DSW<<EndOfProject3;
}}}

EndOfProject3
		}
		print "$base_dir \n" if $quiet;
	}else{
		#print "$File::Find::name - \n";
	}
}

sub produces_a_target(){
	local($dir) = shift;
	local($pat) = $dir.'/*.*';
	local($base_dir) = basename $dir;

	local(@dirlist) = grep {
			(!($_ =~ /cvs$/i)) &&
			-d "$_"}
			glob "*.*";

	foreach $subdir (@dirlist){
		#print "Considering subdirectory $subdir \n";
		#print "Trying to find $subdir/Makefile\n";
		if( -e "$subdir/Makefile" ){
			#print "Failing on ${dir}/$subdir/Makefile \n";
			return 0;
		}
	}
	return 1;
}

sub usage(){
    print<<EOM;
    Usage: make2dsw.pl [--help] [-make-dsw] [-make-dsp]
                [-quiet] [-keep] [-out=<filename.dsw>]
                <directories to include> [-x <directories to exclude>]

   Generates MSVC workspace and/or project files for multiple directories.

          --help        - displays this usage guide
          -make-dsw     - controls whether a workspace file gets created
                          (default: yes)
          -make-dsp     - controls whether .dsp files get (re-) generated
                          (default: no)
          -quiet        - controls the amount of output printed to the screen,
                          this option is passed to make2dsp.pl (default: no)
          -keep         - controls whether dependency (.dep) files and tmake
                          .pro files get preserved, this option is passed to
                          make2dsp.pl (default: no)
   If the list of directories is not specified, then the current directory
   is used.
EOM
    exit(1);
}