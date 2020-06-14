#ifndef DELETE_H
#define DELETE_H

#include "ast.h"

inline int do_delete (DeleteNode *del);

inline int is_equal (void *p, int ti, int col, ExprNode *rhs);

#endif