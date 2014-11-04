#ifndef _MAP_STR_PTR_H_ 
#define _MAP_STR_PTR_H_

#include <string.h>
#include "hash_fun.h"

// This lets the mod value in the hash function be 16319 -- a prime
#define HTABLE_SIZE 16320
/*
inline unsigned long __suif_hash(const char *s);
*/
template <class T>
class MapStrPtr {
private:
	typedef struct tagBucket {
		char *pStr;
		T ptr;
		bool bEnd; // Determines whether this node is final in a bucket chain
		struct tagBucket *pNext;
	} Bucket;
protected:
	Bucket **m_Table;	

protected:
	class iterator {
	friend class MapStrPtr<T>;
	private:
		Bucket *pBucket;
	protected:

	public:
		iterator() {}
		iterator(Bucket *pB) : pBucket(pB) {}
		iterator(const iterator& it) { pBucket = it.pBucket; }
		T& operator*() {
			return pBucket->ptr;
		}
		iterator& operator=(const iterator& it) { 
			pBucket = it.pBucket; 
			return *this;
		}
		iterator& operator++() {
			if (pBucket->pNext) {
				pBucket = pBucket->pNext;
				return *this;
			}
		}
		bool operator==(const iterator& it) { return pBucket == it.pBucket; }
		bool operator!=(const iterator& it) { return pBucket != it.pBucket; }
	}; // class MapStrPtr::iterator
	iterator m_Begin;
	iterator m_End;
public:
	MapStrPtr() {
		// Allocate one extra slot for the end()
		m_Table = new Bucket*[HTABLE_SIZE + 1];
		for (int i = 0; i < HTABLE_SIZE; i++) { m_Table[i] = 0; }
		Bucket *pEnd = new Bucket;
		pEnd->pStr = 0; pEnd->pNext = 0;
		m_Table[HTABLE_SIZE] = pEnd;
		m_End.pBucket = pEnd;
		m_Begin.pBucket = pEnd;
	}

	iterator begin() {
		return m_Begin;
	}	
	iterator end() {
		return m_End;
	}
	
	void insert(char *pString, T ptr) {
		unsigned nHashVal = __suif_hash(pString);
		unsigned nHV = nHashVal;
		Bucket *pNewBucket;
		Bucket *pTmp, *pTmp2;
		pTmp = m_Table[nHashVal];
		/* no buckets in this slot yet */
		if (!pTmp) {
			pNewBucket = new Bucket;
			pNewBucket->pStr = new char[strlen(pString)];
			m_Table[nHashVal] = pNewBucket;
			pTmp2 = m_Table[++nHashVal];
			while (!pTmp2) { 
				pTmp2 = m_Table[++nHashVal]; 
			}
			pNewBucket->pNext = pTmp2; 
			strcpy(pNewBucket->pStr, pString);
			pNewBucket->ptr = ptr;
			pNewBucket->bEnd = true;	

			if (m_Begin == m_End ||
				nHV < __suif_hash((m_Begin.pBucket)->pStr))
			{
				m_Begin.pBucket = pNewBucket;
			}
			else {
				pTmp2 = m_Table[--nHV];
				while (!pTmp2) {
					pTmp2 = m_Table[--nHV];
				}
				while (!(pTmp2->bEnd))
					pTmp2 = pTmp2->pNext;
				pTmp2->pNext = pNewBucket;	
			}
			return;
		}
		while (pTmp) {
			if (!strcmp(pTmp->pStr, pString)) 
				return;	// duplicate found	
			else if (pTmp->bEnd) {
				pNewBucket = new Bucket;
				pNewBucket->pStr = new char[strlen(pString)];
				pNewBucket->pNext = pTmp->pNext;
				strcpy(pNewBucket->pStr, pString);
				pNewBucket->ptr = ptr;
				pNewBucket->bEnd = true;
				pTmp->bEnd = false;
				return;
			}
			pTmp = pTmp->pNext;
		}
	}

	void erase(char *pString) {
		unsigned nHashVal = __suif_hash(pString);
		unsigned nHV = nHashVal;
		Bucket *pTempBucket;
		Bucket *pPrevBucket;
		Bucket *pTmp2;
		bool bChainHead = true;

	    pTempBucket = m_Table[nHashVal];
    	pPrevBucket = pTempBucket;
    	while (pTempBucket && (strcmp(pString, pTempBucket->pStr))) {
			bChainHead = false;
        	pPrevBucket = pTempBucket;
        	pTempBucket = pTempBucket->pNext;
    	}
    	if (!pTempBucket) {
        	return; // A non-extant string is always removed
    	}
		if (pTempBucket == m_Begin.pBucket)
			m_Begin = pTempBucket->pNext;
		else if (bChainHead) {
			pTmp2 = m_Table[--nHV];	
			while (!pTmp2) 
				pTmp2 = m_Table[--nHV];
			while (!(pTmp2->bEnd)) 
				pTmp2 = pTmp2->pNext;
			pTmp2->pNext = pTempBucket->pNext;
		}
      	pPrevBucket->pNext = pTempBucket->pNext;
       	delete [] pTempBucket->pStr;
       	delete pTempBucket;
	pTempBucket  = 0;
	}

	iterator find(char *pStr) {
		unsigned nHashVal = __suif_hash(pStr);
		Bucket *pTmp = m_Table[nHashVal];
		while (pTmp && strcmp(pStr, pTmp->pStr)) {
			pTmp = pTmp->pNext;
		}
		if (!pTmp)
			return end();
		else {
			return pTmp;
		}	
	}
	T& operator[](char *pStr) {
		insert(pStr, T());
		return ((find(pStr)).pBucket)->ptr;	
	}	
}; // class MapStrPtr

/*
inline unsigned long
__suif_hash(const char *s)
{
    register unsigned long h = 0;
    register unsigned long m = HTABLE_SIZE - 1;

    for( ; *s; ++s) {
        h = (131*h + *s) % m;
    }
    return h;
}
*/
#endif

