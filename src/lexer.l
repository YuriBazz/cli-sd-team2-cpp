%{
#include "parser.tab.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>

extern int yylineno;
void yyerror(const char *s);

char string_buf[4096];
int string_len;
%}

%option noyywrap
%option yylineno

%x DQ_STRING
%x SQ_STRING

%%

[ \t\r]+        { return SPACE; }

\n              { return NEWLINE; }

"|"             { return PIPE; }
";"             { return SEMICOLON; }

[a-zA-Z_][a-zA-Z0-9_]*= {
    char* eq = strchr(yytext, '=');
    *eq = '\0';
    yylval.str = strdup(yytext);
    return WORD_ASSIGN;
}

"="             { return ASSIGN; }

"cat"           { yylval.str = strdup(yytext); return TOKEN_CAT; }
"echo"          { yylval.str = strdup(yytext); return TOKEN_ECHO; }
"wc"            { yylval.str = strdup(yytext); return TOKEN_WC; }
"pwd"           { yylval.str = strdup(yytext); return TOKEN_PWD; }
"exit"          { yylval.str = strdup(yytext); return TOKEN_EXIT; }
"grep"          { yylval.str = strdup(yytext); return TOKEN_GREP; }
"ls"            { yylval.str = strdup(yytext); return TOKEN_LS; }

[a-zA-Z_][a-zA-Z0-9_]*  {
    yylval.str = strdup(yytext);
    return WORD;
}

\$[a-zA-Z_][a-zA-Z0-9_]*  {
    yylval.str = strdup(yytext+1);
    return VARIABLE;
}

\$? {
    yylval.str = strdup("?");
    return VARIABLE;
}

[-a-zA-Z0-9_./][-a-zA-Z0-9_./]*  {
    yylval.str = strdup(yytext);
    return WORD;
}

[0-9]+          {
    yylval.num = atoi(yytext);
    return NUMBER;
}

\"              {
    BEGIN(DQ_STRING);
    string_len = 0;
    string_buf[0] = '\0';
}

<DQ_STRING>\"   {
    BEGIN(INITIAL);
    string_buf[string_len] = '\0';
    yylval.str = strdup(string_buf);
    return STRING;
}

<DQ_STRING>\n   {
    yyerror("Unterminated double-quoted string");
    BEGIN(INITIAL);
}

<DQ_STRING>\\n  { string_buf[string_len++] = '\n'; }
<DQ_STRING>\\t  { string_buf[string_len++] = '\t'; }
<DQ_STRING>\\\" { string_buf[string_len++] = '\"'; }
<DQ_STRING>\\\\ { string_buf[string_len++] = '\\'; }

<DQ_STRING>\$[a-zA-Z_][a-zA-Z0-9_]* {
    string_buf[string_len] = '\0';
    if (string_len > 0) {
        yylval.str = strdup(string_buf);
        string_len = 0;
        string_buf[0] = '\0';
        return STRING_SEGMENT;
    }
    yylval.str = strdup(yytext+1);
    return VARIABLE;
}

<DQ_STRING>\$? {
    string_buf[string_len] = '\0';
    if (string_len > 0) {
        yylval.str = strdup(string_buf);
        string_len = 0;
        string_buf[0] = '\0';
        return STRING_SEGMENT;
    }
    yylval.str = strdup("?");
    return VARIABLE;
}

<DQ_STRING>.    { string_buf[string_len++] = yytext[0]; }

\'              {
    BEGIN(SQ_STRING);
    string_len = 0;
    string_buf[0] = '\0';
}

<SQ_STRING>\'   {
    BEGIN(INITIAL);
    string_buf[string_len] = '\0';
    yylval.str = strdup(string_buf);
    return STRING;
}

<SQ_STRING>\n   {
    yyerror("Unterminated single-quoted string");
    BEGIN(INITIAL);
}

<SQ_STRING>\\\' { string_buf[string_len++] = '\''; }
<SQ_STRING>.    { string_buf[string_len++] = yytext[0]; }

.               {
    char msg[100];
    sprintf(msg, "Invalid character '%s'", yytext);
    yyerror(msg);
}

%%
