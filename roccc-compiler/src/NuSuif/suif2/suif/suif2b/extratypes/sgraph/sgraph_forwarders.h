#ifndef SGRAPH_FORWARDERS_H
#define SGRAPH_FORWARDERS_H

#include "common/machine_dependent.h"
#include "common/common_forwarders.h"

class SGraph;

typedef size_t SGraphNode;
typedef list<SGraphNode> SGraphNodeList;

class SGraphNodeIter;

class SGraphEdge;


class SGraphEmptyIter;
class SGraphBitIter;
class SGraphList;
template <class T> class NGraphBase;
template <class T> class NGraph;

#endif
