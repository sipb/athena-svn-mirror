#ifndef _CVT_H_
#define _CVT_H_

typedef struct {
  int len;
  char *buf;
} buffer;

extern char *cvt_query;
extern int cvt_buf2vars(varlist *, buffer *);
extern int cvt_strings2buf(buffer **, char **);
extern int cvt_vars2buf(buffer **, varlist *);
#endif /* _CVT_H_ */
