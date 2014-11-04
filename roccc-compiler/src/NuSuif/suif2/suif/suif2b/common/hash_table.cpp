// hash_table.cc
#include "system_specific.h"
#include <string.h>
#include <stdio.h>

#include <assert.h>
#include "hash_table.h"


unsigned long
HashTable::__suif_hash(const char *s)
{
    register unsigned long h = 0;
	register unsigned long m = g_nHashMod;

    for( ; *s; ++s) {
        h = (131*h + *s) % m;
    }
    return h;
}

HashTable::HashTable(unsigned nSize) :
  m_Table(0), g_nHashMod(nSize)
{
	typedef Bucket *PBucket;
	m_Table = new PBucket[nSize];
	for (size_t i = 0; i < nSize; i++) {
		m_Table[i] = 0;
	}
}

char* HashTable::AddItem(const char *pString)
{
	unsigned nHashVal = __suif_hash(pString);
	Bucket *pNewBucket;
	Bucket *pTempBucket;

	pTempBucket = m_Table[nHashVal];
	/* If nothing in this slot, add a new bucket */

	while (pTempBucket) {
            if (!strcmp((pTempBucket->pStr), pString)) {
                pTempBucket->nRefCnt += 1;
                return pTempBucket->pStr;
		}
	    pTempBucket = pTempBucket->pNext;
            }


	pNewBucket = new Bucket;	
	pNewBucket->pNext = m_Table[nHashVal];
	pNewBucket->pStr = new char[strlen(pString) + 1];
	strcpy(pNewBucket->pStr, pString);
		
	m_Table[nHashVal] = pNewBucket;
	pNewBucket->nRefCnt = 1;
	return (pNewBucket->pStr);
	}

bool HashTable::RemoveItem(const char *pString)
{
	return true;
/*	unsigned nHashVal = __suif_hash(pString);
	Bucket *pTempBucket;
	Bucket *pPrevBucket;

	pTempBucket = m_Table[nHashVal];
	pPrevBucket = NULL;
	while (pTempBucket && (strcmp(pString, pTempBucket->pStr))) {
		pPrevBucket = pTempBucket;
		pTempBucket = pTempBucket->pNext;
	}
	if (!pTempBucket) {
		return true; // A non-extant string is always removed
	}
	pTempBucket->nRefCnt -= 1;
	// If no one else is referencing the string, then we delete it.
	if (pTempBucket->nRefCnt < 1) {	
		if (pPrevBucket == NULL)
		    m_Table[nHashVal] = pTempBucket->pNext;
		else
		    pPrevBucket->pNext = pTempBucket->pNext;
		delete [] pTempBucket->pStr;
		delete pTempBucket;
	} 
	return true;*/
}

HashTable::HashTable(const HashTable&) :
  m_Table(0), g_nHashMod(0)
{
  assert(0);
}
HashTable &HashTable::operator=(const HashTable&) {
  assert(0);
  return(*this);
}
