#include "common/system_specific.h"
#include "subsystem.h"

SubSystem::SubSystem( SuifEnv* s ) : _suif_env( s ) {
}

SuifEnv* SubSystem::get_suif_env() const {
  return _suif_env;
}

SubSystem::~SubSystem() {
}

SubSystem::SubSystem(const SubSystem &other) :
  _suif_env(other._suif_env)
{}
SubSystem& SubSystem::operator=(const SubSystem &other) {
  _suif_env = other._suif_env;
  return(*this);
}

