#ifndef FUNC_EXISTS_H
#define FUNC_EXISTS_H
 
/*
 * preprocess_funcExists
 *
 * Scans `src` and replaces every  @funcExists<name>  with "1" if a
 * function called `name` is defined in the source, or "0" otherwise.
 *
 * Returns a newly heap-allocated string.  Caller must free().
 */
char *preprocess_funcExists(const char *src);
 
#endif /* FUNC_EXISTS_H */
 