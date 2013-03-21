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
    lang::Statement* statement;
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
%token TOKEN_FUNC;
%token TOKEN_CLASS;
%token TOKEN_LPAREN;
%token TOKEN_RPAREN;
%token TOKEN_LCURLY;
%token TOKEN_RCURLY;
 
%type <statement> assignment
%type <statement> class
%type <statement> statement
%type <expression> expression
%type <unit> program
 
%%

compilation_unit: program { lang::parsed_compilation_unit = $1; };

program
	: program statement{ $$ = $1; $$->statements.push_back($2); }
	| statement { $$ = new CompilationUnit(); $$->statements.push_back($1);}
	;

statement
	: class
	| assignment
	;

class
	: TOKEN_CLASS TOKEN_IDENTIFIER TOKEN_LPAREN TOKEN_RPAREN TOKEN_LCURLY TOKEN_RCURLY { $$ = new ClassDefinition($2); free($2);}

assignment
    : TOKEN_IDENTIFIER TOKEN_EQUALS expression TOKEN_SEMICOLON { $$ = new Assignment($1, $3); free($1);}
    ;

expression
	: expression TOKEN_PLUS expression { $$ = new BinaryExpression($1, $3); }
	| TOKEN_NUMBER { $$ = new LiteralExpression($1); }
	| TOKEN_IDENTIFIER { $$ = new IdExpression($1); free($1);}
	;

%%