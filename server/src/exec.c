#include "database.h"
#include "exec.h"
#include "ast.h"

inline int exec(char *command)
{
    SqlAst *root;
    root = parse_sql(command);
    return 0;
}