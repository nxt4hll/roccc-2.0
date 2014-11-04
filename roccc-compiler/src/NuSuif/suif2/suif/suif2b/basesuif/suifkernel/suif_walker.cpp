#include "common/system_specific.h"
#include "suif_walker.h"
#include "iokernel/meta_class.h"

SuifWalker::SuifWalker(SuifEnv *the_env) : Walker(the_env) {}
SuifWalker::~SuifWalker() {}

bool SuifWalker::is_visitable(Address address,const MetaClass *_meta) const {
    while (_meta && (_meta->get_instance_name() != SuifObject::get_class_name()))
        {
        _meta = _meta->get_link_meta_class();
        }
      return (_meta!=NULL);
    }

Walker::ApplyStatus SuifWalker::operator () (Address address,
					     const MetaClass *_meta)
{
  suif_assert_message(is_kind_of_suif_object_meta_class(_meta),
		      ("Oops, not a suif object"));
  return (*this)((SuifObject *)address);
}


