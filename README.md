This is a compiler that was written in 1998.  At that time I was interested in
compilers and worked on this while I was recovering from a motorcycle accident.

I was presented with a project where we needed to generate template source code
for multiple source languages which would call stored procedures in Sybase.

We had a C based intermediate library called libsyb which was a wrapper to the
Sybase OpenClient.  The libsyb library made it __easier__ to program to the OC
API, but it was by no means easy.

This compiler would ingest a short definition file and then compile
and interpret a template generator.  The template generator was rewritten for
each target programming language we supported.

Entire source trees of boilerplate code were easily recreated with simple
Makefiles and terse stored procedure definitions.

We supported C and Java target languages, with several hundred stored
procedures to do various things.

This code demonstrates how to use Lex/Yacc to build a compiler that generates
intermediate bytecode which is then processed by a simple bytecode interpreter
to perform the actions written in the template code.

There are perhaps simpler ways to do this task, but one way or another you are
rewriting the template generator code for each target language, with this
approach you don't have to duplicate code because the compiler is there to
provide that reusable element.
