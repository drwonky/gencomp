#ifndef SP_PARSE_H
#define SP_PARSE_H

#define PR_COMMENT 257
#define PR_DEF 258
#define PR_TYPE 259
#define PR_INPUT 260
#define PR_OUTPUT 261
#define PR_RESULT 262
#define PR_PARM 263
#define PR_WORD 264
#define PR_STRING 265

typedef union {
	char *string;
} PRSTYPE;

extern PRSTYPE prlval;

#define PR_RETURN	0
#define PR_ROW		1
#define PR_ROWS		2

FILE *prin;

#endif
