#ifndef SUIFKERNEL__SUIF_WALKER
#define SUIFKERNEL__SUIF_WALKER

#include "iokernel/walker.h"
#include "suif_object.h"

#include "common/sparse_vector.h"


/** \class SuifWalker suif_walker.h suifkernel/suif_walker.h
 * A SuifWalker will only visit Suif Objects in the ownership tree
 */
class SuifWalker : public  Walker {
    protected:

  /** Do not normally override this. Some of the predefined
   * walkers override this for efficiency
   * If you do override this, make sure it selects only SuifObjects.
   * Just remember to include SuifWalker::is_visitable().
   */
        bool is_visitable(Address address,const MetaClass *_meta) const;
        ApplyStatus operator () (Address address, const MetaClass *_meta);

    public:

        SuifWalker(SuifEnv *the_env);
        virtual ~SuifWalker();

        virtual Walker::ApplyStatus operator () (SuifObject *x) = 0;

    };

#endif

