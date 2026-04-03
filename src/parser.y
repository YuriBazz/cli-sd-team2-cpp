%{
#include "src/ast.hpp"
#include "src/executor.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern int yylineno;
void yyerror(const char *s);
int yylex();

extern Executor* g_executor;
%}

%code requires {
    #include "src/ast.hpp"
}

%union {
    char* str;
    int num;
    Argument* arg;
    ArgumentList* arglist;
    Command* cmd;
    Statement* stmt;
}

%token <str> WORD STRING VARIABLE
%token <num> NUMBER
%token PIPE SEMICOLON ASSIGN NEWLINE
%token TOKEN_CAT TOKEN_ECHO TOKEN_WC TOKEN_PWD TOKEN_EXIT TOKEN_GREP

%type <arg> argument
%type <arglist> argument_list
%type <cmd> command
%type <stmt> pipeline statement assignment

%start program

%%

program:
    /* empty */
    | program statement NEWLINE { g_executor->execute($2); delete $2; if (g_executor->shouldExit()) {YYABORT;}}
    | program NEWLINE
    | program error NEWLINE { yyerror("Syntax error"); yyerrok; }
    ;

statement:
    pipeline      { $$ = $1; }
    | assignment  { $$ = $1; }
    ;

assignment:
    WORD ASSIGN argument_list {
        $$ = new Assignment($1, $3);
    }
    ;

pipeline:
    command                     { $$ = new Pipeline({$1}); }
    | pipeline PIPE command     {
        Pipeline* p = dynamic_cast<Pipeline*>($1);
        if (p) {
            p->commands.push_back($3);
            $$ = p;
        } else {
            yyerror("Invalid pipeline");
            $$ = nullptr;
        }
    }
    ;

command:
    TOKEN_CAT argument_list     { $$ = new CatCommand($2); }
    | TOKEN_ECHO argument_list  { $$ = new EchoCommand($2); }
    | TOKEN_WC argument_list    { $$ = new WcCommand($2); }
    | TOKEN_PWD argument_list   { $$ = new PwdCommand($2); }
    | TOKEN_EXIT argument_list  { $$ = new ExitCommand($2); }
    | TOKEN_GREP argument_list  { $$ = new GrepCommand($2); }
    | WORD argument_list        { $$ = new ExternalCommand($1, $2); }
    ;

argument_list:
    /* empty */                 { $$ = new ArgumentList(); }
    | argument_list argument    {
        $1->push_back($2);
        $$ = $1;
    }
    ;

argument:
    WORD        { $$ = new Argument(Argument::WORD, $1); }
    | STRING    { $$ = new Argument(Argument::STRING, $1); }
    | NUMBER    {
        char buf[32];
        sprintf(buf, "%d", $1);
        $$ = new Argument(Argument::WORD, strdup(buf));
    }
    | VARIABLE  { $$ = new Argument(Argument::VARIABLE, $1); }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error at line %d: %s\n", yylineno, s);
}