#ifndef SUIFLINK__ALL_WALKER
#define SUIFLINK__ALL_WALKER

#include "suifkernel/suif_walker.h"
#include "common/suif_hash_map.h"


/**
 * An AllWalker will visit both referenced and owned objects
 *      It will visit each instance only once.
 */

class AllWalker : public  SuifWalker {
 private:
  suif_hash_map<Address, Address> * const _visited;
 protected:
  virtual bool is_walkable(Address address,
			   bool is_owned,
			   const MetaClass *_meta);
 public:
  AllWalker(SuifEnv *the_env);
  virtual ~AllWalker(void);
};

#endif  // SUIFLINK__ALL_WALKER


