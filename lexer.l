%{
#include "parser.h"
#include <cstring>

using namespace std;
%}

digit         [0-9]
letter        [a-zA-Z]

%%
"="                  { return TOKEN_EQUALS;        }
";"	                 { return TOKEN_SEMICOLON; }
"+"	                 { return TOKEN_PLUS; }
"func"				 { return TOKEN_FUNC; }
"class"				 { return TOKEN_CLASS; }
"("					 { return TOKEN_LPAREN; }
")"					 { return TOKEN_RPAREN; }
"{"				 	 { return TOKEN_LCURLY; }
"}"				 	 { return TOKEN_RCURLY; }
{letter}({letter}|{digit})* {
                       yylval.id = strdup(yytext);
                       return TOKEN_IDENTIFIER;      }
{digit}+             { yylval.num = atoi(yytext);
                       return TOKEN_NUMBER;     }
[ \t\n\r]            /* skip whitespace */
.                    { printf("Unknown character [%c]\n",yytext[0]);
                       return TOKEN_UNKNOWN;    }
%%
 
int yywrap(void){return 1;}