#ifndef _COMMON_FORWARDERS_H_
#define _COMMON_FORWARDERS_H_

#if defined(USE_STL) || defined(USE_STL_VECTOR)
#include <vector>
#define suif_vector vector
#else
template <class T> class suif_vector;
#endif

#if defined(USE_STL) || defined(USE_STL_LIST)
#include <list>
#else
template <class T> class list;
#endif

template <class domain,class range> class suif_hash_map;
template <class domain,class range> class suif_map;

class IInteger;
template <class T> class searchable_list;

#endif /* _COMMON_FORWARDERS_H_ */
