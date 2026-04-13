%{
#include "ast.hpp"
#include "executor.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern int yylineno;
void yyerror(const char *s);
int yylex();

extern Executor* g_executor;
%}

%code requires {
    #include "ast.hpp"
}

%union {
    char* str;
    int num;
    Argument* arg;
    ArgumentList* arglist;
    Command* cmd;
    Statement* stmt;
    std::vector<Argument*>* argvec;
}

%token <str> WORD STRING VARIABLE STRING_SEGMENT WORD_ASSIGN
%token <str> TOKEN_CAT TOKEN_ECHO TOKEN_WC TOKEN_PWD TOKEN_EXIT TOKEN_GREP
%token <num> NUMBER
%token PIPE SEMICOLON ASSIGN NEWLINE

%type <arg> argument
%type <arglist> argument_list
%type <argvec> argument_sequence
%type <cmd> command
%type <stmt> pipeline statement assignment

%start program

%%

program:
    /* empty */
    | program statement NEWLINE {
        if ($2) {
            g_executor->execute($2);
            delete $2;
        }
        if (g_executor->shouldExit()) {
            YYABORT;
        }
    }
    | program NEWLINE
    | program error NEWLINE {
        yyerror("Syntax error");
        yyerrok;
    }
    ;

statement:
    pipeline      { $$ = $1; }
    | assignment  { $$ = $1; }
    ;

assignment:
    WORD_ASSIGN argument_list {
        $$ = new Assignment($1, $2);
        free($1);
    }
    | WORD ASSIGN argument_list {
        $$ = new Assignment($1, $3);
        free($1);
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
    TOKEN_CAT argument_list     { $$ = new CatCommand($2); free($1); }
    | TOKEN_ECHO argument_list  { $$ = new EchoCommand($2); free($1); }
    | TOKEN_WC argument_list    { $$ = new WcCommand($2); free($1); }
    | TOKEN_PWD argument_list   { $$ = new PwdCommand($2); free($1); }
    | TOKEN_EXIT argument_list  { $$ = new ExitCommand($2); free($1); }
    | TOKEN_GREP argument_list  { $$ = new GrepCommand($2); free($1); }
    | WORD argument_list        {
        $$ = new ExternalCommand($1, $2);
        free($1);
    }
    | STRING argument_list      {
        $$ = new ExternalCommand($1, $2);
        free($1);
    }
    | VARIABLE argument_list    {
        std::string varName = std::string("$") + $1;
        $$ = new ExternalCommand(varName, $2);
        free($1);
    }
    ;

argument_list:
    /* empty */                 { $$ = new ArgumentList(); }
    | argument_sequence         {
        $$ = new ArgumentList();
        for (auto arg : *$1) {
            $$->push_back(arg);
        }
        delete $1;
    }
    ;

argument_sequence:
    argument {
        $$ = new std::vector<Argument*>();
        $$->push_back($1);
    }
    | argument_sequence argument {
        $1->push_back($2);
        $$ = $1;
    }
    ;

argument:
    WORD        { $$ = new Argument(Argument::WORD, $1); free($1); }
    | STRING    { $$ = new Argument(Argument::STRING, $1); free($1); }
    | STRING_SEGMENT { $$ = new Argument(Argument::STRING, $1); free($1); }
    | NUMBER    {
        char buf[32];
        sprintf(buf, "%d", $1);
        $$ = new Argument(Argument::WORD, strdup(buf));
    }
    | VARIABLE  { $$ = new Argument(Argument::VARIABLE, $1); free($1); }
    | TOKEN_CAT   { $$ = new Argument(Argument::WORD, $1); }
    | TOKEN_ECHO  { $$ = new Argument(Argument::WORD, $1); }
    | TOKEN_WC    { $$ = new Argument(Argument::WORD, $1); }
    | TOKEN_PWD   { $$ = new Argument(Argument::WORD, $1); }
    | TOKEN_EXIT  { $$ = new Argument(Argument::WORD, $1); }
    | TOKEN_GREP  { $$ = new Argument(Argument::WORD, $1); }
    | argument argument {
        std::vector<Argument*> parts;
        parts.push_back($1);
        parts.push_back($2);
        $$ = new Argument(Argument::COMPOSITE, parts);
    }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error at line %d: %s\n", yylineno, s);
}
