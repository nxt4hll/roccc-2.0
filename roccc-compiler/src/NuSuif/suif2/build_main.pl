#!/usr/local/bin/perl
#
#
#

if (!$#ARGV == 0) {
    print STDERR "Usage: $0 'libs'\n";
    exit 1;
}

#my($args) = $ARGV[0];
my($libline) = $ARGV[0];
#"-lsuifkernel -liokernel -lcommon ".
#    "-lion -ltos -ldflowtypes";




my(%ignore_libs) = ("iokernel" => 1,
		    "tos" => 1,
		    "suifkernel" => 1,
		    "smeg" => 1,
		    "common" => 1,
		    "suif1" => 1,
		    "check" => 1,
		    "useful" => 1,
		    "stdc++" => 1,
		    "m" => 1,
		    "gcc" => 1,
		    "dl" => 1,
		    "c" => 1);

my(@libs_with_l) = split(/\s+/,$libline);
foreach $name (@libs_with_l) {
    my($new_name) = $name;
    if ($new_name !~ /^\-l/) { next; }
    $new_name =~ s/^\-l// ;
    if (!defined($ignore_libs{$new_name})) {
	push(@libs, $new_name);
    }
}

#my(@libs) = s/^\-l// @libs_with_l;

print "// This file was automatically build by\n";
print "// $0 '$libline'\n";
print "// from the \$\(LIBS\} variable in the Makefile\n";
print "// \n";
print << 'EOF';
#include "suifkernel/suifkernel_forwarders.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/token_stream.h"
#include "suifpasses/suifpasses.h"

EOF


if ($do_mem_state) {

    print << 'EOF';
#define MEMORY_STATISTICS
#ifdef MEMORY_STATISTICS
#include "mem_check.cpp"
#endif
EOF

}


print << 'EOF';

int main( int argc, char* argv[] ) {
  // initialize the environment
  SuifEnv* suif = new SuifEnv;
  suif->init();

  // import and initialize the necessary libraries
EOF

  foreach $name (@libs) {
    print "  suif->require_module(\"".$name."\");\n";
  }

  print << 'EOF';

  // transform the input arguments into a stream of
  // input tokens
  TokenStream token_stream( argc, argv );

  // execute the Module "execute"
  ModuleSubSystem* mSubSystem = suif->get_module_subsystem();
  mSubSystem -> execute( "execute", &token_stream );

  delete suif;

EOF


if ($do_mem_stats) {
    print << 'EOF';
#ifdef MEMORY_STATISTICS
  print_memory_statistics();
#endif
EOF
}
    
print << 'EOF';
  return 0;
}
EOF

