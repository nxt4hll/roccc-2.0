#include "system_specific.h"
#include "suif_malloc.h"
#include <malloc.h>
#include <stdio.h>
#include <assert.h>
// #define DEBUG_MALLOC

// Originally produced to overcome problems with windows. Don't repeatedly
// alloc and free small blocks. Resuse them instead

const size_t max_small = 256;   // number of free areas to keep 
			         //(each is sizeof(MemoryFreeArea in size))
const size_t max_to_keep = 5000; // never keep more than this of a given size

struct MemoryFreeArea {
    MemoryFreeArea *next;
    size_t total_size; // of this plus following entries in list
    };

static int magic1 = 0x18273647;
static int magic2 = 0x83748959;
static int magic3 = 0x93847589;


static MemoryFreeArea *headers [max_small];
static bool initialized = false;
static size_t shift;
static int count = 0;
static const char *filename = "unknown";
static int lineno = 0;

struct header {
#ifdef TRACK_ALLOC
    header *next;
    const char *filename;
    int line;
#endif
    size_t size;
    int magic;
    };


#ifdef TRACK_ALLOC
struct size_rec {
	int count;
	int size;
	size_rec *next;
	header *headers;
	};

void set_file_and_line(const char *file,int line) {
    filename = file;
    lineno = line;
    }

static bool Sinit = false; // avoid braindead C++ init problems
class Statistics {
    int alloc_count[max_small];
    header *headers[max_small];
    size_rec *big;
    bool ignore;
  public:
	Statistics() : big(NULL),ignore(false) {
	    for (int i =0; i < max_small;i ++) {
		alloc_count[i] = 0;
		headers[i] = 0;
		}
	    Sinit = true;
	    }
	~Statistics() {
	    long total = 0;
	    for (int i = 0; i < max_small;i ++) {
		if (alloc_count[i] > 0) {
		    printf("size %d left is %d for size %d\n",
			i * sizeof(size_t),
			alloc_count[i],
			i * sizeof(size_t)*alloc_count[i]);
		    print_lines(headers[i]);
		    total += i * sizeof(size_t)*alloc_count[i];
		    }
		}
	    size_rec *next = big;
	    while (next) {
		if (next->count > 0) {
		    printf("size %d left is %d for size %d\n",
                        next->size * sizeof(size_t),
                        next->count,
                        next->size * sizeof(size_t)*next->count);
		    print_lines(next->headers);
                    total += next->size * sizeof(size_t)*next->count;
		    }
		next = next->next;
		}
	    printf("total allocation remaining %d\n",total);
	    }

	void print_lines(header *start) {
	    header *next = start;
	    while (next) {
		if ((next->magic == magic1) && (next->line >= 0)) {
		    int count = 0;
		    header *n = next;
	 	    int lineno = next->line;
		    while (n) {
			if ((n->magic == magic1) && (n->filename == next->filename) 
				&& (n->line == lineno)) {
			    count ++;
			    n->line = -2;
			    }
			n = n->next;
			}
		    printf("%d allocated at %s:%d\n",
			count,next->filename,lineno);
		    }
		next = next->next;
		}
	    }

	void track_allocate(header *area,int index,bool is_new) {
	    if (!Sinit)
		return;
	    if (ignore)
		return;
	    area->line = lineno;
	    if (lineno == 0)
		area->filename = "unknown";
	    else
	        area->filename = filename;
	    filename = "unknown";
	    lineno = 0;
	    if (index < max_small) {
		alloc_count[index] ++;
		if (is_new) {
		    area->next = headers[index];
		    headers[index] = area;
		    }
		}
	    else {
		size_rec *next = big;
            	while (next && (next->size != index)) 
			next = next->next;
		if (!next) {
		    ignore = true;
		    next = new size_rec;
		    ignore = false;
		    next->next = big;
		    big = next;
		    next->count = 0;
		    next->size = index;
		    next->headers = 0;
		    }
		next->count ++;
		if (is_new) {
		    area->next = next->headers;
		    next->headers = area;
		    }
		}
	    }

	void track_free(header *area,int index) {
	    if (!Sinit)
		return;
	    if (index < max_small) {
		alloc_count[index] --;
		}
	    else {
		size_rec *next = big;
            	while (next && (next->size != index)) 
			next = next->next;
		if(next)next->count --;
		}
	    }

        void remove_from_free_lists(header *area,int index) {
            if (!Sinit)
                return;
	    header **last;
            if (index < max_small) {
		last = headers + index;
                alloc_count[index] --;
                }
            else {
                size_rec *next = big;
                while (next && (next->size != index))
                        next = next->next;
                if(!next)
		    return;
		next->count --;
		last = &next->headers;
                }
	    header *next = *last;
	    while (next && (next != area)) {
		last = &next->next;
		next = next->next;
		}
	    if (!next)
		return;
	    *last = next->next;
            }
 	};

Statistics stats;

#endif

#ifndef DEBUG_MALLOC

void * operator new(
        unsigned int cb,
        int nBlockUse,
        const char * szFileName,
        int nLine
        ) {
    return operator new((size_t)cb);
    }

#if 0
void check_addr(void *address,const char *message) {
    MemoryFreeArea *check = headers[8];
    while (check)check=check->next;

    if (address == interesting) {
	count ++;
	printf("%08X %s count %d\n",address,message,count);
	}
    }
#endif

void* operator new( size_t size ) {
    if (!initialized) {
	int i;
	for (i = 0;i < max_small; i++) {
	    headers[i] = 0;
	    }
	shift = 0;
	i = 1;
	while (i < sizeof(size_t)) {
	    shift ++;
	    i += i;
	    }
	initialized = true;
	}
    if (size < sizeof(MemoryFreeArea))
	size = sizeof(MemoryFreeArea);
    size = size + sizeof(size_t) - 1;
    int index = (size >> shift);
    size = index << shift;
    if (index >= max_small) {
	header *area = (header *)malloc(sizeof(header) + size);
	area->size = size;
	area->magic = magic1;
#ifdef TRACK_ALLOC
	stats.track_allocate(area,index,true);
#endif
	return (void *)(area + 1);
	}
    MemoryFreeArea *free = headers[index];
    if (!free) {
	header *area = (header *)malloc(sizeof(header) + size);
	area->size = size;
	area->magic = magic1;
#ifdef TRACK_ALLOC
	stats.track_allocate(area,index,true);
#endif
	return (void *)(area + 1);
	}
    else {
	header *area = (header *)free;
	area --;
	assert(area->magic == magic2);
#ifdef TRACK_ALLOC
	stats.track_allocate(area,index,false);
#endif
	area->magic = magic1;
	}
    headers[index] = free->next;
    return (void *)free;
    }

void operator delete( void* address ) {
    if (!address)
	return;

    header *size_addr = ((header *)address) - 1;
    size_t size = size_addr->size;
    if (size_addr->magic != magic1)
	return; // either trashed or allocated from elsewhere
    size_addr->magic = magic2;
    int index = (size >> shift);
    if (index >= max_small) {
#ifdef TRACK_ALLOC
	stats.remove_from_free_lists(size_addr,index);
#endif
	free(size_addr);
	return;
	}

    MemoryFreeArea *first_free = headers[index];
    MemoryFreeArea *this_free = (MemoryFreeArea *)address;
    if (!first_free) {
	headers[index] = this_free;
	this_free->next = 0;
	this_free->total_size = size;
#ifdef TRACK_ALLOC
        stats.track_free(size_addr,index);
#endif
	}
    else {
	int total_size = first_free->total_size + size;
	if (total_size > max_to_keep) {
#ifdef TRACK_ALLOC
            stats.remove_from_free_lists(size_addr,index);
#endif
	    free(size_addr);
	    return;
	    }
	this_free->next = first_free;
	this_free->total_size = total_size;
	headers[index] = this_free;
#ifdef TRACK_ALLOC
	stats.track_free(size_addr,index);
#endif
	}
    }

#else

struct footer {
    int footer_marker;
    };

struct debug_header {
    int header_marker1;
    footer *tail_ptr;
    size_t size;	    
    bool deleted;
    debug_header *next_header;
    int header_marker2;
    };


static debug_header * alloc_list = 0;
static int alloc_count = 0;
static int delete_count = 0;

typedef unsigned char Byte;
void check_for_corruption() {
    debug_header *next = alloc_list;
    int count = 0;
    while (next) {
	assert(next->header_marker1 == magic1);
	assert((Byte *)next->tail_ptr == (((Byte *)next) + sizeof(debug_header) + next->size));
	assert(next->size > 0);
	assert(next->header_marker2 == magic2);
	assert(next->tail_ptr->footer_marker == magic3);
	next = next->next_header;
	count ++;
	assert(count <= alloc_count);
	}
    assert(count == alloc_count);
    }

// an operator new caching allocations between 1-SIZE_STEP*MAX_SIZE
void* operator new( size_t size ) {
    // if((alloc_count & 0xff) == 0)check_for_corruption();
    Byte* memory = 0;     // the return value
    assert(size > 0);
    size = (size + 3) & 0xfffffffc;
    // try to reuse deleted memory
    debug_header *next = alloc_list;
    while (next) {
	if ((next->size == size) && (next->deleted))
	    break;
	next = next->next_header;
	}
    if (next) {
	memory = (Byte *)next;
	next->deleted = false;
	}
    else {
	memory = (Byte*)malloc( size + sizeof(debug_header) + sizeof(footer) );
	debug_header *head = (debug_header *)memory;
	footer *foot = (footer *)(memory + sizeof(debug_header) + size);
	head->header_marker1 = magic1;
	head->tail_ptr = foot;
	head->size = size;
	head->deleted = false;
	head->next_header = alloc_list;
	alloc_list = head;
	head->header_marker2 = magic2;
	foot->footer_marker = magic3;
	alloc_count ++;
	}
   void *x = (void *) (memory + sizeof(debug_header));
   if ((int) x == 0x9fed52c)
        printf(" its here now\n");

   return x;
   }

void checkit(void *address){
    if((int)address != 0x9fed4fc)return;
    debug_header *head = (debug_header *)((Byte *)address - sizeof(debug_header));

    debug_header *next = alloc_list;
    while (next && next != head) {
        next = next->next_header;
        }
    assert(next);
    assert (!head->deleted);
    assert(head->header_marker1 == magic1);
    assert((Byte *)head->tail_ptr == (((Byte *)head) + sizeof(debug_header) + head->size));
    assert(head->size > 0);
    assert(head->header_marker2 == magic2);
    assert(head->tail_ptr->footer_marker == magic3);
    }

void operator delete( void* address ) {
    // if ((delete_count & 0xff) == 0)check_for_corruption();

    if (!address) return;
  
    debug_header *head = (debug_header *)((Byte *)address - sizeof(debug_header));
    if ((int)address == 0x9fed52c)
	printf(" its here now\n");
    // checkit(address);
    assert (!head->deleted);

    head->deleted = true;
    int size = head->size/sizeof(int);
    int *x = (int *)address;
    for (int i=0;i < size;i++) {
	x[i] = 0xffffffff;
	}
    delete_count ++;
    }
#endif

