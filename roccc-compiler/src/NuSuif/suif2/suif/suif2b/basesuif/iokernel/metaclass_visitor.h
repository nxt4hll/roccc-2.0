#ifndef IOKERNEL__METACLASS_VISITOR_H
#define IOKERNEL__METACLASS_VISITOR_H

#include "meta_class.h"
#include "common/suif_vector.h"

/**
 * Template for registering Objects that associate with
 * metaclasses.
 * Registration is by meta-class.
 * Lookup will find the registered entity for
 * this metaclass or the first ancestor that was registered
 * If there are no registered ancestors, the DefaultEntity
 * will be returned
 *
 * This implementation stores values by meta class id number
 * and caches lookup values
 *
 * With a proper registration mechnism to inform about 
 * new meta class names, we could register by name as well
 * and look up the metaclass
 */
template<class VisitEntity>
class MetaClassVisitor {
public:
  MetaClassVisitor() : _entries( 0 ) {
  }

  virtual ~MetaClassVisitor() {}

  virtual void registerEntity( const MetaClass* meta_class,
                               VisitEntity entity ) {
     clear_cache();
     MetaClassId id = meta_class->get_meta_class_id();
     for ( int i = _entries.size(); i<=id ; i++ ) {
       _entries.push_back( VectorEntry( 0 ) );
     }
     _entries[ id ] = VectorEntry( entity );
  }

  virtual void registerDefaultEntity( VisitEntity entity ) {
    clear_cache();
    if ( _entries.size() ) {
      _entries[0] = VectorEntry( entity );
    } else {
      _entries.push_back( VectorEntry( entity ) );
    }
  }

  virtual VisitEntity retrieveEntity( const MetaClass* mc ) {
     MetaClassId id = mc->get_meta_class_id();
    
     VisitEntity entity;
     if ( (MetaClassId)_entries.size() <= id ) {
       // the look up goes beyond the vector size
       entity = 0;
     } else {
       VectorEntry entry = _entries[ id ];
       if ( (!entry._entity) && (!entry._is_cached) ) {
         // the entry is not valid => look up inheritance link 
         MetaClassId target;
         const MetaClass* parent = mc;
	 do {
           parent = parent->get_link_meta_class();
           
           target = parent ? mc->get_meta_class_id() : 0;
           entry = _entries[ target ];
         } while ( parent && !( entry._entity || entry._is_cached ) );
         _entries[ id ]._entity = entry._entity;
         _entries[ id ]._is_cached = true;
       } 
       entity = entry._entity;
     }
     return entity;
  }

  virtual void clear_cache() {
     /* suif_vector<VectorEntry>*/ 
    typename Vector::iterator current = _entries.begin(),
                                   end = _entries.end();
     while ( current != end ) {
       if ( (*current)._is_cached ) {
          (*current)._entity = 0;
          (*current)._is_cached = false;
       }
       current++;
     }
  }
private:
  struct VectorEntry {
    VectorEntry( VisitEntity e = 0 ) : _entity( e ), _is_cached( false ) {}
    VisitEntity _entity;
    bool _is_cached;
  };
  typedef suif_vector<VectorEntry> Vector;
  suif_vector<VectorEntry> _entries;
};


#endif
