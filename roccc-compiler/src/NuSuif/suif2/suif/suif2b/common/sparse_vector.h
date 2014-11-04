#ifndef __SPACE_VECTOR__H__
#define __SPACE_VECTOR__H__

/**	@file
 *	A sparse vector implementation
 */

class PointerTreeNode {
    public:
	PointerTreeNode *parent;
	int shift;
	void *children[16];
	PointerTreeNode(PointerTreeNode *p,int s);
	~PointerTreeNode();
    private:
        /* avoid default implementations, don't define these */
        PointerTreeNode &operator=(const PointerTreeNode &);
        PointerTreeNode(const PointerTreeNode &);
    };

class PointerTree {
        PointerTreeNode *root;
    public:
	void **get_entry(int index);
	PointerTree();
	~PointerTree();
    private:
        /* avoid default implementations, don't define these */
        PointerTree &operator=(const PointerTree &);
        PointerTree(const PointerTree &);
    };

/**	\class SparseVector sparse_vector.h common/sparse_vector.h
*	A tree based sparse vector implementation. 
*/
template <class PtrType> class SparseVector {
	PointerTree tree;
    public:
	PtrType &operator[](int index)
	    {
	    return *((PtrType *)tree.get_entry(index));
	    }
        SparseVector() {};
    private:
        /* avoid default implementations, don't define these */
        SparseVector &operator=(const SparseVector &);
        SparseVector(const SparseVector &);
    };

#endif
