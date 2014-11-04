# builds a workspace containing with most of useful SUIF libraries

chdir "$ENV{NCIHOME}/suif/suif2b" or die "Can't chdir to $ENV{NCIHOME}/suif/suif2b";

@build = qw
	(
		common basesuif basepasses extratypes ipanalysis tools utils
        basenaf naf_analyses dataflow cfeutils region_problems base_region
	);

@excl  = qw
	(
		irational linksuif macro_suifobj_adapter pgen string_enum
		tree_bit_vector tos walias iter_closure create_suif_hello_world
		texec oosplay
	);

$cmd = "make2dsw.pl -quiet -make_dsp=1 ".join(' ', @build)." -x ".join(' ', @excl);
print "Running $cmd... \n";

system $cmd;