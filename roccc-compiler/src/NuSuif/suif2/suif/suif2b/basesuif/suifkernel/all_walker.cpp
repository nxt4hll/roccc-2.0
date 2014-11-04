#include "common/system_specific.h"
#include "all_walker.h"

AllWalker::AllWalker(SuifEnv *env) :
  SuifWalker(env),
  _visited(new suif_hash_map<Address, Address>())
{
}


AllWalker::~AllWalker(void)
{
  delete _visited;
}


bool AllWalker::is_walkable(Address address,
			    bool is_owned,
			    const MetaClass *_meta)
{
  if (_visited->find(address) != _visited->end())
    return false;
  _visited->enter_value(address, address);
  return true;
}
