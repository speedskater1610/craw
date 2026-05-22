/*
 * File: cvector.h
 * ---------------
 * Minimal dynamic array (CVector) implementation for the Lisp garbage collector.
 * Provides: cvec_init, cvec_append, cvec_filter, cvec_dispose, for_vector.
 */

#ifndef _CVECTOR_H_INCLUDED
#define _CVECTOR_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef void (*CleanupFn)(void *elem);

typedef struct {
  void      *data;       // raw element buffer
  size_t     elemsz;     // size of each element in bytes
  size_t     count;      // number of elements currently stored
  size_t     capacity;   // number of elements that fit in data
  CleanupFn  cleanup;    // called on each element before removal/dispose
} CVector;

/* for_vector(cvec_ptr, elem_ptr) — iterates over elements as void* */
#define for_vector(cv, ep) \
  for (char *(ep) = (char *)(cv)->data; \
       (ep) < (char *)(cv)->data + (cv)->count * (cv)->elemsz; \
       (ep) += (cv)->elemsz)

static inline bool cvec_init(CVector *cv, size_t elemsz, size_t initial_cap, CleanupFn cleanup) {
  assert(cv != NULL);
  assert(elemsz > 0);
  cv->elemsz   = elemsz;
  cv->count    = 0;
  cv->cleanup  = cleanup;
  cv->capacity = initial_cap > 0 ? initial_cap : 8;
  cv->data     = malloc(cv->capacity * elemsz);
  return cv->data != NULL;
}

static inline bool cvec_append(CVector *cv, const void *elem) {
  assert(cv != NULL && elem != NULL);
  if (cv->count == cv->capacity) {
    size_t new_cap = cv->capacity * 2;
    void  *new_data = realloc(cv->data, new_cap * cv->elemsz);
    if (!new_data) return false;
    cv->data     = new_data;
    cv->capacity = new_cap;
  }
  memcpy((char *)cv->data + cv->count * cv->elemsz, elem, cv->elemsz);
  cv->count++;
  return true;
}

/* Keep only elements for which predicate returns true; calls cleanup on removed ones */
static inline void cvec_filter(CVector *cv, bool (*predicate)(const void *elem)) {
  assert(cv != NULL && predicate != NULL);
  size_t write = 0;
  for (size_t i = 0; i < cv->count; i++) {
    void *el = (char *)cv->data + i * cv->elemsz;
    if (predicate(el)) {
      if (write != i)
        memcpy((char *)cv->data + write * cv->elemsz, el, cv->elemsz);
      write++;
    } else {
      if (cv->cleanup) cv->cleanup(el);
    }
  }
  cv->count = write;
}

static inline void cvec_dispose(CVector *cv) {
  assert(cv != NULL);
  if (cv->cleanup) {
    for (size_t i = 0; i < cv->count; i++)
      cv->cleanup((char *)cv->data + i * cv->elemsz);
  }
  free(cv->data);
  cv->data     = NULL;
  cv->count    = 0;
  cv->capacity = 0;
}

#endif /* _CVECTOR_H_INCLUDED */