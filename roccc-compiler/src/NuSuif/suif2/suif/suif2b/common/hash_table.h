#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_

#include "lstring.h"


typedef struct tagBucket {
	char *pStr;
	unsigned nRefCnt;
	struct tagBucket *pNext;
} Bucket;

class HashTable {
private:
	Bucket **m_Table;	
	unsigned long g_nHashMod;

	unsigned long __suif_hash(const char *s);

protected:

public:
	HashTable(unsigned nSize);

	char* AddItem(const char *pString);
	bool RemoveItem(const char *pString);
      private:
        HashTable(const HashTable&);
        HashTable &operator=(const HashTable&);
};

#endif

