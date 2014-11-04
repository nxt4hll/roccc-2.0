#ifndef OOSPLAY_H
#define OOSPLAY_H

//#include <gc_cpp.h>
#include <iter_closure/iter_closure.h>
#include <stdlib.h>
#include <assert.h>

class secondary_key/*: public gc*/ {
public:
  secondary_key() {};
  virtual int equal_to(secondary_key *to) { return 1; }
  virtual ~secondary_key(){}
};

template<class sec_type> 
class nonempty_secondary_key: public secondary_key {
private:
  sec_type k;
public:
  nonempty_secondary_key(sec_type _k) { k = _k; }
  virtual int equal_to(secondary_key *to) {
    return k == ((nonempty_secondary_key<sec_type>*)to)->k;
  }
  sec_type get_key() { return k; }
};

/* A simple node in the tree: just has a key (so a "set")  */
template <class key1_t>
class oosplay_node/*: public gc*/ {
private:
public:
  key1_t key1;
  oosplay_node() { left = right = NULL; }
  oosplay_node<key1_t> *left, *right;
  oosplay_node(key1_t k)		{ key1 = k; left = right = NULL;}
  key1_t get_key1()			{ return key1; }
  virtual secondary_key *get_key2()	{ return NULL; }
  virtual oosplay_node<key1_t> *get_chain() { return NULL; }
  virtual void set_chain(oosplay_node<key1_t> *to) { abort(); }
  oosplay_node<key1_t> *copy() {
    oosplay_node<key1_t> *retval = new oosplay_node<key1_t>;
    retval->key1 = key1;
    retval->left = left ? left->copy() : NULL;
    retval->right = right ? right->copy() : NULL;
    return retval;
  }
};


/* A node that has a key and an element: a simple map */
template<class key1_t, class elt_t>
class oosplay_node_elt/*: public gc*/ {
private:
  elt_t elt;
public:
  key1_t key1;
  oosplay_node_elt() { left = right = NULL; }
  oosplay_node_elt(key1_t k, elt_t e) {
    key1 = k;
    elt = e;
    left = right = NULL; 
  }
  elt_t get_elt() { return elt; }
  elt_t *get_elt_addr() { return &elt; }
  void set_elt(elt_t to) { elt = to; }

  oosplay_node_elt<key1_t, elt_t> *left, *right;
  key1_t get_key1()			{ return key1; }
  virtual secondary_key *get_key2()	{ return NULL; }
  virtual oosplay_node_elt<key1_t, elt_t> *get_chain() { return NULL; }
  virtual void set_chain(oosplay_node_elt<key1_t, elt_t> *to) { abort(); }
  oosplay_node_elt<key1_t, elt_t> *copy() {
    oosplay_node_elt<key1_t, elt_t> *retval = 
      new oosplay_node_elt<key1_t, elt_t>;
    retval->key1 = key1;
    retval->left = left ? left->copy() : NULL;
    retval->right = right ? right->copy() : NULL;
    retval->elt = elt;
    return retval;
  }
};

/* A node that has a primary and a secondary key */
template<class key1_t, class key2_t>
class oosplay_node_2key/*: public gc*/ {
protected:
  key2_t *key2;
private:
  oosplay_node_2key<key1_t, key2_t> *chain;
public:
  key1_t key1;
  oosplay_node_2key() { chain = NULL; left = right = NULL; }
  oosplay_node_2key(key1_t k1, key2_t *k2) {
    chain = NULL;
    key1 = k1;
    key2 = k2;
    left = right = NULL; 
  }
  virtual key2_t *get_key2() {
    return key2;
  }
  oosplay_node_2key<key1_t, key2_t> *left, *right;
  key1_t get_key1()			{ return key1; }
  virtual oosplay_node_2key<key1_t, key2_t> *get_chain() { return chain; }
  virtual void set_chain(oosplay_node_2key<key1_t, key2_t> *to) { 
    chain = to;
  }
  oosplay_node_2key<key1_t, key2_t> *copy() {
    oosplay_node_2key<key1_t, key2_t> *retval =
      new oosplay_node_2key<key1_t, key2_t>;
    retval->key1 = key1;
    retval->key2 = key2;
    retval->left = left ? left->copy() : NULL;
    retval->right = right ? right->copy() : NULL;
    retval->chain = chain ? chain->copy() : NULL;
    return retval;    
  }
};

/* A node that has a primary key, secondary key, and an element */
template<class key1_t, class key2_t, class elt_t>
class oosplay_node_2key_elt: public oosplay_node_2key<key1_t, key2_t> {
private:
  elt_t elt;
  oosplay_node_2key_elt<key1_t, key2_t, elt_t> *chain;
public:
  oosplay_node_2key_elt() {left = right = NULL; }
  oosplay_node_2key_elt(key1_t k1, key2_t *k2, elt_t e) {
    oosplay_node_2key<key1_t, key2_t>::key1 = k1; 
    oosplay_node_2key<key1_t, key2_t>::key2 = k2;
    elt = e;
    left = right = NULL; 
  }
  
  key2_t *get_key2() {
    return oosplay_node_2key<key1_t, key2_t>::key2;
  }
  elt_t get_elt() { return elt; }
  void set_elt(elt_t to) { elt = to; }
  oosplay_node_2key_elt<key1_t, key2_t, elt_t> *left, *right;
  key1_t get_key1() { return oosplay_node_2key<key1_t, key2_t>::key1; }
  virtual oosplay_node_2key_elt<key1_t, key2_t, elt_t> *get_chain() { 
    return chain; 
  }
  virtual void set_chain(oosplay_node_2key_elt<key1_t, key2_t, elt_t> *to) { 
    chain = to;
  }
  oosplay_node_2key_elt<key1_t, key2_t, elt_t> *copy() {
    oosplay_node_2key_elt<key1_t, key2_t, elt_t> *retval = 
      new oosplay_node_2key_elt<key1_t, key2_t, elt_t>;
    retval->key1 = oosplay_node_2key<key1_t, key2_t>::key1;
    retval->key2 = oosplay_node_2key<key1_t, key2_t>::key2;
    retval->elt = elt;
    retval->left = left ? left->copy() : NULL;
    retval->right = right ? right->copy() : NULL;
    retval->chain = chain ? chain->copy() : NULL;
    return retval;    
  }
};

template<class key1_t, class node_t> class oosplay_tree /*: public gc*/{
private:
  node_t *_root;
  node_t *rotate_left(node_t *n);
  node_t *rotate_right(node_t *n);

  node_t *link_left(node_t *node, node_t *left);
  node_t *link_right(node_t *node, node_t *right);
  node_t *assemble (node_t *node, node_t *left, node_t *right);
  node_t *td_splay (node_t *root, key1_t x);
  int in_iterate;
public:
  oosplay_tree() { _root = NULL; in_iterate = 0; }
  /* Delete a node with keys matching key_node */
  node_t *delete_node (node_t &key_node);
  /* Extract a node with key1 >= x */
  node_t *extract_node (key1_t x);
  /* Return a node whose key is >= x */
  node_t *contains_node (key1_t x);
  void insert_node(node_t *node);
  /* Returns TRUE if there is a node whose keys match key_node */
  int lookup(node_t &key_node, node_t **found);
  /* Find one representative for key1 = x1.  Return 1 if such a node is found */
  int lookup(key1_t x1, node_t **found);
  /* Same as lookup but reorganizes the tree too */
  int lookup_reorg(key1_t x1, node_t **found);
  int tree_size (void);
  void iterate(void (*f) (node_t *node));
  void iterate(IterClosure<node_t> *f);
  /* Copy the tree pointed to by from into this */
  void copy(oosplay_tree<key1_t, node_t> *from);
  int is_empty() { return _root == NULL; }
  void erase() { _root = NULL; }
};

/* A handy class if one wants a more elaborate secondary key */
template<class t1, class t2>
class oopair {
public:
  t1 _op1;
  t2 _op2;
  oopair() { }
  oopair(t1 op1, t2 op2) { _op1 = op1; _op2 = op2; }
  int operator==(const oopair<t1, t2> &x) {
    return _op1 == x._op1 && _op2 == x._op2;
  }
};

/* The stuff really belongs in a .cpp or .tcc file */
/*
 * A top-down splay tree implementation by 
 *	Amer Diwan
 * For details and analysis of the algorithms, please refer to
 *	D.D. Sleator and R.E. Tarjan,
 *	"Self-Adjusting Binary Search Trees",
 *	JACM 32(3), July 1985, 652-686.
 */

/***********************************************************************/


/***********************************************************************/
/* PURPOSE: rotate_right takes the left edge of node and rotates it.
 * rotate_left is symmetrically defined.
 * e.g. of rotate_left:
 * 
 *              R              B
 *             / \            / \
 *            A   B  =>      R   D
 *               / \        / \
 *              C   D      A   C
 */

template<class key1_t, class node_t>
node_t *
oosplay_tree<key1_t, node_t>::rotate_right (node_t *node)
{
  node_t *p = node->left;
  node->left = p->right;
  p->right = node;
  return p;
}

template<class key1_t, class node_t>
node_t *
oosplay_tree<key1_t, node_t>::rotate_left (node_t *node)
{
  node_t *p = node->right;
  node->right = p->left;
  p->left = node;
  return p;
}

/***********************************************************************/
/*
 * PURPOSE: link_left takes n and its left subtree and attaches it to l.  
 *          link_right is symmetrically defined.
 * PARAMETERS: node, a pointer to the root of the tree to be split.
 *             left (right similarly defined): pointer to a node.  All nodes in
 *             the left subtree are less than any node in the tree rooted at n.
 *             The right subtree is a pointer to the largest node in the l
 *             subtree.  This is needed as this is the place the linking
 *             occurs.  Note that this is an unusual use of the node (since now
 *             we have a dag!).  This part will need to be modified if an
 *             array representation of the splay tree is used.
 * an example of link_left:
 *
 *         L          N                      L         E
 *        / \        / \       =>           / \
 *       A  |       D   E                  A   \
 *      / \ |                             / \  |
 *     B   C                             B   C |
 *                                            \|
 *                                             N
 *                                            /
 *                                           D
 */

template<class key1_t, class node_t>
node_t *
oosplay_tree<key1_t, node_t>::link_left(node_t *node, node_t *left)
{
  node_t *temp = node->right;
  node->right = NULL;
  if (left->left) {
    left->right->right = node;
    left->right = node;
  }
  else
    left->left = left->right = node;
  return temp;
}

template<class key1_t, class node_t>
node_t *
oosplay_tree<key1_t, node_t>::link_right(node_t *node, node_t *right)
{
  node_t *temp = node->left;
  node->left = NULL;
  if (right->left) {
    right->right->left = node;
    right->right = node;
  }
  else
    right->right = right->left = node;
  return temp;
}
/***********************************************************************/
/*
 * PURPOSE: Assembles a tree from node, left, right.
 *	Look at documentation for link_left for a description of how 'left' and
 *	'right' are used.  This routine special cases the case in which a node
 *	being searched hasn't been found--in this case, it makes the node just
 *	larger than the searched node the root of the tree (which is exactly
 *	what we want for a Best Fit memory allocation package).
 */
template<class key1_t, class node_t>
node_t *
oosplay_tree<key1_t, node_t>::assemble(node_t *node, node_t *left, node_t *right)
{
  if (node) {
    if (left->left) {
      left->right->right = node->left;
      node->left = left->left;
    }
    if (right->left) {
      right->right->left = node->right;
      node->right = right->left;
    }
    return node;
  }
  else { /* node not found */
    if (right->left) {
      right->left = td_splay (right->left, right->right->key1);
      right->right->left = left->left;
      return right->left;
    }
    return left->left;
  }
}

/***********************************************************************/
/*
 * PURPOSE: This is the routine that actually does the splay.  ROOT is a 
 *          pointer to the root of the tree being splayed, and KEY is the 
 *          item being searched.  The splay essentially searches down the
 *          tree for the value and, as it descends the tree, moves
 *          most of the nodes it touches towards the root.  Eventually, KEY
 *          ends up as the root of the tree.  If KEY is not found, then 
 *          the node just greater than KEY becomes the root.
 *          For further details of the algorithm, please refer to the reference
 *          given in the header to this file.
 */

template<class key1_t, class node_t>
node_t *
oosplay_tree<key1_t, node_t>::td_splay(node_t *root, key1_t x)
{
  node_t left, right;
  left.left = left.right = right.left = right.right = NULL;
  
  assert(!in_iterate);
#ifdef DEBUG
  check_tree ();
#endif

  if (root == NULL)
    return NULL;
  do {
    if (root->key1 > x) {
      if (root->left) {
	if (root->left->key1 == x) 
	  root = link_right (root, &right);
	else if (root->left->key1 > x)
	  root = link_right (rotate_right (root), &right);
	else root = link_left (link_right (root, &right), &left);
      }
      else break;
    }
    else if (root->key1 < x) {
      if (root->right) {
	if (root->right->key1 == x)
	  root = link_left (root, &left);
	else if (root->right->key1 > x)
	  root = link_right (link_left (root, &left), &right);
	else
	  root = link_left (rotate_left (root), &left);
      }
      else {
	/* At this point, the node we desire is greater than the root
	   but less than any in RIGHT.  Moreover, the root does
	   not have a right subtree so this means that the node we need
	   is not any that is in the tree rooted at "root".   So we just
	   link-left which causes root to become NULL and the tree pointed
	   to by root to be linked to LEFT.  Thus assemble will
	   make root the smallest node in RIGHT, which is exactly what we
	   want. */
	root = link_left (root, &left);
	break;
      }
    }
  } while (root && root->key1 != x);
  root = assemble (root, &left, &right);

#ifdef DEBUG
  check_tree ();
#endif

  return root;
}

/***********************************************************************/
/*
 * PURPOSE: Inserts NODE in the tree rooted at ROOT_PTR, returning tree root
 *          after insertion.
 * ALGORITHM: First it does a top-down splay to get the node whose primary key 
 *            is equal to (or just greater than) that of NODE.  The rest of 
 *            the procedure makes NODE the root, setting its left and right 
 *            child depending on whether the current root's primary key is 
 *            greater than or smaller than NODE's.
 */

template<class key1_t, class node_t>
void
oosplay_tree<key1_t, node_t>::insert_node(node_t *node)
{
#ifdef DEBUG
  int size;
  check_tree ();
  size = tree_size ();
#endif
  if (_root) {
    _root = td_splay (_root, node->key1);
    if (_root->key1 < node->key1) {
      /* td_splay is guaranteed to return a tree such that its root is 
         either the size of node->key1 or just larger.  This doesn't happen
	 only when the tree does NOT have ANYTHING that satisfies this
	 criteria.  So everything in the tree is SMALLER than node. */
      node->left = _root;
      node->right = NULL;
    }
    else if (_root->key1 > node->key1) {
      node->right = _root;
      node->left = _root->left;
      _root->left = NULL;
    }
    else { /* they are equal--just chain it on */
      node->left = _root->left;
      node->right = _root->right;
      _root->right = _root->left = NULL;
      node->set_chain(_root);
    }
  }
  _root = node;
#ifdef DEBUG
  if (tree_size() != size + (long)node->key1)
    abort ();
  node->check_tree();
#endif
}

/***********************************************************************/
/*
 * PURPOSE: Deletes a node from the tree rooted at ROOT that has X1 as the 
 *          primary key and X2 as the secondary key.  It assumes that such
 *          a node exists.
 * ALGORITHM: First a splay is done to get the node with X1 as the primary
 *            key to the root.   There are two cases to consider: (1) the
 *            root is the node to be deleted, and (2) the node to be deleted
 *            is chained to the root.  The (2) case is trivial.  In (1),
 *            if the root has a chain, then this is easy as well--pick a node
 *            from the chain, and make the root instead.  If the root does
 *            not have a chain, then I need to join the left and the right
 *            subtrees.  First I check if either the left or the right
 *            subtrees are NULL--if this is the case, then the joining is 
 *            trivial.  Otherwise, I need to splay the right subtree with the 
 *            key from the root of the left subtree.  This guarantees that
 *            the root of the right subtree has NULL as its left child, and 
 *            thus, I can join the two subtrees.
 */
template<class key1_t, class node_t>
node_t *
oosplay_tree<key1_t, node_t>::delete_node (node_t &key_node)
{
  node_t *targ, *prev;
#ifdef DEBUG
  int size;
  check_tree (root);
  size = tree_size (root);
#endif
  _root = td_splay (_root, key_node.key1);
  for (targ = _root, prev = NULL;
       targ && !(targ->get_key2()->equal_to(key_node.get_key2()));
       prev = targ, targ = targ->get_chain());
  if (targ) {
    if (targ->key1 != key_node.key1)
      targ = NULL;
    else if (targ == _root) {
      if (targ->get_chain()) {
	_root = targ->get_chain();
	_root->left = targ->left;
	_root->right = targ->right;
      }
      else {
	if (targ->right == NULL)
	  _root = targ->left;
	else if (targ->left == NULL)
	  _root = targ->right;
	else {
	  _root = td_splay (targ->right, targ->left->key1);
	  _root->left = targ->left;
	}
      }
    } else /* targ is not root */
      prev->set_chain(targ->get_chain());

#ifdef DEBUG
    if (tree_size(_root) != size - key_node.key1)
      abort ();
#endif
  }

#ifdef DEBUG
  check_tree (_root);
#endif
  return targ;
}

/***********************************************************************/
/*
 * PURPOSE: This finds a node with primary key equal to X or simply
 *          greater than it.
 * ALGORITHM: This is almost identical to delete--refer to the delete routine
 *            documentation for greater details.
 */
template<class key1_t, class node_t>
node_t *
oosplay_tree<key1_t, node_t>::extract_node (key1_t x)
{
  node_t *targ;
#ifdef DEBUG
  int size;
  check_tree (_root);
  size = tree_size (_root);
#endif
  if (_root == NULL)
    return NULL;
  _root = td_splay (_root, x);
  if (_root->key1 >= x) {
    if (_root->get_chain()) {
      /* grab next in chain to avoid some work */
      targ = _root->get_chain();
      _root->set_chain(targ->get_chain());
    }
    else {
      targ = _root;
      if (targ->right == NULL)
	_root = targ->left;
      else if (targ->left == NULL)
	_root = targ->right;
      else {
	_root = td_splay (targ->right, targ->left->key1);
	_root->left = targ->left;
      }
    }
  } else {
    targ = NULL;
  }

#ifdef DEBUG
  if (targ != NULL && tree_size(*root_ptr) != size - targ->key1)
    abort ();
  check_tree (_root);
#endif
  return targ;
}

/*
 * contains_node returns the node whose key is equal to KEY or just greater.
 */
template<class key1_t, class node_t>
node_t *
oosplay_tree<key1_t, node_t>::contains_node (key1_t x)
{
  node_t *node;

#ifdef DEBUG
  check_tree ();
#endif

  if (_root == NULL)
    return NULL;
  _root = td_splay (_root, x);
  if (_root->key1 >= x)
    node = _root;
  else {
    node = NULL;
  }

#ifdef DEBUG
  check_tree ();
#endif

  return node;
}

template<class key1_t, class node_t>
int
oosplay_tree<key1_t, node_t>::lookup (node_t &key_node, node_t **found) {
  node_t *cur = _root;
  key1_t x1 = key_node.get_key1();
  secondary_key *x2 = key_node.get_key2();
  while (cur != NULL && cur->key1 != x1) {
    if (cur->key1 < x1) cur = cur->right;
    else if (cur->key1 > x1) cur = cur->left;
  }
  while (cur && cur->get_key2() && x2 &&
	 !cur->get_key2()->equal_to(x2)) {
    cur = cur->get_chain();
  }
  if (cur) {
    *found = cur;
    return 1;
  }
  else {
    return 0;
  }


  node_t *l = contains_node(key_node.key1);
  if (l == NULL || l->key1 != key_node.key1) return 0;
  while (l != NULL && !(l->get_key2()->equal_to(key_node.get_key2()))) {
    l = l->get_chain();
  }
  if (l == NULL) {
    return 0;
  }
  else {
    *found = l;
    return 1;
  }
}

template<class key1_t, class node_t>
int
oosplay_tree<key1_t, node_t>::lookup_reorg (key1_t x1, node_t **found) {
  node_t *l = contains_node(x1);
  if (l == NULL || l->key1 != x1) return 0;
  else {
    *found = l;
    return 1;
  }
}

template<class key1_t, class node_t>
int
oosplay_tree<key1_t, node_t>::lookup (key1_t x1, node_t **found) {
  node_t *cur = _root;
  while (cur != NULL && cur->key1 != x1) {
    if (cur->key1 < x1) cur = cur->right;
    else if (cur->key1 > x1) cur = cur->left;
  }
  if (cur) {
    *found = cur;
    return 1;
  }
  else {
    return 0;
  }
}

template<class node_t>
int count_nodes(node_t *n) {
  return n ? (1 + count_nodes(n->left) + count_nodes(n->right)) : 0;
}

template <class node_t>
class CountClosure: public IterClosure<node_t> {
public:
  int count;
  CountClosure() { count = 0; }
  void apply(node_t *n) { ++count; }
};

template<class key1_t, class node_t>
int
oosplay_tree<key1_t, node_t>::tree_size(void) {
 CountClosure<node_t> CC;
 iterate(&CC);
 return CC.count;
 /*  return count_nodes<node_t>(_root);*/
}

template<class node_t>
void node_iterate(node_t *starting_from, void (*f)(node_t *node)) {
  f(starting_from);
  if (starting_from->get_chain()) {
    node_iterate/*<node_t>*/(starting_from->get_chain(), f);
  }
  if (starting_from->left) {
    node_iterate/*<node_t>*/(starting_from->left, f);
  }
  if (starting_from->right) {
    node_iterate/*<node_t>*/(starting_from->right, f);
  }
}

template<class key1_t, class node_t>
void oosplay_tree<key1_t, node_t>::iterate(void (*f) (node_t *node))
{
  if (_root == NULL) return;
  in_iterate = 1;
  node_iterate/*<node_t>*/(_root, f);
  in_iterate = 0;
}


template<class node_t>
void node_iterate(node_t *starting_from, IterClosure<node_t> *f) {
  if (starting_from->left) {
    node_iterate/*<node_t>*/(starting_from->left, f);
  }
  f->apply(starting_from);
  if (starting_from->get_chain()) {
    node_iterate/*<node_t>*/(starting_from->get_chain(), f);
  }
  if (starting_from->right) {
    node_iterate/*<node_t>*/(starting_from->right, f);
  }
  return;
}

template<class key1_t, class node_t>
void oosplay_tree<key1_t, node_t>::iterate(IterClosure<node_t> *f)
{
  if (_root == NULL) return;
  in_iterate = 1;
  node_iterate/*<node_t>*/(_root, f);
  in_iterate = 0;
}

template<class key1_t, class node_t>
void oosplay_tree<key1_t, node_t>::copy(oosplay_tree<key1_t, node_t> *from) {
  assert(_root == NULL);
  if (from->_root) _root = from->_root->copy();
}



#endif /* SPLAY_H */
