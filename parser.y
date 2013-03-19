%code requires{
 
 
#include "ast.h"
#include "stdio.h"
using namespace lang; 
int yylex(void);

void yyerror (char const *s);
 
}

%output  "parser.cpp"
%defines "parser.h"

%error-verbose

%union {
    int num;
    char* id;
    lang::Assignment* assignment;
    lang::Expression* expression;
    lang::CompilationUnit* unit;
}
 
%left '+' TOKEN_PLUS
%left '*' TOKEN_MULTIPLY

%token TOKEN_EQUALS
%token TOKEN_SEMICOLON
%token <id> TOKEN_IDENTIFIER
%token <num> TOKEN_NUMBER
%token TOKEN_ASSIGN
%token TOKEN_UNKNOWN
 
%type <assignment> assignment
%type <expression> expression
%type <unit> program
 
%%

compilation_unit: program { lang::parsed_compilation_unit = $1; };

program
	: assignment program { $$ = $2; $$->statements.push_front($1); }
	| assignment { $$ = new CompilationUnit(); $$->statements.push_front($1);}
	;

assignment
    : TOKEN_IDENTIFIER TOKEN_EQUALS expression TOKEN_SEMICOLON { $$ = new Assignment($1, $3); free($1);}
    ;

expression
	: expression TOKEN_PLUS expression { $$ = new BinaryExpression($1, $3); }
	| TOKEN_NUMBER { $$ = new LiteralExpression($1); }
	| TOKEN_IDENTIFIER { $$ = new IdExpression($1); free($1);}
	;

%%