%{
#include <string.h>
#include"y.tab.h"
#include "miniprolog.h"

int inFile = 0;
YY_BUFFER_STATE standardInput;
int yywrap(void) {
	return 1; //end of file
}
%}

%option yylineno

/* regular definitions */
letter 			[A-Za-z]
digit   		[0-9]
variable      	[A-Z"_"][A-Za-z0-9"_"]*
id 				[0-9a-z]+
functor 		[a-z]+
file 			"["[A-Za-z0-9"_"".""/"]+"]"
%%


"?-" {
	return QUERY;
}

"\n" {
	return NEWLINE;
}

"." {
	return DOT;
}

":-" {
	return MID;
}

"," {
	return COMMA;
}
"(" {
	return LPARENT;
}
")" {
	return RPARENT;
}
{id} {
	yylval.string_val = strdup(yytext);
	return ID;
}
{variable} {
	yylval.string_val = strdup(yytext);
	return VARIABLE;
} 
{file} {
	inFile = 1; //flag to prevent printing the prompt
	//get file name and open file
	yylval.string_val = (char *)malloc(128);
	char * start = yylval.string_val;
	while(*(++yytext) != ']') *(yylval.string_val++) = *yytext; 
	*yylval.string_val = 0;
   	yytext = start;
   	
   	//switch input streams
	standardInput = YY_CURRENT_BUFFER;
	yyin = fopen( yytext, "r" );
	if (!yyin) yyerror("error, couldn't open file");
	yy_switch_to_buffer(yy_create_buffer( yyin, YY_BUF_SIZE ) );
	BEGIN(INITIAL); 
}
<<EOF>> {
	//either IO rdeirection or file stream, only switch buffers for file stream case
    if(inFile == 1) {
       inFile = 0;
       yy_delete_buffer( YY_CURRENT_BUFFER );
       yy_switch_to_buffer(standardInput);
    }
    else {
    	//end of input file from IO redirection, exit program
    	exit(0);
    }
}
. {
	//garbage
}
%%

