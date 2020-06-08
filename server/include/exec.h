#ifndef EXEC_H
#define EXEC_H

#include <time.h>

#include "define.h"
#include "select.h"
#include "ast.h"

extern ExprNode error_expr, null_expr, zero_expr, one_expr, lazy_expr;

enum
{
    QUERY_BEGIN,
    QUERY_REEVAL
};

extern clock_t op_start, op_end;
extern byte query_status;

inline int exec (char *command);

#endif