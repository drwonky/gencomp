#ifndef MACHINE_H
#define MACHINE_H

#include "opcodes.h"
#include "stack.h"

#define	DEF_PROG_SIZE	4096	/* default program size initialized to */

typedef struct instruction {
	opcode	op;		/* opcode enum */
	int	op_len;		/* offset to next opcode */
} Instruction;

typedef struct opparm {
	int	type;
	void	*parm;
} Opparm;

typedef struct program {
	int		prog_size;	/* program max size */
	int		prog_cur_size;	/* program current size */
	char		*prog;		/* program text */
	char		*prog_ip;	/* program current pointer */
} Program;

#endif
