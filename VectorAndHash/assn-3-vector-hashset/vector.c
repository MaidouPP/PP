#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
  v->elems = malloc( initialAllocation * elemSize );
  v->elemSize = elemSize;
  v->allocLength = initialAllocation;
  v->logLength = 0;
  v->freeFn = freeFn;
}

void VectorDispose(vector *v)
{
  if(v->freeFn != NULL) {
    for (int i=0; i<v->logLength; i++)
      v->freeFn((char*)v->elems + v->elemSize * i);
  }
  free(v->elems);
}

int VectorLength(const vector *v)
{ return v->logLength; }

void *VectorNth(const vector *v, int position)
{ return (char*)v->elems + v->elemSize * position; }

void VectorReplace(vector *v, const void *elemAddr, int position)
{
  assert(position>=0 && position<v->logLength);
  if(v->freeFn != NULL)
    v->freeFn(VectorNth(v, position));
  memcpy((char*)v->elems + v->elemSize * position, elemAddr, v->elemSize);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
  assert(position>=0 && position<=v->logLength);
  if(v->logLength == v->allocLength) {
    v->allocLength *= 2;
    v->elems = realloc(v->elems, v->allocLength);
  }
  if(position == v->logLength)
    VectorAppend(v, elemAddr);
  else {
    memmove((char*)v->elems + v->elemSize * (position + 1), (char*)v->elems + v->elemSize * position, v->logLength - position);
    memcpy((char*)v->elems + v->elemSize * position, elemAddr, v->elemSize);
    v->logLength++;
  }
}

void VectorAppend(vector *v, const void *elemAddr)
{
  if(v->logLength >= v->allocLength) {
    v->allocLength *= 2;
    v->elems = realloc(v->elems, v->allocLength * v->elemSize);
  }
  memcpy(VectorNth(v, v->logLength), elemAddr, v->elemSize);
  v->logLength++;
}

void VectorDelete(vector *v, int position)
{
  assert(position>=0 && position<v->logLength);
  if(v->freeFn != NULL)
    v->freeFn(VectorNth(v, position));
  // v->logLength--;
  memmove(VectorNth(v, position), VectorNth(v, position+1), v->elemSize * (v->logLength-position-1));
  v->logLength--;
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
  assert(compare != NULL);
  qsort(v->elems, v->logLength, v->elemSize, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
  for(int i=0; i<v->logLength; i++) {
    mapFn(VectorNth(v, i), auxData);
  }
}

static int vectorIndex(void* found, void* base, int elemSize) {
  return ((char*)found - (char*)base)/elemSize;
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{
  assert(startIndex>=0 && startIndex<=v->logLength);
  assert(key != NULL && searchFn != NULL);
  void* found = NULL;
  if(isSorted) {
    found = bsearch(key, VectorNth(v, startIndex), v->logLength-startIndex, v->elemSize, searchFn);
  } else {
    for(int i=0; i<v->logLength; i++) {
      if(searchFn(key, VectorNth(v, i)) == 0) found = VectorNth(v, i);
    }
  }
  if(found != NULL) return vectorIndex(found, v->elems, v->elemSize);
  else return -1;
} 
