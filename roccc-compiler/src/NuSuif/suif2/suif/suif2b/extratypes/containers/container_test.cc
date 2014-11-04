
#include "container_iter.h"
#include "container_list.h"
#include "container_vector.h"
#include <stdio.h>

void print(ConstContainer<int> *c) {
    fprintf(stderr, "\n");
    for (ConstContainerIter<int> iter(c);
	 !iter.done();
	 iter.increment()) {
      int i = iter.get();
      fprintf(stderr, "%d\n", i);
    }
    fprintf(stderr, "\n");
}

void do_stuff(Container<int> *l) {
  for (unsigned i = 0; i< 10; i++) {
    l->push(i);
    print(l);
  }
  
  print(l);
  l->pop();

  print(l);

  l->clear();
  for (unsigned i = 0; i< 10; i++) {
    l->append(i);
    print(l);
  }

  l->unappend();

  print(l);
  l->insert(4, 101);
  print(l);
  l->insert(2, 101);
  print(l);
}



int main(int argc, char *argv[]) {
  ContainerList<int> l;
  do_stuff(&l);
  ContainerVector<int> vec;
  do_stuff(&vec);
}

  

