#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn) {
  h->elemSize = elemSize;
  h->numBuckets = numBuckets;
  h->hashfn = hashfn;
  h->comparefn = comparefn;
  h->freefn = freefn;
  h->buckets = malloc( h->numBuckets * sizeof(vector) );
  for(int i=0; i<numBuckets; i++) {
    VectorNew(h->buckets+i, elemSize, freefn, 4);
  }
}

void HashSetDispose(hashset *h)
{
  for(int i=0; i<h->numBuckets; i++) {
    vector* start = (vector*)HashSetNthBucket(h, i);
    VectorDispose(start);
  }
  free(h->buckets);
}

int HashSetCount(const hashset *h)
{
  int count = 0;
  for (int i=0; i<h->numBuckets; i++)
    count += VectorLength(h->buckets + i);
  return count;
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
  assert(mapfn != NULL);
  for(int i=0; i<h->numBuckets; i++) {
    VectorMap(HashSetNthBucket(h, i), mapfn, auxData);
  }
}

vector* HashSetNthBucket(hashset *h, int idx)
{
  return h->buckets + idx;
}

void HashSetEnter(hashset *h, const void *elemAddr, const void *found)
{
  int idx = h->hashfn(elemAddr, h->numBuckets);
  assert(elemAddr != NULL);
  if(found != NULL) {
    memcpy(found, elemAddr, h->elemSize);
  } else {
    VectorAppend(HashSetNthBucket(h, idx), elemAddr);
    VectorSort(HashSetNthBucket(h, idx), h->comparefn);
  }
}

void* HashSetLookup(const hashset *h, const void *elemAddr)
{
  assert(elemAddr != NULL);
  int idx = h->hashfn(elemAddr, h->numBuckets);
  vector *v = HashSetNthBucket(h, idx);
  int pos = VectorSearch(v, elemAddr, h->comparefn, 0, true);
  return pos == -1 ? NULL : VectorNth(v, pos);
}
