#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "y.tab.h"
#include "symtab.h"
#include "globals.h"
#include "sp_parse.h"

extern int lineno;
extern int yylineno;
extern FILE *yyin;
extern char prchar;
extern char yychar;
char	*prfilename;
char	*yyfilename;

typedef struct filelist {
	int		fd;
	struct filelist *next;
	char		*filename;
} Filelist;

Filelist *flist;

void add_flist(int fd,char *filename)
{
	Filelist *f;

	f=malloc(sizeof(Filelist));

	f->fd=fd;
	f->filename=filename;

	f->next=flist;

	flist=f;
}

void close_del(void)
{
	Filelist	*f;

	f=flist;
	while(f) {
		close(f->fd);
		unlink(f->filename);

		flist=f->next;
		free(f);

		f=flist;
	}
}

void prerror(char *msg, ...)
{
	va_list	ap;
	
	va_start(ap,msg);

	printf("%s: error: %d: ",prfilename, lineno);
	vprintf(msg,ap);
	printf("\n");

	va_end(ap);

	close_del();
	exit(1);
}

void yyerror(char *msg, ...)
{
	va_list	ap;
	
	va_start(ap,msg);

	printf("%s: error: %d: ",yyfilename, lineno);
	vprintf(msg,ap);
	printf("\n");

	va_end(ap);

	close_del();
	exit(1);
}

BOOLEAN init(void)
{

	if(!(symbol_table=new_symtab_node())) return FALSE;
	
	pr_input=NULL;
	pr_output=NULL;
	pr_result=NULL;

	if(!(pr_list=create_symtab_node(symbol_table,"parameters",NULL,SYMTAB_NUL))) 
		return FALSE;

	tmp_node=NULL;
	flist=NULL;
	stack=NULL;
	lineno=1;

}

void usage(char *prog)
{
	fprintf(stderr,
	"USAGE: %s -t <template filename> -o <symbolic descriptor>=<output filename>\n"
	"\tYou may have multiple -o options to the program, to define as many files\n"
	"\tas you want.  The symbolic name is the name you use within the template\n"
	"\tfile to reference the descriptor you're writing to.  The output filename\n"
	"\tis simply the name of the file you want to open.\n", prog);
}

int main(int argc, char *argv[])
{
	int		opt,fd;
	char		*p,tmp[1024];
	FILE		*out;
	Filelist	*f;
	char		*compiled=NULL;
	
	prin=NULL;
	yyin=NULL;

	if(!init()) {
		fprintf(stderr,"Error initializing compiler\n");
		exit(1);
	}

	while((opt=getopt(argc,argv,"c:o:t:"))>0) {
		switch(opt) {
			case 'c':
				compiled=optarg;
				break;
			case 't':
				if(!(yyin=fopen(optarg,"r"))) {
					fprintf(stderr,"Error opening template file %s\n",optarg);
				}

				yyfilename=optarg;
				break;
			case 'o':
				p=strchr(optarg,'=');
				if(!p) {
					fprintf(stderr,"Syntax error in -o '%s', missing '='\n",optarg);
					usage(argv[0]);
					exit(1);
				}

				if(p-optarg<1023) {
					strncpy(tmp,optarg,p-optarg);
					tmp[p-optarg]='\0';
				} else {
					fprintf(stderr,"Parameter to -o option too long\n");
					usage(argv[0]);
					exit(1);
				}
				p++;

				if(!(fd=open(p,O_WRONLY|O_CREAT|O_TRUNC,00664))) {
					fprintf(stderr,"Couldn't open %s for write\n",p);
					exit(1);
				}

				#if DEBUG == 9
				fprintf(stderr,"%s => %d\n",p,fd);
				#endif

				add_flist(fd,p);

				if(!create_symtab_node(symbol_table,tmp,&fd,SYMTAB_INT)) {
					fprintf(stderr,"Error adding file '%s' to symbol table\n",p);
					exit(1);
				}
				break;
		}
	}

	if(!optind || !argv[optind]) {
		fprintf(stderr,"No definition file supplied\n");
		usage(argv[0]);
		exit(1);
	}

	prin=fopen(argv[optind],"r");
	prfilename=argv[optind];

	if(!yyin || !prin) {
		fprintf(stderr,"Either the template file or the definition file could not be opened\n");
		usage(argv[0]);
		exit(1);
	}

	if(!init_program()) {
		fprintf(stderr,"Error initializing program text\n");
		exit(1);
	}

	while(!feof(prin)) {
		prparse();
	}

	lineno=1;

	while(!feof(yyin)) {
		yyparse();
	}

	generate(program,op_halt);

	if(compiled) {
		out=fopen(compiled,"w");

		fwrite(program->prog,program->prog_cur_size,1,out);

		fclose(out);
	}
	
	if(!run_program()) {
		fprintf(stderr,"Error running code\n");
		exit(1);
	}


	f=flist;
	while(f) {
		close(f->fd);

		flist=f->next;
		free(f);

		f=flist;
	}

	exit(0);
}
