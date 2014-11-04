#ifndef SUIFKERNEL__WALKING_MAPS_SUBSYSTEM_H
#define SUIFKERNEL__WALKING_MAPS_SUBSYSTEM_H

#include "suifkernel_forwarders.h"

/**
 * The WalkingMapsSubsystem allows a user to 
 * register a WalkingMap by name so that modules may
 * share a walking map.
 */

class WalkingMapsSubsystem {
public:
  WalkingMapsSubsystem( SuifEnv* _suif_env );
  virtual ~WalkingMapsSubsystem();

  virtual bool is_available( const LString& walking_maps_name );
  virtual void register_walking_maps( WalkingMaps* walking_maps );
  virtual WalkingMaps* retrieve_walking_maps( const LString& walking_maps_name );

private:
  SuifEnv* _suif_env;
  suif_map<LString,WalkingMaps*>* listOfRegisteredWalkingMaps;
  WalkingMapsSubsystem(const WalkingMapsSubsystem &);
  WalkingMapsSubsystem& operator=(const WalkingMapsSubsystem &);

};



#endif
