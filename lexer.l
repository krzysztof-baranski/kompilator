%option noyywrap  
%{
	#include "parser.h"
	#include "global.h"
	int lineno = 1;
	using namespace std;

%}

letter					[a-zA-Z]
digit 					[0-9]
id						{letter}({letter}|{digit})*

digits					{digit}+
optional_exponent		[eE][+-]?{digits}
int_v					{digits}
real_v					{digits}("."{digits}){optional_exponent}?

%%
[\t | " "]+
[ \n ] 				{lineno++;}
"program"			{return PROGRAM_TKN;}
"begin"				{return BEGIN_TKN;}
"end" 				{return END_TKN;}
"var"				{return VAR_TKN;}
"integer" 			{yylval = INTEGER_TKN; return INTEGER_TKN;}
"real"				{yylval = REAL_TKN; return REAL_TKN;}
"or"				{return OR_TKN;}
"not"				{return NOT_TKN;}
"if" 				{return IF_TKN;}
"then" 				{return THEN_TKN;}
"else" 				{return ELSE_TKN;}
"while"				{return WHILE_TKN;}
"do"				{return DO_TKN;}
"array" 			{return ARRAY_TKN;}
"of" 				{return OF_TKN;}
"function"			{return FUNCTION_TKN;}
"procedure"			{return PROCEDURE_TKN;}
"="					{yylval = EQ_TKN; return RELOP_TKN;}
"<>"				{yylval = NE_TKN; return RELOP_TKN;}
"<"					{yylval = LT_TKN; return RELOP_TKN;}
"<="				{yylval = LE_TKN; return RELOP_TKN;}
">"					{yylval = GT_TKN; return RELOP_TKN;}
">="				{yylval = GE_TKN; return RELOP_TKN;}
"*" 				{yylval = MUL_TKN; return MULOP_TKN;}
"div" 				{yylval = DIV_TKN; return MULOP_TKN;}
"/" 				{yylval = DIV_TKN; return MULOP_TKN;}
"mod"  				{yylval = MOD_TKN; return MULOP_TKN;}
"and"  				{yylval = AND_TKN; return MULOP_TKN;}
"+" 				{yylval = PLUS_TKN; return SIGN_TKN;}
"-" 				{yylval = MINUS_TKN; return SIGN_TKN;}
":="				{return ASSIGNOP_TKN;}
{id} 				{
						int index = findSymbolIndexByScope(yytext);
						if(index == -1){
							index = addToSymbolTable(yytext, NONE_TKN, ID_TKN);
						}
						yylval = index;
						return ID_TKN;
					}
{int_v}				{
						int index = addNum(yytext, INTEGER_TKN);
						yylval = index;
						return NUM_TKN;
					}
{real_v}			{
						int index = addNum(yytext, REAL_TKN);
						yylval = index;
						return NUM_TKN;
					}	
<<EOF>>				{return DONE_TKN;}
.					{return *yytext;}
%%
