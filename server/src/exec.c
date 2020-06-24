#include <malloc.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "database.h"
#include "strptime.h"
#include "analyze.h"
#include "update.h"
#include "delete.h"
#include "insert.h"
#include "server.h"
#include "select.h"
#include "shell.h"
#include "debug.h"
#include "json.h"
#include "ast.h"
#include "log.h"

char single_command[BUFFER_LENGTH];
ExprNode error_expr = {.type = EXPR_ERROR},
         null_expr = {.type = EXPR_NULL},
         lazy_expr = {.type = EXPR_LAZY},
         zero_expr = {.type = EXPR_INTNUM, .intval = 0},
         one_expr = {.type = EXPR_INTNUM, .intval = 1};
byte query_status;
clock_t op_start, op_end;

/*
    判断是否是特殊操作, 如SAVE
*/
inline int is_spop (char *op, char *sql)
{
    while (*sql == ' ' || *sql == '\n' || *sql == '\r' || *sql == '\t')
    {
        ++sql;
    }
    return !stricmp (op, sql);
}

/*
    执行单句SQL, 输入为单行SQL(含分号)
*/
inline int exec_single (char *sql)
{
    op_start = clock();
    query_initialize();
    if (is_spop ("SAVE;", sql))
    {
        int res = write_db (database_path);
        if (res != ERROR)
        {
            jsonify_success ("SAVE");
        }
        else
        {
            jsonify_error ("SAVE");
        }
        return res;
    }
    else if (is_spop ("EXIT;", sql))
    {
        int res = write_db (database_path);
        if (res != ERROR)
        {
            jsonify_success ("EXIT");
        }
        else
        {
            jsonify_error ("EXIT");
        }
        exit (res);
    }
    SqlAst *root = parse_sql (sql);
    //print_ast(root, 0);
    if (root == NULL)
    {
        return STATUS_ERROR;
    }
    else if (root->type == SELECT_STMT)
    {
        int res = do_select (root->select, &rec, &recs, 0,
                             is_grpby = (root->select->group != NULL),
                             is_odrby = (root->select->order != NULL));
        if (res != ERROR)
        {
            switch (crims_status)
            {
            case STATUS_SERVER:
                jsonify_result (&recs);
                break;
            case STATUS_SHELL:
            case STATUS_EXEC:
                plog ("[INFO]: Select sucessfully!\n");
                print_result (&recs);
                break;
            }
        }
        else
        {
            switch (crims_status)
            {
            case STATUS_SERVER:
                jsonify_error ("SELECT");
                break;
            case STATUS_SHELL:
            case STATUS_EXEC:
                plog ("[ERROR]: Select failed!\n");
                break;
            }
        }
        return res;
    }
    else if (root->type == DELETE_STMT)
    {
        int res = do_delete (root->delete);
        if (res == ERROR)
        {
            switch (crims_status)
            {
            case STATUS_SERVER:
                jsonify_error ("DELETE");
                break;
            case STATUS_SHELL:
            case STATUS_EXEC:
                plog ("[ERROR]: Delete failed!\n");
                break;
            }
            return ERROR;
        }
        else
        {
            switch (crims_status)
            {
            case STATUS_SERVER:
                jsonify_success ("DELETE");
                break;
            case STATUS_SHELL:
            case STATUS_EXEC:
                plog ("[INFO]: Delete successfully\n");
                break;
            }
        }
        return 0;
    }
    else if (root->type == INSERT_STMT)
    {
        int res = do_insert (root->insert);
        if (res == ERROR)
        {
            switch (crims_status)
            {
            case STATUS_SERVER:
                jsonify_error ("INSERT");
                break;
            case STATUS_SHELL:
            case STATUS_EXEC:
                plog ("[ERROR]: Insert failed!\n");
                break;
            }
            return ERROR;
        }
        else
        {
            switch (crims_status)
            {
            case STATUS_SERVER:
                jsonify_success ("INSERT");
                break;
            case STATUS_SHELL:
            case STATUS_EXEC:
                plog ("[INFO]: Insert successfully\n");
                break;
            }
        }
        return 0;
    }
    else if (root->type == UPDATE_STMT)
    {
        int res = do_update (root->update);
        if (res == ERROR)
        {
            switch (crims_status)
            {
            case STATUS_SERVER:
                jsonify_error ("UPDATE");
                break;
            case STATUS_SHELL:
            case STATUS_EXEC:
                plog ("[ERROR]: Update failed!\n");
                break;
            }
            return ERROR;
        }
        else
        {
            switch (crims_status)
            {
            case STATUS_SERVER:
                jsonify_success ("UPDATE");
                break;
            case STATUS_SHELL:
            case STATUS_EXEC:
                plog ("[INFO]: Update successfully\n");
                break;
            }
        }
        return 0;
    }
    else
    {
        return STATUS_UNKNOWN;
    }
}

/*
    执行SQL, 输入参数为SQL字符串(可能为多行)
*/
inline int exec (char *command)
{
    int l = strlen (command), start = 0;
    int res = 0;
    for (int i = 0; i < l; ++i)
    {
        if (command[i] == ';')
        {
            strncpy (single_command, command + start, i - start + 1);
            start = i + 1;
            single_command[start] = '\0';
            plog ("[INFO]: Execute '%s'\n", single_command);
            res = exec_single (single_command);
        }
    }
    return res;
}