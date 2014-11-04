#include "common/system_specific.h"
#include "walking_maps_subsystem.h"

#include "walking_maps.h"

#include "common/suif_map.h"

#include <assert.h>

WalkingMapsSubsystem::WalkingMapsSubsystem( SuifEnv* suif_env ) :
  _suif_env(suif_env),
  listOfRegisteredWalkingMaps(new suif_map<LString,WalkingMaps*>)
{}


WalkingMapsSubsystem::~WalkingMapsSubsystem() {
  delete listOfRegisteredWalkingMaps;
}

bool WalkingMapsSubsystem::is_available( const LString& walking_maps_name ) {
  WalkingMaps* walking_maps = retrieve_walking_maps( walking_maps_name );
  return ( walking_maps != 0 );
}

void WalkingMapsSubsystem::register_walking_maps( WalkingMaps* walking_maps ) {
  assert( walking_maps );
  const LString& walking_maps_name = walking_maps->get_walking_maps_name();
  if ( retrieve_walking_maps( walking_maps_name )  ) {
    delete walking_maps; // Forget it.
  } else {
    //    walking_maps->initialize();
    //    (*listOfRegisteredWalkingMaps)[ walking_maps_name ] = walking_maps;
    listOfRegisteredWalkingMaps->enter_value(walking_maps_name, walking_maps);
  }
}


WalkingMaps* WalkingMapsSubsystem::retrieve_walking_maps( const LString& walking_maps_name ) {
  WalkingMaps* walking_maps = 0;
  suif_map<LString,WalkingMaps*>::iterator it =
      listOfRegisteredWalkingMaps->find( walking_maps_name );
  if ( it != listOfRegisteredWalkingMaps->end() ) {
    walking_maps = (*it).second;
  }
  return walking_maps;
}

WalkingMapsSubsystem::WalkingMapsSubsystem(const WalkingMapsSubsystem &other) :
  _suif_env(other._suif_env),
  listOfRegisteredWalkingMaps(0)
{
  suif_assert(false);
}
WalkingMapsSubsystem& WalkingMapsSubsystem::operator=(const WalkingMapsSubsystem &other) {
  _suif_env = other._suif_env;
  suif_assert(false);
  return(*this);
}







