#include "regex.h"

/* match :在text中查找regexp */
int match (char *regexp, char *text)
{
    if (regexp[0] == '^')
    {
        return match_here (regexp + 1, text);
    }
    do  /*即使字符串为空也必须检查*/
    {
        if (match_here (regexp, text))
        {
            return 1;
        }
    }
    while (*text++ != '\0');
    return 0;
}

/* match_here在text的开头查找regexp */
int match_here (char *regexp, char *text)
{
    if (regexp[0] == '\0')
    {
        return 1;
    }
    if (regexp[1] == '*')
    {
        return match_star (regexp[0], regexp + 2, text);
    }
    if (regexp[0] == '$' && regexp[1] == '\0')
    {
        return *text == '\0';
    }
    if (*text != '\0' && (regexp[0] == '.' || regexp[0] == *text))
    {
        return match_here (regexp + 1, text + 1);
    }
    return 0;
}

/* match_star :在text的开头查找C*regexp */
int match_star (int c, char *regexp, char *text)
{
    do   /*通配符*匹配零个或多个实例*/
    {
        if (match_here (regexp, text))
        {
            return 1;
        }
    }
    while (*text != '\0' && (*text++ == c || c == '.'));
    return 0;
}