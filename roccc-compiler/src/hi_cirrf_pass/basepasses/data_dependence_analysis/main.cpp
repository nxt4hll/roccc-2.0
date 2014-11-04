// This file was automatically build by
// /mnt/seagate2/csuif/betul/csuif_dev/Suif2/nci/build_main.pl '-lsuifkernel -liokernel -lcommon -lutils -ltransforms -lcfenodes -lsuifnodes -lbasicnodes -lsuifpasses  -ltypebuilder'
// from the $(LIBS} variable in the Makefile
// 
#include "suifkernel/suifkernel_forwarders.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/token_stream.h"
#include "suifpasses/suifpasses.h"


int main( int argc, char* argv[] ) {
  // initialize the environment
  SuifEnv* suif = new SuifEnv;
  suif->init();

  // import and initialize the necessary libraries
  suif->require_module("utils");
  suif->require_module("transforms");
  suif->require_module("cfenodes");
  suif->require_module("suifnodes");
  suif->require_module("basicnodes");
  suif->require_module("suifpasses");
  suif->require_module("typebuilder");

  // transform the input arguments into a stream of
  // input tokens
  TokenStream token_stream( argc, argv );

  // execute the Module "execute"
  ModuleSubSystem* mSubSystem = suif->get_module_subsystem();
  mSubSystem -> execute( "execute", &token_stream );

  delete suif;

  return 0;
}
