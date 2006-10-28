/********************************************************************
 * toolshed.h - ToolShed global header
 *
 * $Id$
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
#include <cocopath.h>

#define TS_MAXSTR	128


/* ERROR CODES */
#define EOS_PADROM		257


void TSReportError(error_code te, char *errorstr);

error_code TSPadROM(char *pathlist, int padSize, char padChar);
error_code TSRename(char *pathlist, char *new_name);
error_code TSRBFAttrGet(char *p, char attr, char *strattr);
error_code TSRBFAttrSet(char *file, int attrSetMask, int attrResetMask, char attr, char *strattr);

#ifdef __cplusplus
}
#endif
