/********************************************************************
 * cocoutil.h - CoCo Utility header file
 *
 * $Id$
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <cocotypes.h>

/* Function prototypes for supported CoCo commands are here */

int cocoattr(int, char **);
int cococmp(int, char **);
int cococopy(int, char **);
int cocodcheck(int, char **);
int cocodel(int, char **);
int cocodeldir(int, char **);
int cocodir(int, char **);
int cocodsave(int, char **);
int cocodump(int, char **);
int cocoformat(int, char **);
int cocofree(int, char **);
int cocofstat(int, char **);
int cocogen(int, char **);
int cocoid(int, char **);
int cocoident(int, char **);
int cocolist(int, char **);
int cocomakdir(int, char **);
int cocomodbust(int, char **);
int cocopadrom(int, char **);
int cocorename(int, char **);

int StrToInt(char *s);
void show_help(char **helpMessage);

#ifdef __cplusplus
}
#endif
