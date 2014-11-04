#include "system_specific.h"
#include "sparse_vector.h"


void **PointerTree::get_entry(int index)
    {
    PointerTreeNode *next;
    if (!root) {
        root = new PointerTreeNode(0,0);
	}
    int i = index >> root->shift;
    while (i > 15)
	{
	next = root;
        root = new PointerTreeNode(0,next->shift + 4);
        root->children[0] = next;
	i = index >> root->shift;
	}

    next = root;
    while (next)
        {
	int i = index >> next->shift;
	if (next->shift == 0)
	    break;

	if (next->children[i])
            {
            index = index - (i << next->shift);
            next = (PointerTreeNode *)next->children[i];
            }
        else
            {
            next->children[i] = new PointerTreeNode(next,next->shift - 4);
            index = index - (i << next->shift);
            next = (PointerTreeNode *)next->children[i];
            }
        }
    return (void **)next->children+index;
    }

PointerTree::PointerTree() : root(0) {}

PointerTree::~PointerTree() {
    delete root;
    }

PointerTreeNode::PointerTreeNode(PointerTreeNode *p,int s) : parent(p),shift(s)
    {
    for (int i = 0;i < 16;i++)
        children[i] = 0;
    }

PointerTreeNode::~PointerTreeNode() {
    for (int i = 0;i < 16;i++)
	delete (char*)(children[i]);
    }

