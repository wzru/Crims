#ifndef REGEX_H
#define REGEX_H

/* match :在text中查找regexp */
int match (char *regexp, char *text);

/* match_here在text的开头查找regexp */
int match_here (char *regexp, char *text);

/* match_star :在text的开头查找C*regexp */
int match_star (int c, char *regexp, char *text);

#endif