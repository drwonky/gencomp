#ifndef OPCODES_H
#define OPCODES_H

static char *opcodes_txt[]={
	"push",
	"lookup",
	"pop",
	"store",
	"label",
	"jump",
	"jumpz",
	"jumpnz",
	"halt",
	"cmp",
	"or",
	"write",
	"next",
	"alias",
	"cmpnot",
	"not",
	"and"
};

typedef enum {
	op_push,	/* push immediate */
	op_lookup,	/* resolve a literal reference and push value onto stack */
	op_pop,		/* pop value */
	op_store,	/* pop and store value that's on stack */
	op_label,	/* general symbolic label */
	op_jump,	/* jump to label */
	op_jumpz,	/* pop value off stack, if zero jump to label */
	op_jumpnz,	/* pop value off stack, if non-zero jump to label */
	op_halt,	/* stop execution and cleanup */
	op_cmp,		/* compare top 2 items on stack and place comparison on stack */
	op_or,		/* compare top 2 operands, if either is non-zero, push non-zero */
	op_write,	/* pop item off stack and write it to immediate */
	op_next,	/* update pointer of immediate and push 0/1 onto stack */
	op_alias,	/* do an aliasing trick for the foreach statement, init foreach variable */
	op_cmpnot,	/* compare not */
	op_not,		/* invert bool on stack */
	op_and		/* and compare */
} opcode;

#endif
