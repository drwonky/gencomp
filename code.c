#include "machine.h"
#include "opcodes.h"
#include "stack.h"
#include "datatypes.h"
#include "symtab.h"
#include "globals.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef union {
	char		*string;
	int		number;
	char		bool;
	symtab_node	*node;
} parm_data;


/* macro for dealing with opcode parameters */
#define arg_load(a,parm,ap) do{ \
	switch(a) { \
		case ST_STRING: \
			parm.string=va_arg(ap,char *); \
			len+=strlen(parm.string)+1; \
			break; \
		case ST_INT: \
			parm.number=va_arg(ap,int); \
			len+=sizeof(int); \
			break; \
		case ST_BOOL: \
			parm.bool=va_arg(ap,char); \
			len+=sizeof(parm.bool); \
			break; \
		case ST_SYMTAB: \
			parm.node=va_arg(ap,symtab_node *); \
			len+=sizeof((symtab_node *)parm.node); \
			break; \
	} \
}while(0)

/* macro to copy instruction to program text */
#define copy_opcode(prog,ip) do{ \
	memcpy(prog->prog_ip,&ip,sizeof(Instruction)); \
	prog->prog_cur_size+=sizeof(Instruction); \
	prog->prog_ip+=sizeof(Instruction); \
}while(0)

/* macro for reallocating program storage */
#define resize_prog(prog,ap) do { \
	prog->prog=realloc(prog->prog,prog->prog_size+DEF_PROG_SIZE); \
	if(!prog->prog) { \
		va_end(ap); \
		return FALSE; \
	} \
	prog->prog_size=prog->prog_size+DEF_PROG_SIZE; \
} while(0)


/* macro to copy opcode argument to program text */
#define copy_arg(prog,arg_type,arg) do{ \
	memcpy(prog->prog_ip,&arg_type,sizeof(arg_type));  \
	switch(arg_type) {  \
		case ST_STRING:  \
			memcpy(prog->prog_ip+sizeof(arg_type),arg.string,strlen(arg.string)+1);  \
			break;  \
		case ST_INT:  \
			memcpy(prog->prog_ip+sizeof(arg_type),&arg.number,sizeof(arg.number));  \
			break;  \
		case ST_BOOL:  \
			memcpy(prog->prog_ip+sizeof(arg_type),&arg.bool,sizeof(arg.bool));  \
			break;  \
		case ST_SYMTAB:  \
			memcpy(prog->prog_ip+sizeof(arg_type),(symtab_node *)&arg.node,sizeof(arg.node));  \
			break;  \
	}  \
}while(0)

BOOLEAN generate(Program *prog, opcode op, ...)
{
	int		len=0,offset;	/* instruction length */
	Instruction	ip;		/* assembled instruction */
	int		p_type=0;	/* opcode parameter type */
	va_list		ap;		/* arg list */
	parm_data	parm;

	va_start(ap,op);
	

#if 0
	fprintf(stderr,"Prog ip before: %p\n",prog->prog_ip);
#endif
	len=sizeof(Instruction); /* size of opcode, inclusive */
	ip.op=op;

	switch(op) {
		case op_lookup:
		case op_store:
		case op_label:
		case op_jump:
		case op_jumpz:
		case op_jumpnz:
		case op_write:
		case op_alias:
		case op_next:
		case op_push:
			p_type=va_arg(ap,int);

			len+=sizeof(p_type);

			arg_load(p_type,parm,ap); /* fetches parameter and increments len */

			if(prog->prog_size < prog->prog_cur_size + len) {
				offset=(int)(prog->prog_ip-prog->prog);
				prog->prog=realloc(prog->prog,prog->prog_size+DEF_PROG_SIZE); 
				if(!prog->prog) { 
					va_end(ap); 
					return FALSE; 
				}
				prog->prog_size=prog->prog_size+DEF_PROG_SIZE;
				prog->prog_ip=prog->prog+offset;
			}

			if(len % 8) {
				len+=(8-(len%8));
			}

			ip.op_len=len;
			copy_opcode(prog,ip);		/* copy opcode and increment ip */
			copy_arg(prog,p_type,parm);	/* copy opcode parameters and inc ip */

			break;
		case op_or:
		case op_and:
		case op_cmp:
		case op_cmpnot:
		case op_not:
		case op_halt:
		case op_pop:
			if(prog->prog_size < prog->prog_cur_size + len) {
				offset=(int)(prog->prog_ip-prog->prog);
				prog->prog=realloc(prog->prog,prog->prog_size+DEF_PROG_SIZE); 
				if(!prog->prog) { 
					va_end(ap); 
					return FALSE; 
				}
				prog->prog_size=prog->prog_size+DEF_PROG_SIZE;
				prog->prog_ip=prog->prog+offset;
			}

			if(len % 8) {
				len+=(8-(len%8));
			}

			ip.op_len=len;
			copy_opcode(prog,ip);

			prog->prog_ip+=(len-sizeof(Instruction));
			break;
	}

	if(len % 8) {
		len+=(8-(len%8));
	}

	#if DEBUG >= 8
	if(ip.op_len != len) {
		fprintf(stderr,"consistency error in opcode length! %d != %d\n",ip.op_len,len);
	}
	#endif

	prog->prog_cur_size+=len;
	prog->prog_ip+=(len-sizeof(Instruction));

#if 0
	fprintf(stderr,"Prog ip after: %p\n",prog->prog_ip);
#endif

	va_end(ap);

	return TRUE;
}
